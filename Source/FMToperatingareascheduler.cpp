/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifdef FMTWITHOSI

#include "FMToperatingareascheduler.h"
#include <algorithm>
#include <random>
#include "OsiClpSolverInterface.hpp"
#ifdef  FMTWITHMOSEK
	#include "OsiMskSolverInterface.hpp"
	#include "mosek.h"
#endif
//#include "FMTmatrixbuild.h"

namespace Heuristics
{
	void FMToperatingareascheduler::clearrowcache()
		{
		if (!useprimal)
			{
			FMTlpsolver::clearrowcache();
			}
		}


	void FMToperatingareascheduler::unboundall(bool atprimal)
		{
		try {
			this->clearrowcache();
			std::vector<int>targeteditems;
			std::vector<double>bounds;
			for (std::vector<FMToperatingareascheme>::const_iterator operatingareait = operatingareas.begin();
				operatingareait != operatingareas.end(); ++operatingareait)
				{
				if (useprimal || atprimal)
					{
					operatingareait->unboundallprimalschemes(targeteditems, bounds);
				}
				else { // dual
					operatingareait->unboundalldualschemes(targeteditems, bounds);
				}

				}
				if (useprimal || atprimal)
				{
					this->setColSetBounds(&targeteditems[0], &targeteditems.back() + 1, &bounds[0]);
				}else {
					this->setRowSetBounds(&targeteditems[0], &targeteditems.back() + 1, &bounds[0]);
					}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMToperatingareascheduler::unboundall", __LINE__, __FILE__, _section);
			}
		}

	void FMToperatingareascheduler::closeprimalbounds()
		{
        try{
            std::vector<int>variables;
            std::vector<double>bounds;
            for (std::vector<FMToperatingareascheme>::const_iterator operatingareait = operatingareas.begin();
                operatingareait != operatingareas.end(); ++operatingareait)
                    {
                    operatingareait->boundallprimalschemes(variables,bounds,0);
                    }
            this->setColSetBounds(&variables[0], &variables.back() + 1, &bounds[0]);
        }catch (...)
            {
			_exhandler->printexceptions("", "FMToperatingareascheduler::closeprimalbounds", __LINE__, __FILE__);
            }
		}

	void FMToperatingareascheduler::setallinteger()
		{
        try{
		if (!useprimal)//need to turn back the formulation to primal to make it work ith MIP
			{
				this->clearrowcache();
				const double* rowupper = this->getRowUpper();
				std::vector<int>rowsitems;
				std::vector<double>rowsbounds;
				std::vector<int>colsitems;
				std::vector<double>colsbounds;
				for (std::vector<FMToperatingareascheme>::const_iterator operatingareait = operatingareas.begin();
					operatingareait != operatingareas.end(); ++operatingareait)
					{
					size_t selected = 0;
					if (operatingareait->getdualsolutionindex(rowupper, selected))
						{
						operatingareait->boundprimalscheme(colsitems, colsbounds, selected);
						}
					operatingareait->unboundalldualschemes(rowsitems,rowsbounds);
					}
				//Need to bring it back to initial state!
				for (double& bound : rowsbounds)
					{
					if (bound!= std::numeric_limits<double>::lowest())
						{
						bound = 0;
						}
					}
				this->setRowSetBounds(&rowsitems[0], &rowsitems.back() + 1, &rowsbounds[0]);
				this->setColSetBounds(&colsitems[0], &colsitems.back() + 1, &colsbounds[0]);
				this->resolvemodel();
			}
		std::vector<int>integervariables;
		for (std::vector<FMToperatingareascheme>::const_iterator operatingareait = operatingareas.begin();
			operatingareait != operatingareas.end(); ++operatingareait)
				{
				operatingareait->pushbinaries(integervariables);
				}
		this->setInteger(&integervariables[0], static_cast<int>(integervariables.size()));
        }catch(...)
            {
            _exhandler->raisefromcatch("","FMToperatingareascheduler::setallinteger", __LINE__, __FILE__, _section);
            }
		}

