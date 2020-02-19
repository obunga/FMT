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

#ifndef FMTconstraint_H_INCLUDED
#define FMTconstraint_H_INCLUDED

#include "FMToutput.h"
#include <vector>
#include <string>
#include <limits>
#include "FMTbounds.h"
#include "FMToperator.h"
#include "FMTbounds.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

namespace Core
{

enum  FMTconstrainttype
{
	FMTMAXobjective = 1,
	FMTMINobjective = 2,
	FMTMAXMINobjective = 3,
	FMTMINMAXobjective = 4,
	FMTevenflow = 5,
	FMTnondeclining = 6,
	FMTsequence = 7,
	FMTstandard = 8,
	FMTspatialsize = 9,
	FMTspatialadjacency = 10,
	FMTspatialgreenup = 11
};




class FMTconstraint: public FMToutput,public FMTspec
	{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("output", boost::serialization::base_object<FMToutput>(*this));
		ar & boost::serialization::make_nvp("specification", boost::serialization::base_object<FMTspec>(*this));
		ar & BOOST_SERIALIZATION_NVP(type);
	}
	FMTconstrainttype type;
	public:
		FMTconstraint();
		FMTconstraint(FMTconstrainttype ltype, const FMToutput& loutput);
		FMTconstraint(const FMTconstraint& rhs);
		FMTconstraint& operator = (const FMTconstraint& rhs);
		bool operator == (const FMTconstraint& rhs) const;
		bool operator != (const FMTconstraint& rhs) const;
		bool extravariables() const;
		bool isgoal() const;
		std::vector<std::string>getpenalties(double & sense) const;
		std::vector<std::string>getvariablelevels() const;
		void getgoal(std::string& name,double& value) const;
		bool isobjective() const;
		double sense() const;
		void getbounds(double& lower, double& upper,int period = 0) const;
		void setrhs(const double& lower, const double& upper);
		void setlength(int firstperiod = 1,int lastperiod = std::numeric_limits<int>::max());
		void getvariations(double& lower, double& upper) const;
		bool ismultiple() const;
		bool acrossperiod() const;
		size_t hash() const;
		void setoutput(const FMToutput& out);
		void setconstrainttype(FMTconstrainttype ltype);
		FMTconstrainttype getconstrainttype() const;
		operator std::string() const;
		void standardstring(std::string& line, std::string& period_bounds, std::string& goal) const;
		size_t outputempty() const;
		FMTconstraint presolve(const FMTmask& basemask,
			const std::vector<FMTtheme>& originalthemes,
			const FMTmask& presolvedmask,
			const std::vector<FMTtheme>& newthemes,
			const std::vector<FMTaction>& actions, const FMTyields& yields) const;
		~FMTconstraint()=default;
	};
}


#endif
