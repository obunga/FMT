/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifdef FMTWITHOSI

#include "FMToperatingareaclusterer.h"
#include <random>



namespace Heuristics
{
	FMToperatingareaclusterer::FMToperatingareaclusterer(const Models::FMTsolverinterface& interfacetype,const size_t& lseed,const std::vector<FMToperatingareacluster>& lclusters):
	    FMTlpheuristic(interfacetype,lseed),
		clusters(lclusters),
		numberofsimulationpass(100)
	{

	}
	void FMToperatingareaclusterer::setallinteger()
		{
		try {
		    const std::vector<int>integervariables = this->getbinariesvariables();
            this->setInteger(&integervariables[0], static_cast<int>(integervariables.size()));
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::setallinteger", __LINE__, __FILE__);
			}
		}

    void FMToperatingareaclusterer::unboundall()
		{
		try {
		    const std::vector<int>integervariables = this->getbinariesvariables();
		    std::vector<double>colsbounds;
		    for (const int& variable : integervariables)
                {
                colsbounds.push_back(0);
                colsbounds.push_back(1);
                }
            this->setColSetBounds(&integervariables[0],&integervariables.back() + 1,&colsbounds[0]);
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::unboundall", __LINE__, __FILE__);
			}
		}

    double FMToperatingareaclusterer::getspreadprobability(const std::vector<FMToperatingareaclusterbinary>& incluster,const FMToperatingareaclusterbinary& target) const
        {
        double maxdifference = 0;
        try {
        for (const FMToperatingareaclusterbinary& binary : incluster)
            {
            const double difference = std::abs(binary.getstatistic()-target.getstatistic());
            if (difference > maxdifference)
                {
                maxdifference = difference;
                }

            }
        }catch(...)
            {
            _exhandler->raisefromcatch("", "FMToperatingareaclusterer::getspreadprobability", __LINE__, __FILE__);
            }
        return (1-maxdifference);
        }

   double FMToperatingareaclusterer::gettargetedoperatingareasize(const FMToperatingareacluster& target)
        {
        double returnedsize = 0;
        try{
            double minimaltarget = target.getminimalarea();
            double maximaltarget = target.getmaximalarea();
            std::uniform_real_distribution<double>areadistribution(minimaltarget,maximaltarget);
            returnedsize = areadistribution(generator);
        }catch(...)
            {
            _exhandler->raisefromcatch("", "FMToperatingareaclusterer::gettargetedoperatingareasize", __LINE__, __FILE__);
            }
        return returnedsize;
        }

