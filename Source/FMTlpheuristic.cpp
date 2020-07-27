/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifdef FMTWITHOSI

#include "FMTlpheuristic.h"
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
	
	FMTlpheuristic::FMTlpheuristic(const FMTlpheuristic& rhs) :
		Core::FMTobject(rhs),Models::FMTlpsolver(rhs),
		generator(rhs.generator),seed(rhs.seed),usingsolvercopy(true)
		{
		
		}
	FMTlpheuristic& FMTlpheuristic::operator = (const FMTlpheuristic& rhs)
		{
		if (this!=&rhs)
			{
			FMTobject::operator = (rhs);
			FMTlpsolver::operator = (rhs);
			generator=rhs.generator;
			seed = rhs.seed;
			usingsolvercopy = true;
			}
		return *this;
		}
	
	FMTlpheuristic::FMTlpheuristic(Models::FMTlpsolver& basesolve, size_t lseed,bool copysolver):
		Core::FMTobject(), Models::FMTlpsolver(),generator(lseed),seed(lseed), usingsolvercopy(copysolver)
		{
		try {
			if (copysolver)
				{
				FMTlpsolver::operator=(basesolve);
			}else {
				FMTlpsolver::passinsolver(basesolve);
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMTlpheuristic::FMTlpheuristic", __LINE__, __FILE__);
			}
		}


	void FMTlpheuristic::setgeneratorseed(const size_t& lseed)
		{
		seed = lseed;
		generator.seed(lseed);
		}

}
BOOST_CLASS_EXPORT_IMPLEMENT(Heuristics::FMTlpheuristic)
#endif