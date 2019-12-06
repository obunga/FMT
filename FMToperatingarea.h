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

#ifndef FMToperatingarea_H_INCLUDED
#define FMToperatingarea_H_INCLUDED

#include "FMTgraphdescription.h"
#include "FMTgraph.h"
#include "FMTmask.h"
#include "OsiSolverInterface.hpp"
#include <vector>
#include <memory>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

namespace Heuristics
	{
	class FMToperatingarea
		{
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
			{
			ar & BOOST_SERIALIZATION_NVP(mask);
			ar & BOOST_SERIALIZATION_NVP(neighbors);
			ar & BOOST_SERIALIZATION_NVP(openingconstraints);
			ar & BOOST_SERIALIZATION_NVP(openingbinaries);
			ar & BOOST_SERIALIZATION_NVP(maximalschemesconstraint);
			ar & BOOST_SERIALIZATION_NVP(schemesperiods);
			ar & BOOST_SERIALIZATION_NVP(openingtime);
			ar & BOOST_SERIALIZATION_NVP(returntime);
			ar & BOOST_SERIALIZATION_NVP(repetition);
			ar & BOOST_SERIALIZATION_NVP(greenup);
			ar & BOOST_SERIALIZATION_NVP(openingratio);
			ar & BOOST_SERIALIZATION_NVP(neihgborsperimeter);
			}
		FMTmask mask;
		vector<FMTmask>neighbors;
		vector<int>openingconstraints;
		vector<int>openingbinaries;
		int maximalschemesconstraint;
		vector<vector<int>>schemesperiods;
		size_t openingtime,returntime,repetition,greenup;
		double openingratio,neihgborsperimeter;
		double getarea(const double* primalsolution, const Graph::FMTgraph& maingraph, const vector<Graph::FMTvertex_descriptor>& verticies) const;//Get the area of the operating area base on a solution
		size_t getbestschemeid(const double* primalsolution) const;//Get the best possible scheme looking at the primal solution
		vector<vector<vector<Graph::FMTvertex_descriptor>>> generateschemes(const vector<vector<Graph::FMTvertex_descriptor>>& verticies); // Generate unique schemes base on parameters
		void schemestoLP(const vector<vector<vector<Graph::FMTvertex_descriptor>>>& schemes, 
						const vector<vector<Graph::FMTvertex_descriptor>>& periodics,
						std::shared_ptr<OsiSolverInterface> solverinterface,
						const Graph::FMTgraph& maingraph,const vector<int>& actionIDS); //Fill opening constraints and opening binairies in the LP and in the OParea
		vector<vector<int>> schemestoperiods(const vector<vector<vector<Graph::FMTvertex_descriptor>>>& schemes, const Graph::FMTgraph& maingraph) const;
		public:
			vector<FMTmask> getneighbors() const;
			const vector<int>& getopeningbinaries() const;
			double getbinariessum(const double* primalsolution) const;
			map<int, vector<int>> getcommonbinairies(const FMToperatingarea& neighbor) const;
			size_t getsolutionindex(const double* primalsolution) const; //return -1 if no binary use
			bool havebinarysolution(const double* primalsolution) const;
			bool isallbounded(const double* lowerbounds, const double* upperbounds) const;
			bool isbounded(const double* lowerbounds, const double* upperbounds) const;
			vector<size_t>getpotentialschemes(const double* primalsolution,const vector<FMToperatingarea>& neighbors) const;
			void getressourcestodelete(vector<int>& colstodelete,vector<int>& rowstodelete) const; //Remove all variable and constraints related to the operating area from the solver
			size_t binarize(std::shared_ptr<OsiSolverInterface> solverinterface) const; //Set all opening binairies to integer variable to get ready for branch and bound
			size_t unboundallschemes(std::shared_ptr<OsiSolverInterface> solverinterface) const; //Unbound all binairies to 0<=B<=1
			size_t boundallschemes(std::shared_ptr<OsiSolverInterface> solverinterface) const;
			bool boundscheme(std::shared_ptr<OsiSolverInterface> solverinterface, const size_t& schemeid) const; //Looking at the primal solution set the best scheme to the solverinterface 1<=B<=1 and check optimality
			vector<double> getsolution(const double* primalsolution) const; //Get the solution into yields
			void setneighbors(const vector<FMTmask>& lneighbors);
			void setconstraints(const vector<vector<Graph::FMTvertex_descriptor>>& verticies,
				const Graph::FMTgraph& graph,std::shared_ptr<OsiSolverInterface> solverinterface,
				const vector<int>& actionIDS);
			FMToperatingarea(const FMTmask& lmask,const size_t& lopeningtime, const size_t& lreturntime,
				const size_t& lrepetition, const size_t& lgreenup,
				const double& lopeningratio, const double& lneihgborsperimeter);
			FMToperatingarea()=default;
			FMToperatingarea(const FMToperatingarea&) = default;
			FMToperatingarea& operator = (const FMToperatingarea& )=default;
			bool operator == (const FMToperatingarea& rhs) const;
			bool operator != (const FMToperatingarea& rhs) const;
			~FMToperatingarea()=default;
			double getneihgborsperimeter() const;
			FMTmask getmask() const;
		};

	class FMToperatingareacomparator
		{
			FMTmask mask;
		public:
			FMToperatingareacomparator(const FMTmask& lmask);
			bool operator()(const FMToperatingarea& oparea) const;

		};

	}

#endif