    bool FMToperatingareaclusterer::spread(const FMToperatingareacluster& ignition,std::vector<FMToperatingareaclusterbinary>& assigned)
        {
        try{
            return (std::find_if(assigned.begin(),assigned.end(),FMToperatingareacomparator(ignition.getcentroid()))!=assigned.end());
            double firesize = ignition.getcentroid().getarea();
            std::vector<FMToperatingareaclusterbinary>incluster(1,ignition.getcentroid());
            std::vector<FMToperatingareaclusterbinary>outcluster;
            std::vector<FMToperatingareaclusterbinary>actives;
            for (const FMToperatingareaclusterbinary& binary : ignition.getbinaries())
                {
                if (std::find_if(assigned.begin(),assigned.end(),FMToperatingareacomparator(binary))==assigned.end())
                    {
                    if (binary.getneighbors().empty())
                        {
                        actives.push_back(binary);
                    }else{
                        outcluster.push_back(binary);
                        }
                    }
                }
            const double maximalfiresize = this->gettargetedoperatingareasize(ignition);
            while(!actives.empty() && firesize < maximalfiresize)
                {
                    std::vector<double>probabilities;
                    double totaldifference = 0;
                    for (const FMToperatingareaclusterbinary& active : actives)
                        {
                        const double difference = getspreadprobability(incluster,active);
                        totaldifference+=difference;
                        probabilities.push_back(difference);
                        }
                    std::vector<int>intprobability;
                    for(const double& probability : probabilities)
                        {
                        intprobability.push_back(static_cast<int>(probability/totaldifference)*100);
                        }
                    std::discrete_distribution<int>spreaddistribution(intprobability.begin(),intprobability.end());
                    const int selection = spreaddistribution(generator);
                    const FMToperatingareaclusterbinary selected = actives.at(selection);
                    actives.erase(actives.begin()+selection);
                    incluster.push_back(selected);
                    firesize+=selected.getarea();
                    std::vector<FMToperatingareaclusterbinary>updatedoutcluster;
                    for (const FMToperatingareaclusterbinary& outbinary : outcluster)
                        {
                        const std::vector<Core::FMTmask>neighbors = outbinary.getneighbors();
                        if (std::find_if(neighbors.begin(),neighbors.end(),Core::FMTmaskcomparator(selected.getmask()))!=neighbors.end())
                            {
                                int fullsize = static_cast<int>(neighbors.size());
                                for (const FMToperatingareaclusterbinary& inbinary : incluster)
                                    {
                                    if (std::find_if(neighbors.begin(),neighbors.end(),Core::FMTmaskcomparator(inbinary.getmask()))!=neighbors.end())
                                        {
                                        --fullsize;
                                        }

                                    }
                                if(fullsize == 0)
                                    {
                                    actives.push_back(outbinary);
                                    }else{
                                    updatedoutcluster.push_back(outbinary);
                                    }

                            }else{
                            updatedoutcluster.push_back(outbinary);
                            }
                        }
                    outcluster = updatedoutcluster;
                    std::vector<double>bounds;
                    std::vector<int>indexes;
                    if (ignition.isvalidarea(firesize))
                        {
                        for (const FMToperatingareaclusterbinary& inbinary : incluster)
                            {
                            assigned.push_back(inbinary);
                            bounds.push_back(1.0);
                            bounds.push_back(1.0);
                            indexes.push_back(inbinary.getvariable());
                            }
                        this->setColSetBounds(&indexes[0],&indexes.back() + 1,&bounds[0]);
                        return true;
                        }
                    }
        }catch(...)
            {
            _exhandler->raisefromcatch("", "FMToperatingareaclusterer::spread", __LINE__, __FILE__);
            }
        return false;
        }

    void FMToperatingareaclusterer::setnumberofsimulationpass(const int& pass)
        {
        numberofsimulationpass = pass;
        }

    std::vector<int>FMToperatingareaclusterer::getbinariesvariables() const
        {
        std::vector<int>varindexes;
        for (const FMToperatingareacluster& cluster : clusters)
            {
            varindexes.push_back(cluster.getcentroid().getvariable());
            for (const FMToperatingareaclusterbinary& binary : cluster.getbinaries())
                {
                varindexes.push_back(binary.getvariable());
                }
            }
        return varindexes;
        }

