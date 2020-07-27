/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifdef FMTWITHOSI

#ifndef FMToperatingareascheduler_H_INCLUDED
#define FMToperatingareascheduler_H_INCLUDED

#include "FMTmask.h"
#include "FMTyieldhandler.h"
#include "FMToperatingareascheme.h"
#include "FMTgraph.h"
#include "FMToutput.h"
#include "FMTmodel.h"
#include "OsiSolverInterface.hpp"
#include "FMTserializablematrix.h"
#include "FMTsolverinterface.h"
#include "FMToutputnode.h"
#include "FMTtheme.h"
#include "FMTyieldhandler.h"
#include "FMTlpheuristic.h"
#include <random>
#include <memory>
#include <map>
#include <vector>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/export.hpp>


namespace Heuristics
{
	// DocString: FMToperatingareascheduler
	/**
	FMToperatingareaheuristic is a heuristics made to solve spatialization problem 
	across landscape for multiple operating areas. MIP Forest planning problem tend to be 
	symmetrical and tought to solve using a regular MIP solver. This heuristics is made to quickly 
	generate good enought starting solution for those kind of Forest management problem.
	Before using this class the user must have sets all parameters of a vector of FMToperatingarea
	so that the heuristic can sets the constraints and variables of each operating area into
	the matrix. It's up to the user to decide to just generate a good initialsolution or 
	generate a good initialsolution and then try to find the optimaly using a BnB solver.
	*/
	class FMToperatingareascheduler : public FMTlpheuristic
	{
		// DocString: FMToperatingareaheuristic::save
		/**
		Save function is for serialization, used to do multiprocessing across multiple cpus (pickle in Pyhton)
		*/
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("lpsolve", boost::serialization::base_object<FMTlpheuristic>(*this));
			ar & BOOST_SERIALIZATION_NVP(operatingareas);
			ar & BOOST_SERIALIZATION_NVP(adjacencyconstraints);
			ar & BOOST_SERIALIZATION_NVP(proportionofset);
			ar & BOOST_SERIALIZATION_NVP(userandomness);
			ar & BOOST_SERIALIZATION_NVP(useprimal);
		}
		// DocString: FMToperatingareascheduler::operatingareas
		///Vector of FMToperatingarea holding all informations about each operating area coming from the user
		std::vector<FMToperatingareascheme>operatingareas;
		// DocString: FMToperatingareascheduler::adjacencyconstraints
		///Adjacency constraints indexes of all operating area the pair is used to not duplicate neighboring constraints
		std::map<std::pair<Core::FMTmask, Core::FMTmask>, std::vector<int>>adjacencyconstraints;
		// DocString: FMToperatingareascheduler::proportionofset
		///This data member gives the information about the proportion of operating area status change allowed before resolving the problem
		double proportionofset;
		// DocString: FMToperatingareascheduler::userandomness
		///If true the heuristic is going to shuffle the potential oprating area schemes and area schemes before fixing the bounds.
		bool userandomness;
		// DocString: FMToperatingareascheduler::useprimal
		///If true the heuristic will solve try to solve the problem by the primal variables else by the dual constraints.
		bool useprimal;
		// DocString: FMToperatingareascheduler::setoperatingareasconstraints
		/**
		The function sets all the operating area constraints (changes the stade of the operating area) and adding all constraints
		and variables to the solverinterface using the FMTlpmodel graph (maingraph), it's parent class (model) and finaly using 
		an output node (the output node needs to be linked to a least one action of the FMTmodel).
		*/
		void setoperatingareasconstraints(const Graph::FMTgraph& maingraph,
			const Models::FMTmodel& model,
			const Core::FMToutputnode& target);
		// DocString: FMToperatingareascheduler::setadjacencyconstraints
		/**
		The function fills up the adjacencyconstraints data member and add these constraints to the solverinterface.
		Adjacency constraints cannot be added from each operating area thats why the constraints are directly linked to
		the FMToperatingareaheuristic and the FMToperatingarea.
		*/
		void setadjacencyconstraints();
		// DocString: FMToperatingareascheduler::setdraw
		/**
		Returns a vector of potential oeprating areas that have no schedule sets yet. Based on the dual or primal solution if useprimal == true.
		Depending if userandomness == false the draw is going to be sorted starting by the best operating area to set schedule
		looking at the actual primal or dual solution. If userrandomness == true the draw is going to be shuffled
		*/
		std::vector<std::vector<FMToperatingareascheme>::const_iterator> setdraw();
		// DocString: FMToperatingareascheduler::setbounds
		/**
		Depending if we solve the dual or the primal the this function will set a  random schedule if userrandomness == true or 
		simply set the best known schedule to a set of operating areas (tobound).
		*/
		size_t setbounds(const std::vector<std::vector<FMToperatingareascheme>::const_iterator>& tobound);
		// DocString: FMToperatingareascheduler::unboundall
		/**
		This function is like a reset button, it will unselect all schedules already selected for each management units
		and unbound all variables/constraints related to all operating areas in the solverinterface.
		*/
		void unboundall(bool atprimal = false);
		// DocString: FMToperatingareascheduler::closeprimalbounds
		/**
		Sets all binary variables of all operating area to 0 by fixing theirs primal bounds.
		*/
		void closeprimalbounds();
		// DocString: FMToperatingareascheduler::setallinteger
		/**
		After solving the heuristic with initialsolve() the user may want to try to get a optimal solution or to
		valide the actual solution using BnB. This function sets all schemes variables or each operating area
		as integer variables within the solverinterface and prepare the matrix for Bnb if the solve was in dual.
		*/
		void setallinteger();
		// DocString: FMToperatingareascheduler::resolvemodel
		/**
		The function call solverinterface->resolve() by default but depending of the solvertype used it
		may use different algorithm or parameters to get a fast resolve of the problem.
		*/
		int resolvemodel();
		// DocString: FMToperatingareascheduler::clearrowcache
		/**
		Function only usefull for when solvertype == mosek the row cache doesn't seems to handle realy well the heuristic.
		So the row memory cache needs to be cleared.
		*/
		void clearrowcache();
	public:
		// DocString: FMToperatingareascheduler::initialsolve
		/**
		Solve the heuristic problem using the original heuristic resolving the problem till finding a initial solution
		for each operating area. The user can use the function getsolution to first yield solution. 
		*/
		bool initialsolve() final;
		// DocString: FMToperatingareascheduler::branchnboundsolve
		/**
		Solve problem using Branch and bound on the primal formulation. If the function is called after a call to initialsolve()
		it's going to use the heuristic solution has a starting MIP solution, if not it's going to directly use the BnB on the formulated problem.
		*/
		void branchnboundsolve();
		// DocString: FMToperatingareascheduler::setasrandom
		/**
		Sets True the userandomness data member
		*/
		void setasrandom();
		// DocString: FMToperatingareascheduler::setasprimal
		/**
		Sets True the useprimal data member
		*/
		void setasprimal();
		// DocString: FMToperatingareascheduler::setgeneratorseed
		/**
		Setter for the seed data member
		*/
		void setgeneratorseed(const size_t& lseed);
		// DocString: FMToperatingareascheduler::getsolution
		/**
		Gets the actual solution into a FMTyieldhandler format (time yield) 
		The user can decide the (yldname) to use. Each operating area will have its own
		yieldhandler.
		*/
		std::vector<Core::FMTyieldhandler> getsolution(const std::string& yldname) const;
		// DocString: FMToperatingareascheduler(const std::vector<FMToperatingarea>,const Graph::FMTgraph,const Models::FMTmodel,const Core::FMToutputnode,std::shared_ptr<OsiSolverInterface>,const Models::FMTsolverinterface,size_t lseed,double proportionofset,bool userandomness,bool copysolver)
		/**
		Main constructor used to initialize a FMToperatingareaheuristic, the constructor needs
		alot of information comming from a FMTlpmodel. Also constructing a FMToperatingareaheuristic is
		not a small task if copysolver = true because the solverinterface beging the FMTlpmodel needs
		to be copied. After constructing the object the operatingareaheuristic doesn't need any information
		about the FMTlpmodel.
		*/
		FMToperatingareascheduler(const std::vector<FMToperatingareascheme>& loperatingareas,
			const Graph::FMTgraph& maingraph,
			const Models::FMTmodel& model,
			const Core::FMToutputnode& target,
			Models::FMTlpsolver& basesolve, size_t lseed = 0,
			double proportionofset = 0.25, bool userandomness = false, bool copysolver = true);
		// DocString: FMToperatingareascheduler()
		/**
		Default FMToperatingareaheuristic constructor
		*/
		FMToperatingareascheduler()=default;
		// DocString: FMToperatingareascheduler(const FMToperatingareaheuristic)
		/**
		FMToperatingareaheuristic copy constructor
		*/
		FMToperatingareascheduler(const FMToperatingareascheduler& rhs);
		// DocString: FMToperatingareascheduler::operator=
		/**
		FMToperatingareaheuristic copy assignment
		*/
		FMToperatingareascheduler& operator = (const FMToperatingareascheduler& rhs);
		// DocString: FMToperatingareascheduler::isfeasible
		/**
		Return true if the actual solution of the heuristic is feasible.
		*/
		~FMToperatingareascheduler();
	};
}
BOOST_CLASS_EXPORT_KEY(Heuristics::FMToperatingareascheduler)
#endif
#endif