	int FMToperatingareascheduler::resolvemodel()
		{
		try {
			if (!useprimal || solvertype != Models::FMTsolverinterface::CLP)
			{
				FMTlpsolver::resolve();
			}
			else {
				this->stockresolve();
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMToperatingareascheduler::resolvemodel", __LINE__, __FILE__, _section);
			}
		return FMTlpsolver::getiterationcount();
		}

	bool FMToperatingareascheduler::initialsolve()
		{
		try {
			if (this->isProvenOptimal())
			{
				const double initialobjectivevalue = this->getObjValue();
				size_t opareaprocessed = 0;
				std::string problemsolved = "primal";
				if (!useprimal)
				{
					this->unboundall(); //Make sure rhs are right need to be released
					this->closeprimalbounds(); //Need that to get some activities
					this->resolvemodel();
					problemsolved = "dual";
				}
				std::vector<std::vector<FMToperatingareascheme>::const_iterator> selected;
				do {
					this->clearrowcache();
					selected = this->setdraw();
					const size_t setssize = this->setbounds(selected);
					const int iterations = this->resolvemodel();
					opareaprocessed += selected.size();
					if (!selected.empty())
					{
						int setratio = static_cast<int>(((static_cast<double>(opareaprocessed)) / (static_cast<double>(this->operatingareas.size()))) * 100);
						(*_logger) << "Solution generation phase (" + std::to_string(setratio) + "%) took " + std::to_string(iterations) + " iterations on " + problemsolved +" formulation" << "\n";
					}
					if (!this->isProvenOptimal())
					{
						_exhandler->raise(Exception::FMTexc::FMTignore,
							"FMToperatingareascheduler failed switching to random",
							"FMToperatingareascheduler::initialsolve", __LINE__, __FILE__, _section);
						userandomness = true; //Switch to random now
						this->unboundall(); //release everything
						if (!useprimal)
						{
							this->closeprimalbounds();
						}
						this->resolvemodel();
						opareaprocessed = 0;
					}
					if (opareaprocessed > this->operatingareas.size())
						{
						_exhandler->raise(Exception::FMTexc::FMTfunctionfailed,
							"unable to bound operating areas ",
							"FMToperatingareascheduler::initialsolve", __LINE__, __FILE__, _section);
						}
				} while (!selected.empty() && this->isProvenOptimal());
				if (this->isProvenOptimal())
				{
					const double newobjective = this->getObjValue();
					const double dblgap = (100 - (round((newobjective / initialobjectivevalue) * 1000) / 10));
					(*_logger) << "Feasible solution found objective: " + std::to_string(round(newobjective)) + " (" + std::to_string(dblgap) + "%)" << "\n";
					this->clearrowcache();
				}
			}
		}catch (...)
		{
			_exhandler->printexceptions("", "FMToperatingareascheduler::initialsolve", __LINE__, __FILE__);
		}


		return this->isProvenOptimal();
		}

	void FMToperatingareascheduler::branchnboundsolve()
		{
        try{
            if (this->isProvenOptimal())
                {
                //In that order it seems to work...
                this->setallinteger();
                this->branchAndBound();
                this->unboundall(true);
                this->branchAndBound();
                }
        }catch(...)
            {
            _exhandler->printexceptions("", "FMToperatingareascheduler::branchnboundsolve", __LINE__, __FILE__);
            }
		}


	void FMToperatingareascheduler::setoperatingareasconstraints(const Graph::FMTgraph<Graph::FMTvertexproperties, Graph::FMTedgeproperties>& maingraph,
																const Models::FMTmodel& model,
																const Core::FMToutputnode& target)
		{
		try {
			Core::FMToutputnode specifictarget(target);
			const std::vector<Core::FMTaction>modelactions = model.getactions();
			const std::unordered_map<size_t, Graph::FMTgraph<Graph::FMTvertexproperties, Graph::FMTedgeproperties>::FMTvertex_descriptor> basedescriptors = maingraph.getperiodverticies(maingraph.getfirstactiveperiod());
			Core::FMToutputnode areatarget(specifictarget);
			Core::FMTmask areamask = areatarget.source.getmask();
			for (const Core::FMTtheme& theme : model.getthemes())
			{
				areamask.set(theme, "?");
			}
			areatarget.source = Core::FMToutputsource(Core::FMTspec(), areamask, Core::FMTotar::inventory);
			const std::vector<const Core::FMTaction*>actions = specifictarget.source.targets(modelactions);
			std::vector<int>actionids;
			for (const Core::FMTaction* actptr : actions)
			{
				actionids.push_back(std::distance(&modelactions[0], actptr));
			}
			const double* primalsolution = this->getColSolution();
			for (std::vector<FMToperatingareascheme>::iterator operatingareait = operatingareas.begin();
				operatingareait != operatingareas.end(); ++operatingareait)
			{
				specifictarget.source.setmask(operatingareait->getmask());
				areatarget.source.setmask(operatingareait->getmask());
				std::vector<std::vector<Graph::FMTgraph<Graph::FMTvertexproperties, Graph::FMTedgeproperties>::FMTvertex_descriptor>>descriptors;
				std::vector<Graph::FMTgraph<Graph::FMTvertexproperties, Graph::FMTedgeproperties>::FMTvertex_descriptor>totalareadescriptors;
				for (int period = (maingraph.getfirstactiveperiod() + operatingareait->getstartingperiod()); period < static_cast<int>((maingraph.size() - 1)); ++period)
				{
					if (descriptors.empty())
					{
						totalareadescriptors = maingraph.getnode(model, areatarget, period);
					}
					std::vector<Graph::FMTgraph<Graph::FMTvertexproperties, Graph::FMTedgeproperties>::FMTvertex_descriptor> perioddescriptors;
					if (!totalareadescriptors.empty())
					{
						perioddescriptors = maingraph.getnode(model, specifictarget, period);
					}
					descriptors.push_back(perioddescriptors);
				}
				if (!descriptors.empty())
				{
					operatingareait->setconstraints(descriptors, totalareadescriptors, maingraph, *this, primalsolution, actionids);
				}
				if (operatingareait->getarea()==0)
					{
					_exhandler->raise(Exception::FMTexc::FMTignore,
						"area of operating area "+
						std::string(operatingareait->getmask())+" is null",
						"FMToperatingareascheduler::setoperatingareasconstraints", __LINE__, __FILE__);
					}else if (operatingareait->getopeningbinaries().empty())
						{
						_exhandler->raise(Exception::FMTexc::FMTignore,
							std::string(operatingareait->getmask()) + " not operable",
							"FMToperatingareascheduler::setoperatingareasconstraints", __LINE__, __FILE__);
						}
			}
			this->synchronize();
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMToperatingareascheduler::setoperatingareasconstraints", __LINE__, __FILE__);
			}
		}

	void FMToperatingareascheduler::setadjacencyconstraints()
		{
		try {
		//Models::FMTmatrixbuild matrixbuild;
		const std::vector<double>elements(2, 1.0);
		std::vector<int>columns(2, 0);
		int constraintsid = this->getNumRows();
		for (std::vector<FMToperatingareascheme>::const_iterator operatingareait = operatingareas.begin();
			operatingareait != operatingareas.end(); ++operatingareait)
			{
			const std::vector<Core::FMTmask>neighbors = operatingareait->getneighbors();
			for (const Core::FMTmask& neighbor : neighbors)
				{
				std::pair<Core::FMTmask, Core::FMTmask>simple(operatingareait->getmask(), neighbor);
				std::pair<Core::FMTmask, Core::FMTmask>reverse(neighbor, operatingareait->getmask());
				if (adjacencyconstraints.find(simple)== adjacencyconstraints.end() &&
					adjacencyconstraints.find(reverse) == adjacencyconstraints.end())
					{
					std::vector<FMToperatingareascheme>::const_iterator opneighbor = std::find_if(operatingareas.begin(), operatingareas.end(), FMToperatingareaschemecomparator(neighbor));
					std::map<int, std::vector<int>> neighborsbin;
					if (opneighbor!= operatingareas.end())
						{
						neighborsbin  = operatingareait->getcommonbinairies(*opneighbor);
						}
					std::vector<int>constraintindexes;
					for (std::map<int, std::vector<int>>::const_iterator binit = neighborsbin.begin();binit!=neighborsbin.end();++binit)
						{
						for (const int& index : binit->second)
							{
							constraintindexes.push_back(constraintsid);
							++constraintsid;
							columns[0] = binit->first;
							columns[1] = index;
							this->addRow(2, &columns[0], &elements[0], 0, 1);
							//matrixbuild.addRow(2, &columns[0], &elements[0], 0, 1);
							}
						}
					if (!constraintindexes.empty())
						{
						adjacencyconstraints[simple] = constraintindexes;
						}

					}
				}
			}
		this->synchronize();
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMToperatingareascheduler::setadjacencyconstraints", __LINE__, __FILE__);
			}
		}

	FMToperatingareascheduler::FMToperatingareascheduler(const FMToperatingareascheduler& rhs) :
		FMTlpheuristic(rhs),operatingareas(rhs.operatingareas), adjacencyconstraints(rhs.adjacencyconstraints),
		proportionofset(rhs.proportionofset),
		userandomness(rhs.userandomness),useprimal(false)
		{

		}
	FMToperatingareascheduler& FMToperatingareascheduler::operator = (const FMToperatingareascheduler& rhs)
		{
		if (this!=&rhs)
			{
			FMTlpheuristic::operator=(rhs);
			operatingareas=rhs.operatingareas;
			adjacencyconstraints=rhs.adjacencyconstraints;
			proportionofset = rhs.proportionofset;
			userandomness = rhs.userandomness;
			useprimal = false;
			}
		return *this;
		}
	FMToperatingareascheduler::~FMToperatingareascheduler()
		{
		try {
		    if (!usingsolvercopy)
                {
                  //Will need a clean matrix to fit with FMTlpmodel!
                std::vector<int>rowstodelete;
                std::vector<int>columnstodelete;
                for (std::vector<FMToperatingareascheme>::const_iterator operatingareait = operatingareas.begin();
                    operatingareait != operatingareas.end(); ++operatingareait)
                {
                    operatingareait->getressourcestodelete(columnstodelete, rowstodelete);
                }
                for (std::map<std::pair<Core::FMTmask, Core::FMTmask>, std::vector<int>>::const_iterator it = adjacencyconstraints.begin(); it != adjacencyconstraints.end(); it++)
                {
                    rowstodelete.insert(rowstodelete.end(), it->second.begin(), it->second.end());
                }
                if (!rowstodelete.empty())
                {
                    this->deleteRows(rowstodelete.size(), &rowstodelete[0]);
                }
                if (!columnstodelete.empty())
                {
                    this->deleteCols(columnstodelete.size(), &columnstodelete[0]);
                }
                if (!rowstodelete.empty() || !columnstodelete.empty())
                {
                    this->resolvemodel();
                }
                operatingareas.clear();
                adjacencyconstraints.clear();
                }
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMToperatingareascheduler::~", __LINE__, __FILE__);
			}
		}

	std::vector<std::vector<FMToperatingareascheme>::const_iterator> FMToperatingareascheduler::setdraw()
		{
        std::vector<std::vector<FMToperatingareascheme>::const_iterator>selected;
        try{
            std::vector<std::vector<FMToperatingareascheme>::const_iterator>potentials;
            const double* upperbounds = this->getColUpper();
            const double* lowerbounds = this->getColLower();
            const double* primalsolution = this->getColSolution();
            const double* dualsolution = this->getRowActivity();
            const double* rhsupper = this->getRowUpper();
            std::vector<FMToperatingareascheme>::const_iterator areait = operatingareas.begin();
            while (areait != operatingareas.end())
                {
                if (!areait->empty() && ((useprimal && !areait->isprimalbounded(lowerbounds, upperbounds) && !areait->isallprimalbounded(lowerbounds, upperbounds))||
                    (!useprimal && !areait->isdualbounded(rhsupper))))
                    {
                    //Make sure it's sorted!
                    double value = 0;
                    if (useprimal)
                        {
                        value = areait->getbinariessum(primalsolution);
                    }else {
                        value = areait->getactivitysum(dualsolution);
                        }
                    if (!potentials.empty())
                        {
                        std::vector<std::vector<FMToperatingareascheme>::const_iterator>::iterator vit = potentials.begin();
                        size_t oldsize = potentials.size();
                        while (potentials.size() == oldsize)
                            {
                            double potentialvalue = 0;
                            if (vit != potentials.end())
                                {
                                if (useprimal)
                                    {
                                        potentialvalue = (*vit)->getbinariessum(primalsolution);
                                    }
                                    else {
                                        potentialvalue = (*vit)->getactivitysum(dualsolution);
                                    }
                                }
                            if (vit == potentials.end() || value > potentialvalue)
                                {
                                potentials.insert(vit, areait);
                                }
                                ++vit;
                            }
                        }else {
                            potentials.push_back(areait);
                            }
                    }
                ++areait;
                }
            if (proportionofset==0)
                {
                _exhandler->raise(Exception::FMTexc::FMTrangeerror,"proportion of selected operating area equal 0","FMToperatingareascheduler::setdraw",__LINE__,__FILE__);
                }
            const size_t maxareatopick = static_cast<size_t>(std::ceil(static_cast<double>(operatingareas.size()) * proportionofset));
            if (userandomness)
                {
                std::shuffle(potentials.begin(), potentials.end(), this->generator);
                }
            std::vector<std::vector<FMToperatingareascheme>::const_iterator>::iterator randomit = potentials.begin();
            while ((selected.size() < maxareatopick) && randomit != potentials.end())
                {
                selected.push_back(*randomit);
                ++randomit;
                }
        }catch(...)
            {
                _exhandler->raisefromcatch("","FMToperatingareascheduler::setdraw", __LINE__, __FILE__);
            }
		return selected;
		}

	size_t FMToperatingareascheduler::setbounds(const std::vector<std::vector<FMToperatingareascheme>::const_iterator>& tobound)
	{
		size_t gotschedule = 0;
		try{
		const double* primalsolution = this->getColSolution();
		const double* dualsolution = this->getRowActivity();
		const double* lowerprimalbounds = this->getColLower();
		const double* upperprimalbounds = this->getColUpper();
		const double* rowupperbound = this->getRowUpper();
		std::vector<int>targeteditems;
		std::vector<double>bounds;
		for (std::vector<FMToperatingareascheme>::const_iterator opit : tobound)
		{
			std::vector<FMToperatingareascheme>allneighbors;
			for (const Core::FMTmask& neighbormask : opit->getneighbors())
			{
				std::vector<FMToperatingareascheme>::const_iterator opneighbor = std::find_if(operatingareas.begin(), operatingareas.end(), FMToperatingareaschemecomparator(neighbormask));
				if (opneighbor != operatingareas.end())
				{
					allneighbors.push_back(*opneighbor);
				}

			}
			std::vector<size_t>potentialschemes;
			if (useprimal)
			{
				potentialschemes = opit->getpotentialprimalschemes(primalsolution, lowerprimalbounds, upperprimalbounds, allneighbors);
			}
			else {
				potentialschemes = opit->getpotentialdualschemes(dualsolution, rowupperbound, allneighbors);
			}
			if (!potentialschemes.empty())
			{
				if (userandomness)
				{
					std::shuffle(potentialschemes.begin(), potentialschemes.end(), this->generator);
				}
				++gotschedule;
				if (useprimal)
				{
					opit->boundprimalscheme(targeteditems, bounds, *potentialschemes.begin());
				}
				else {
					opit->unbounddualscheme(targeteditems, bounds, *potentialschemes.begin());
				}
			}
			else {
				_exhandler->raise(Exception::FMTexc::FMTignore,
					"no schedule found for Operating area "+std::string(opit->getmask()),
					"FMToperatingareascheduler::setbounds",__LINE__, __FILE__);
				if (useprimal)
				{
					opit->boundallprimalschemes(targeteditems, bounds);
				}
				else {
					opit->boundalldualschemes(targeteditems, bounds);
				}
			}
		}
		if (useprimal)
		{
			this->setColSetBounds(&targeteditems[0], &targeteditems.back() + 1, &bounds[0]);
		}else {
			this->setRowSetBounds(&targeteditems[0], &targeteditems.back() + 1, &bounds[0]);
			this->clearrowcache();
			}
		}catch(...)
            {
            _exhandler->raisefromcatch("","FMToperatingareascheduler::setbounds", __LINE__, __FILE__);
            }
		return gotschedule;
		}

	std::vector<Core::FMTyieldhandler> FMToperatingareascheduler::getsolution(const std::string& yldname) const
		{
		std::vector<Core::FMTyieldhandler>allhandlers;
		try {
            const double* primalsolution = this->getColSolution();
            const double* rowupperbound = this->getRowUpper();
            for (std::vector<FMToperatingareascheme>::const_iterator operatingareait = operatingareas.begin();
                operatingareait != operatingareas.end(); ++operatingareait)
                {
                std::vector<double>data;
                if (useprimal)
                    {
                    data=operatingareait->getprimalsolution(primalsolution);
                }else {
                    data=operatingareait->getdualsolution(rowupperbound);
                    }
                std::vector<std::string>source;
                Core::FMTyieldhandler handler(Core::FMTyldtype::FMTtimeyld, operatingareait->getmask());
                handler.push_data(yldname,Core::FMTdata(data, Core::FMTyieldparserop::FMTnone, source));
                allhandlers.push_back(handler);
                }
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMToperatingareascheduler::getsolution", __LINE__, __FILE__);
			}
		return allhandlers;
		}

	FMToperatingareascheduler::FMToperatingareascheduler(const std::vector<FMToperatingareascheme>& loperatingareas,
		const Graph::FMTgraph<Graph::FMTvertexproperties, Graph::FMTedgeproperties>& maingraph,
		const Models::FMTmodel& model,
		const Core::FMToutputnode& target,
		Models::FMTlpsolver& basesolve, size_t lseed,
		double proportionofset, bool userandomness, bool copysolver):
		FMTlpheuristic(basesolve,lseed,copysolver),operatingareas(loperatingareas),adjacencyconstraints(), proportionofset(proportionofset),
		userandomness(userandomness), useprimal(false)
		{
		try {
			if (copysolver)
				{
				FMTlpsolver::operator=(basesolve);
			}else {
				FMTlpsolver::passinsolver(basesolve);

				}
			this->setoperatingareasconstraints(maingraph, model, target);
			this->setadjacencyconstraints();
			this->resolvemodel();
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMToperatingareascheduler::FMToperatingareaheuristic", __LINE__, __FILE__);
			}
		}

	void FMToperatingareascheduler::setasrandom()
		{
		userandomness = true;
		}

	void FMToperatingareascheduler::setasprimal()
		{
		useprimal = true;
		}

	void FMToperatingareascheduler::setgeneratorseed(const size_t& lseed)
		{
		seed = lseed;
		generator.seed(lseed);
		}

}
BOOST_CLASS_EXPORT_IMPLEMENT(Heuristics::FMToperatingareascheduler)
#endif