    bool FMToperatingareaclusterer::initialsolve()
        {
        try{
            double passleft =  numberofsimulationpass;
            if (Models::FMTlpsolver::initialsolve())
                {
                double bestobjectivevalue = 0;
                std::vector<double>bestcolsbound;
                const std::vector<int>varindexes = this->getbinariesvariables();
                while(passleft>0)
                    {
                    this->unboundall();
                    std::vector<FMToperatingareaclusterbinary>assigned;
                    if (Models::FMTlpsolver::resolve())
                        {
                        std::vector<FMToperatingareacluster>clustertospread=clusters;
                        std::shuffle(clustertospread.begin(),clustertospread.end(),generator);
                        size_t iterationdone = 0;
                        while(!clustertospread.empty() && iterationdone < (clusters.size()*2))
                            {
                            if (this->spread(*clustertospread.begin(),assigned))
                                {
                                clustertospread.erase(clustertospread.begin());
                                }
                            ++iterationdone;
                            }
                        if (clustertospread.empty() && Models::FMTlpsolver::resolve() &&
                            (Models::FMTlpsolver::getObjSense()*Models::FMTlpsolver::getObjValue()) < bestobjectivevalue)
                            {
                            bestcolsbound.clear();
                            const double* upperbound = Models::FMTlpsolver::getColUpper();
                            const double* lowerbound = Models::FMTlpsolver::getColLower();
                            for (const FMToperatingareacluster& cluster : clusters)
                                {
                                const int centroidvar = cluster.getcentroid().getvariable();
                                bestcolsbound.push_back(*(lowerbound+centroidvar));
                                bestcolsbound.push_back(*(upperbound+centroidvar));
                                for (const FMToperatingareaclusterbinary& binary : cluster.getbinaries())
                                    {
                                    const int binaryvar = binary.getvariable();
                                    bestcolsbound.push_back(*(lowerbound+centroidvar));
                                    bestcolsbound.push_back(*(upperbound+centroidvar));
                                    }
                                }
                            bestobjectivevalue = (Models::FMTlpsolver::getObjSense()*Models::FMTlpsolver::getObjValue());
                            (*_logger) << "Feasible solution found objective: " + std::to_string(bestobjectivevalue)<< "\n";
                            }
                        }
                    --passleft;
                    }
                if (bestcolsbound.empty())
                    {
                    this->unboundall();
                    return false;
                    }else{
                        this->setColSetBounds(&varindexes[0],&varindexes.back() + 1,&bestcolsbound[0]);
                    return Models::FMTlpsolver::resolve();
                    }
                }
            }catch(...)
                {
                _exhandler->printexceptions("", "FMToperatingareaclusterer::initialsolve", __LINE__, __FILE__);
                }
        return false;
        }

    std::vector<FMToperatingareacluster>FMToperatingareaclusterer::getsolution() const
        {
        std::vector<FMToperatingareacluster>solution;
        try{
            const double* primalsolution = Models::FMTlpsolver::getColSolution();
            for (const FMToperatingareacluster& cluster : clusters)
                {
                if (*(primalsolution+cluster.getcentroid().getvariable())>0.5)//active cluster
                    {
                    std::vector<FMToperatingareaclusterbinary>selectedbinaries;
                    for (const FMToperatingareaclusterbinary& binary : cluster.getbinaries())
                        {
                        if (*(primalsolution+binary.getvariable())>0.5)//active binary
                            {
                            selectedbinaries.push_back(binary);
                            }
                        }
                    solution.push_back(FMToperatingareacluster(FMToperatingareacluster(cluster.getcentroid(),selectedbinaries),cluster.getrealminimalarea(),cluster.getrealmaximalarea()));
                    }
                }
        }catch(...)
            {
            _exhandler->printexceptions("", "FMToperatingareaclusterer::getsolution", __LINE__, __FILE__);
            }
        return solution;
        }


	void FMToperatingareaclusterer::buildclustersvariables()
		{
		try {
			std::vector<FMToperatingareacluster>newclusterswithvariables;
			std::vector<int>variablesadded;
			int variableid = 0;
			for (const FMToperatingareacluster& cluster : clusters)
				{
				FMToperatingareaclusterbinary newcentroid = cluster.getcentroid();
				newcentroid.setvariable(variableid);
				this->addCol(0, nullptr, nullptr, 0.0, 1.0, 0.0);
				variablesadded.push_back(variableid);
				++variableid;
				std::vector<FMToperatingareaclusterbinary>newbinaries = cluster.getbinaries();
				for (FMToperatingareaclusterbinary& neighbor : newbinaries)
					{
					neighbor.setvariable(variableid);
					variablesadded.push_back(variableid);
					this->addCol(0, nullptr, nullptr, 0.0, 1.0, 0.0);
					++variableid;
					}
				newclusterswithvariables.push_back(FMToperatingareacluster(FMToperatingareacluster(newcentroid, newbinaries),cluster.getminimalarea(),cluster.getmaximalarea()));
				}
			clusters = newclusterswithvariables;
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::buildclustersvariables", __LINE__, __FILE__);
			}

		}

