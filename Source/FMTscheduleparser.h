/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMTscheduleparser_H_INCLUDED
#define FMTscheduleparser_H_INCLUDED

#include "FMTparser.h"
#include "FMTtheme.h"
#include "FMTaction.h"
#include "FMTschedule.h"
#include "FMTutility.h"
#include <vector>
#include <string>

namespace Parser
{
// DocString: FMTscheduleparser
/**
FMTscheduleparser implement the reading and writing of a vector of FMTschedules.
This parser is also used by the FMTmodelparser.
*/
class FMTscheduleparser: public FMTparser
    {
		// DocString: FMTscheduleparser::getvariable
		/**
		This function uses the comment present in the schedule section representing the
		variable index of the scheduled area to harvest and return it has a int value.
		*/
		int getvariable() const;
    public:
		// DocString: FMTscheduleparser()
		/**
		Default constructor for FMTscheduleparser
		*/
        FMTscheduleparser();
		// DocString: ~FMTscheduleparser()
		/**
		Default destructor for FMTscheduleparser
		*/
		~FMTscheduleparser() = default;
		// DocString: FMTscheduleparser(const FMTscheduleparser&)
		/**
		Default copy constructor for FMTscheduleparser
		*/
        FMTscheduleparser(const FMTscheduleparser& rhs);
		// DocString: FMTscheduleparser::operator=
		/**
		Default copy assignment operator for FMTscheduleparser
		*/
        FMTscheduleparser& operator = (const FMTscheduleparser& rhs);
		// DocString: FMTscheduleparser::read
		/**
		This function reads a schedule section at a given (location) and turns it into a vector of FMTschedule,
		using a complete vector of FMTtheme (themes), a vector of FMTaction (action), and a optional (tolerance).
		If area harvested for a given development is <= to tolerance parameter then it wont be selected. 
		*/
        std::vector<Core::FMTschedule>read(const std::vector<Core::FMTtheme>& themes,
			const  std::vector<Core::FMTaction>& actions,const std::string& location,double tolerance = FMT_DBL_TOLERANCE);
		// DocString: FMTscheduleparser::write
		/**
		This function write a vector of FMTschedules (schedules) into a (location) schedule file.
		*/
        void write(const  std::vector<Core::FMTschedule>& schedules, const std::string& location) const;
    };

}


#endif // FMTscheduleparser_H_INCLUDED
