/*
MIT License

Copyright (c) [2019] [Bureau du forestier en chef]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifdef FMTWITHOSI
#ifndef FMTlpmodel_H_INCLUDED
#define FMTlpmodel_H_INCLUDED

#include "FMTgraphdescription.h"
#include "FMTdevelopment.h"
#include "FMTaction.h"
#include "FMTtransition.h"
#include "FMTyields.h"
#include "FMTtheme.h"
#include "FMTschedule.h"
#include "FMTutility.h"
#include "FMToutputproperties.h"
#include "FMTdevelopmentpath.h"
#include "FMTserializablematrix.h"

#ifdef FMTWITHMOSEK
	/*
	#include "OsiCpxSolverInterface.hpp"
	#include "OsiGrbSolverInterface.hpp"*/
	#include "OsiMskSolverInterface.hpp"
#endif

#include "OsiSolverInterface.hpp"
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include "FMTgraphstats.h"
#include <memory>
#include <tuple>
#include <unordered_map>
#include <map>
#include <utility>
#include <vector>
#include <queue>
#include "FMTgraph.h"
#include "FMTmodel.h"
#include "FMTsolverinterface.h"
#include "FMToperatingareaheuristic.h"


namespace Models
{
/**
enum for matrix element type  present within the matrix
elements private member of the FMTlpmodel uses those enumerators to
locate constraints/objectives/levels in the matrix for each period.
*/
enum FMTmatrixelement
	{
	goalvariable=0,//period base
	levelvariable=1,//general
	objectivevariable=2,//general
	constraint=3,//period base...
	nr_items=4
	};


/**
This model is a type III LP forest planning model. The graph
is divided per period. This model is made for replanning and simple
forest planning LP optimization. It makes heavy use of FMTgraph
for building the matrix. FMTgraph being based on a list structure
FMTlpmodel can easely be extended (by the back) using the function buildperiod() or
shrinked (by the front) using the function eraseperiod.
The matrix is held within the solverinterface pointer.
*/

class FMTlpmodel : public FMTmodel
	{
	/**
	Save and load functions are for serialization, used to do multiprocessing across multiple cpus (pickle in Pyhton)
	*/
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int version) const
		{
		ar & boost::serialization::make_nvp("model", boost::serialization::base_object<FMTmodel>(*this));
		ar & BOOST_SERIALIZATION_NVP(graph);
		ar & BOOST_SERIALIZATION_NVP(elements);
		FMTserializablematrix matrix(solverinterface, solvertype);
		ar & BOOST_SERIALIZATION_NVP(matrix);
		ar & BOOST_SERIALIZATION_NVP(deletedconstraints);
		ar & BOOST_SERIALIZATION_NVP(deletedvariables);
		}
	template<class Archive>
	void load(Archive& ar, const unsigned int version)
		{
		ar & boost::serialization::make_nvp("model", boost::serialization::base_object<FMTmodel>(*this));
		ar & BOOST_SERIALIZATION_NVP(graph);
		ar & BOOST_SERIALIZATION_NVP(elements);
		FMTserializablematrix matrix;
		ar & BOOST_SERIALIZATION_NVP(matrix);
		matrix.setsolvertype(solvertype);
		solverinterface = matrix.buildsolverinterface(solvertype, &*this->_logger);
		matrix.setmatrix(solverinterface);
		ar & BOOST_SERIALIZATION_NVP(deletedconstraints);
		ar & BOOST_SERIALIZATION_NVP(deletedvariables);
		}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	///graph holding the FMTdevelopments for all the periods.
	Graph::FMTgraph graph;
	///Solver type used maybe usefull for initialsolve or resolve to know what solver we are using to speed-up the process.
	FMTsolverinterface solvertype;
	///The osisolverinterface Abstract class (constraints/objectives/matrix ....LP) can be shared with an heuristic!
	std::shared_ptr<OsiSolverInterface>solverinterface;
	///Locations of the constraints and variables in the matrix for the constraints / objective.
	std::vector<std::unordered_map<size_t,
		std::vector<std::vector<int>>>>elements;
	///Deleted constraints used in replanning context when the constraints indexes need to be updated.
	std::vector<int>deletedconstraints;
	///Deleted variables used in replanning context when the variables indexes need to be updated.
	std::vector<int>deletedvariables;
	/**
	Simple function to summarize constraints that are un a map structure key = variables, element = coefficiant 
	to a array structure (vector) for osisolverinterface. map structure is easier to deal with thant two vectors.
	*/
	bool summarize(const std::map<int, double>& variables ,
		std::vector<int>& sumvariables, std::vector<double>& sumcoefficiants) const;
	/**
	Initialize the solverinterface called once when the FMTgraph was empty after the first call of buildperiod.
	*/
	Graph::FMTgraphstats initializematrix();
	/**
	During a call to build period after the graph has been updated with nes developments type the solverinterface matrix
	need to be updated. Variables and constraints related to each of those new developements will be added to the matrix.
	So area transfer row and natural growth plus action variables.
	*/
	Graph::FMTgraphstats updatematrix(const std::unordered_map<size_t, Graph::FMTvertex_descriptor>& targets,
			const Graph::FMTgraphstats& newstats);
	/**
	When the user add constraints using the setconstraint function or the setobjective function the model needs to had 
	variables and/or constraints to the matrix to satisfy the FMTconstraint. Each type a variable or constraint need to be added
	to the matrix the function is called and return the index of the element if it exists (already in matrix) or not (new element).
	*/
		int getsetmatrixelement(const Core::FMTconstraint& constraint,
                     const FMTmatrixelement& element_type, const std::map<int, double>& indexes,
                     int period = -1,
                     double lowerbound = COIN_DBL_MIN,double upperbound = COIN_DBL_MAX);
	/**
	Return goals (index) if it already exist within the other constraints (goals (goalsnames) can be used across multiple FMTconstraints.
	*/
        bool getgoals(const std::vector<std::string>& goalsnames, std::map<int,double>& index,const double& sense) const;
	/**
	Will check if the level (variable_level) already exist within the matrix for other constraints than the (constraint) 
	for a given period.
	*/
        int getsetlevel(const Core::FMTconstraint& constraint,const std::string& variable_level,int period);
	/**
	Return all the elements (level / constraint / variable) related to a given (constraint) for a period
	for a given period.
	*/
		std::vector<std::vector<int>>getmatrixelement(const Core::FMTconstraint& constraint,int period) const;
	/**
	Getter for a shared pointer to the matrix (solverinterface).
	*/
		std::shared_ptr<OsiSolverInterface>& getsolverinterface();
	/**
	For a given period lookup in the graph to fill the variables map (variables) for a given level (nodes).
	*/
        void locatelevels(const std::vector<Core::FMToutputnode>& nodes,int period, std::map<int, double>& variables,const Core::FMTconstraint& constraint);
	/**
	For a given period lookup in the graph to fill the variables map (variables) for a given FMTconstraints (nodes). 
	Also apply the multiplier to coefficiants of the map the map<variableindex,coefficiants>.
	*/
		bool locatenodes(const std::vector<Core::FMToutputnode>& nodes, int period, std::map<int, double>& variables,double multiplier = 1) const;
	/**
	When the eraseperiod function is called the matrix size is shrinked and the variables/constraints indexes have
	to be updated. This function use the deletedconstraints and deletedvariables private member to update the indexes of
	a given matrix elements.
	*/
		void updatematrixelements(std::vector<int>& matrixelements, const std::vector<int>& deletedelements) const;
	/**
	When the eraseperiod function is called the matrix size is shrinked and the variables/constraints indexes have
	to be updated. This function update the indexes of all the FMTconstraints of the elements calling updatematrixelements().
	*/
		void updateconstraintsmapping(const std::vector<int>& Dvariables,const std::vector<int>& Dconstraints);
	/**
	When the eraseperiod function is called the matrix size is shrinked and the variables/constraints indexes have
	to be updated. This function update the indexes of all the FMTconstraints of the elements 
	and also the FMTdevelopement constraints and variables of in the graph and delete those variables and constraints
	from the solverinterface matrix.
	*/
		bool updatematrixngraph();
	/**
	Check if the FMTconstraint had a element of (element_type) located in the matrix for a given period.
	*/
		bool ismatrixelement(const Core::FMTconstraint& constraint,
			const FMTmatrixelement& element_type, int period) const;
	/**
	Check if the requested matrix element (matrixindex) is the same as the found one (LB/UB + variables).
	*/
		bool issamematrixelement(const int& matrixindex, const FMTmatrixelement& element_type,
			const double& lowerb, const double& upperb, const std::map<int, double>& variables) const;
	/**
	Erase all constraints / variables related to a FMTconstraint for all the planning horizon (graph length).
	*/
		Graph::FMTgraphstats eraseallconstraint(const Core::FMTconstraint& constraint);
	public:
	/**
	Main constructor used to build FMTlpmodel using it's base class and to let the user choose the solvertype
	(CLP,MOSEK,GUROBI,CPLEX) see FMTsolverinterface.h for more information about the supported solvertype.
	*/
		FMTlpmodel(const FMTmodel& base, FMTsolverinterface lsolvertype);
		/**
		Default constructor of FMTlpmodel
		*/
		FMTlpmodel();
		/**
		Copy constructor of FMTlpmodel
		*/
		FMTlpmodel(const FMTlpmodel& rhs);
		/**
		Cold start of the LPsolve of a simple LP model.
		By default initialsolve will call solverinterface->initialsolve() but using the FMTsolverinterface enum
		this function will try to use the best solver parameters for a Type III Forest planning model.
		For all solvers interior point is considered the best algorith.
		*/
		bool initialsolve();
		/**
		If the user wants to set a solution for a given period for warmstarting the model or prepare to 
		bound the model to that solution.
		*/
		bool setsolution(int period, const Core::FMTschedule& schedule);
		/**
		This function bounds the primal variables to the primal solution present within the matrix for
		a given period and tolerance. Perfect function to update a FMTlpmodel or get ready for replanning.
		*/
		bool boundsolution(int period,double tolerance = FMT_DBL_TOLERANCE);
		/**
		Unbound the primal bounds of a given period.
		*/
		bool unboundsolution(int period);
		/**
		Check if FMTdevelopment area are bounded on there primal variables for a given period.
		*/
		bool isperiodbounded(int period) const;
		/**
		Get the standard solution for a given period (FMTschedule dont have natural growth solution included).
		*/
		Core::FMTschedule getsolution(int period) const;
		/**
		Get the graph stats of the graph and matrix (number of columns/rows/edges/verticies...)
		*/
		Graph::FMTgraphstats getstats() const;
		/**
		Comparison operator of FMTlpmodel
		*/
		bool operator == (const FMTlpmodel& rhs) const;
		/**
		Comparison operator of FMTlpmodel
		*/
		bool operator != (const FMTlpmodel& rhs) const;
		/**
		Get the output value of a output for a given period using the solution of the matrix.
		the map key returned consist of output name 
		if level == FMToutputlevel::standard || level == FMToutputlevel::totalonly,
		or developement name if level == FMToutputlevel::developpement
		*/
		std::map<std::string, double> getoutput(const Core::FMToutput& output,
			int period, Graph::FMToutputlevel level = Graph::FMToutputlevel::standard);
		/**
		This function is the main function used to build the graph and the matrix.
		A call to that function add a period within the graph and the matrix of the FMTlpmodel.
		If the schedule is not empty than the model is going to be generated in partialbuild mode and 
		not fullbuild mode. Partialbuild will only build the graph linked to the solution. make it perfect to
		reinterpret outputs for a given solution without rebuilding the whole graph.
		*/
		Graph::FMTgraphstats buildperiod(Core::FMTschedule schedule = Core::FMTschedule(),
			bool forcepartialbuild = false);
		/**
		This function set the objective of the matrix for the whole planning horizon (graph length).
		Note that the objective function is always the firts constraint in the constraints vector of a 
		FMTmodel. If a objective was already set before it will replace it when calling this function.
		*/
		Graph::FMTgraphstats setobjective(const Core::FMTconstraint& objective);
		/**
		This function set a constraint in the matrix for the whole planning horizon (graph length).
		If the function is recalled if the constraint already exist in the matrix in wont be replaced.
		Can be called after calling builperiod in replanning.
		*/
		Graph::FMTgraphstats setconstraint(const Core::FMTconstraint& constraint);
		/**
		Erase a constraint for a given period in the matrix and in the model elements.
		*/
		Graph::FMTgraphstats eraseconstraint(const Core::FMTconstraint& constraint, int period);
		/**
		Get the variability of multiple outputs for a given tolerance across the planning horizon.
		Need to call this function after initialsolve.
		The actual objective function is transform to a constraint and then each output 
		are minimized and maximized to get their actual range for each period.
		the returned map key are the name of the output plus UB for upper bound (maximization) and
		LB for lower bound (minimization). This function  do alot of resolve.
		*/
		std::map<std::string, std::vector<double>>getvariabilities(const std::vector<Core::FMToutput>& outputs,
																double tolerance = FMT_DBL_TOLERANCE);
		/**
		When doing replanning or simply model update the user may want to delete the first period (front) 
		of the graph and the matrix to get a FMTmodel - first period. The planning length will be shrinked to 
		originalsize - 1.
		*/
		Graph::FMTgraphstats eraseperiod();
		/**
		Return the first active period should be always 0 in case or planning. 
		But when eraseperiod is called the first active period is going to move to 1 and so on.
		*/
		int getfirstactiveperiod() const;
		/**
		Using multiple operating areas and a simple output node a MIP formulation (using the BFECopt heuristic) is done using the matrix of 
		the FMTlpmodel. The matrix can be copied within the operatingareaheuristic or directly uses the matrix
		of the FMTlpmodel (if numberofheuristics == 1 and copysolver == true). The user can than decide to change
		some parameters of heuristics that are in the vectors.
		*/
		std::vector<Heuristics::FMToperatingareaheuristic>getoperatingareaheuristics(const std::vector<Heuristics::FMToperatingarea>& opareas,
																				const Core::FMToutputnode& node,
																				size_t numberofheuristics=1,
																				bool copysolver=true);
		/**
		Get the objective value of the solved matrix.
		*/
		double getObjValue() const;
		/**
		By default call solverinterface->resolve() when some changes are done to the model.
		The user dont necessery need the call initialsolve every time the matrix has changed a call to resolve maybe enought.
		*/
		bool resolve();
		/**
		Write the solverinterface matrix to a file (location) using the lp formulation.
		*/
		void writeLP(const std::string& location) const;
		/**
		Write the solverinterface matrix to a file (location) using the MPS formulation.
		*/
		void writeMPS(const std::string& location) const;
		/**
		Copy assignment of FMTlpmodel
		*/
		FMTlpmodel& operator = (const FMTlpmodel& rhs);
		/**
		Default destructor of FMTlpmodel
		*/
		~FMTlpmodel() = default;
	};

}


#endif
#endif