	void FMToperatingareaclusterer::addmaxminobjective(const FMToperatingareacluster& cluster, const FMToperatingareaclusterbinary& binary,
		const std::vector<FMToperatingareaclusterbinary>& binaries)
	{
		try {
			std::vector<double>maxvariables;
			maxvariables.push_back(binary.getstatistic());
			maxvariables.push_back(-1.0);
			std::vector<int>indexesofmaxvariables;
			indexesofmaxvariables.push_back(binary.getvariable());
			indexesofmaxvariables.push_back(cluster.getmaximalobjectivevariable());
			this->addRow(2, &indexesofmaxvariables[0], &maxvariables[0], -COIN_DBL_MAX,1.0);
			std::vector<double>minvariables;
			std::vector<int>indexesofminvariables;
			for (const FMToperatingareaclusterbinary& localbinary : binaries)
				{
				if (binary.getvariable()!=localbinary.getvariable())
					{
					minvariables.push_back(-1.0);
					indexesofminvariables.push_back(localbinary.getvariable());
				}else {
					minvariables.push_back(-binary.getstatistic());
					indexesofminvariables.push_back(binary.getvariable());
					}
				}
			minvariables.push_back(-1.0);
			indexesofmaxvariables.push_back(cluster.getminimalobjectivevariable());
			this->addRow(static_cast<int>(indexesofminvariables.size()), &indexesofminvariables[0], &minvariables[0], -COIN_DBL_MAX, 1.0);
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::addmaxminobjective", __LINE__, __FILE__);
			}
		}

    std::map<Core::FMTmask,std::vector<FMToperatingareaclusterbinary>>FMToperatingareaclusterer::getallbinaries() const
        {
        std::map<Core::FMTmask,std::vector<FMToperatingareaclusterbinary>>allbinaries;
        try {
            for (const FMToperatingareacluster& cluster : clusters)
                {
                if (allbinaries.find(cluster.getcentroid().getmask())!=allbinaries.end())
                    {
                    allbinaries[cluster.getcentroid().getmask()].push_back(cluster.getcentroid());
                    }else{
                    allbinaries[cluster.getcentroid().getmask()]=std::vector<FMToperatingareaclusterbinary>(1,cluster.getcentroid());
                    }
                for (const FMToperatingareaclusterbinary& binary : cluster.getbinaries())
                    {
                    if (allbinaries.find(binary.getmask())!=allbinaries.end())
                        {
                        allbinaries[binary.getmask()].push_back(binary);
                        }else{
                        allbinaries[binary.getmask()]=std::vector<FMToperatingareaclusterbinary>(1,binary);
                        }
                    }
                }
        }catch (...)
            {
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::getallbinaries", __LINE__, __FILE__);
            }
        return allbinaries;
        }

	void FMToperatingareaclusterer::addobjective()
		{
		try {
		    std::map<Core::FMTmask,std::vector<FMToperatingareaclusterbinary>>choices = this->getallbinaries();
			for (FMToperatingareacluster& cluster : clusters)
                {
				int columnid = this->getNumCols();
				cluster.setmaximalobjectivevariable(columnid);
				this->addCol(0, nullptr, nullptr,0.0, 1.0, 1.0);
				++columnid;
				cluster.setminimalobjectivevariable(columnid);
				this->addCol(0, nullptr, nullptr,-1.0,0, 1.0);
				std::vector<double>variableclosed(2,-1);
				std::vector<int>columnidclosed;
				columnidclosed.push_back(cluster.getcentroid().getvariable());
				columnidclosed.push_back(cluster.getminimalobjectivevariable());
				this->addRow(2, &columnidclosed[0], &variableclosed[0],-COIN_DBL_MAX, 0.0);
				this->addmaxminobjective(cluster, cluster.getcentroid(), cluster.getbinaries());
				for (const FMToperatingareaclusterbinary& binary : cluster.getbinaries())
					{
                    this->addmaxminobjective(cluster,binary,choices.at(binary.getmask()));
					}
                }
			this->setObjSense(1.0);
		}catch (...)
            {
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::addobjective", __LINE__, __FILE__);
            }
		}

    void FMToperatingareaclusterer::addlinksrows()
        {
        try {
            for (const FMToperatingareacluster& cluster : clusters)
                {
                const std::vector<FMToperatingareaclusterbinary>clusterbinaries = cluster.getbinaries();
                for (const FMToperatingareaclusterbinary& binary : clusterbinaries)
                    {
                    std::vector<double>linkvalues;
                    std::vector<int>linkindex;
                    linkvalues.push_back(-(static_cast<double>(binary.getneighbors().size())+1));
                    linkindex.push_back(binary.getvariable());
                    linkvalues.push_back(1.0);
                    linkindex.push_back(cluster.getcentroid().getvariable());
                    for (const Core::FMTmask& neighbor : binary.getneighbors())
                        {
                        std::vector<FMToperatingareaclusterbinary>::const_iterator neighborit = std::find_if(clusterbinaries.begin(),clusterbinaries.end(),FMToperatingareacomparator(neighbor));
                        linkvalues.push_back(1.0);
                        linkindex.push_back(neighborit->getvariable());
                        }
                    this->addRow(static_cast<int>(linkindex.size()),&linkindex[0],&linkvalues[0],0);
                    }

                }
        }catch (...)
			{
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::addlinksrows", __LINE__, __FILE__);
			}
        }

    void FMToperatingareaclusterer::addforcingrows()
        {
        try {
		    std::map<Core::FMTmask,std::vector<FMToperatingareaclusterbinary>>choices = this->getallbinaries();
		    for (std::map<Core::FMTmask,std::vector<FMToperatingareaclusterbinary>>::const_iterator binit = choices.begin();binit!=choices.end();binit++)
                {
                std::vector<double>values;
                std::vector<int>indexes;
                for(const FMToperatingareaclusterbinary& neighbor : binit->second)
                    {
                    values.push_back(1.0);
                    indexes.push_back(neighbor.getvariable());
                    }
                this->addRow(static_cast<int>(indexes.size()),&indexes[0],&values[0],1.0,1.0);
                }

		}catch (...)
            {
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::addforcingrows", __LINE__, __FILE__);
            }

        }


	void FMToperatingareaclusterer::addareaconstraints()
		{
		try {
			for (const FMToperatingareacluster& cluster : clusters)
				{
                std::vector<double>maxvalues(1,cluster.getcentroid().getarea());
                std::vector<int>maxindexes(1,cluster.getcentroid().getvariable());
                std::vector<double>minvalues(1,-cluster.getminimalarea()+cluster.getcentroid().getarea());
                std::vector<int>minindexes(1,cluster.getcentroid().getvariable());
                for (const FMToperatingareaclusterbinary& binary : cluster.getbinaries())
                    {
                    maxvalues.push_back(binary.getarea());
                    maxindexes.push_back(binary.getvariable());
                    minvalues.push_back(binary.getarea());
                    minindexes.push_back(binary.getvariable());
                    }
                if(cluster.getminimalarea()>0)
                    {
                    this->addRow(static_cast<int>(minvalues.size()),&minindexes[0],&minvalues[0],0);
                    }
                if (cluster.getmaximalarea()>0)
                    {
                    this->addRow(static_cast<int>(maxvalues.size()),&maxindexes[0],&maxvalues[0],-COIN_DBL_MAX,cluster.getmaximalarea());
                    }

				}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMToperatingareaclusterer::addareaconstraints", __LINE__, __FILE__);
			}
		}

    void FMToperatingareaclusterer::branchnboundsolve()
        {
        try{
            if (this->isProvenOptimal())
                {
                //In that order it seems to work...
                this->setallinteger();
                this->branchAndBound();
                this->unboundall();
                this->branchAndBound();
                }
        }catch(...)
            {
            _exhandler->printexceptions("", "FMToperatingareaclusterer::branchnboundsolve", __LINE__, __FILE__);
            }
        }

   void FMToperatingareaclusterer::buildproblem()
        {
        try{
            this->addobjective();
            this->addlinksrows();
            this->addforcingrows();
            this->addareaconstraints();
         }catch(...)
            {
            _exhandler->raisefromcatch("", "FMToperatingareaclusterer::buildproblem", __LINE__, __FILE__);
            }
        }

}
BOOST_CLASS_EXPORT_IMPLEMENT(Heuristics::FMToperatingareaclusterer)
#endif
