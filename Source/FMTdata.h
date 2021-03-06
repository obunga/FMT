/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMTDATA_H_INCLUDED
#define FMTDATA_H_INCLUDED

#include <vector>
#include <string>
#include <map>
#include "FMToperator.h"
#include "FMTmask.h"
#include "FMTexpression.h"
#include "FMTutility.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include "FMTutility.h"

namespace Core
{

class FMTdata
    {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(ops);
		ar & BOOST_SERIALIZATION_NVP(source);
		ar & BOOST_SERIALIZATION_NVP(stacking);
		ar & BOOST_SERIALIZATION_NVP(data);
	}
    FMTyieldparserop ops;
	std::vector<std::string>source;
	std::vector<bool>stacking;//Stacking a string = true stacking a number = false
	mutable std::map<size_t,double> _cache; //hash base on (age and/or period and/or mask) //only for complex yield!!!!
	mutable bool _agebase;
	size_t hashdata(const int& period, const int& age, const FMTmask& mask) const;
    public:
		std::vector<double> data;
        FMTdata();
		~FMTdata() = default;
        FMTdata(const FMTdata& rhs);
		FMTdata(const std::vector<double>& lvalues,
			const FMTyieldparserop& lops,
			const std::vector<std::string>& lsource);
        FMTdata(const std::vector<double>& lvalues,
                const FMTyieldparserop& lops,
                const std::vector<std::string>& lsource,
				const std::vector<bool>& varstack);
        FMTdata& operator = (const FMTdata& rhs);
        FMTyieldparserop getop() const;
		bool constant() const;
		bool nulldata() const;
		bool cachevalue(const FMTmask& mask, const int& age, const int& period) const;
		double get(const FMTmask& mask, const int& age, const int& period) const;
		void set(const double& value, const FMTmask& mask, const int& age, const int& period, const bool& age_only) const;
		std::vector<std::string> getsource() const;
		std::vector<double>tovalues(const std::map<std::string, double>& sources) const;
		FMTexpression toexpression() const;
		bool operator == (const FMTdata& rhs) const;
        operator std::string() const;
		FMTdata operator * (const double& factor) const;
    };
}

BOOST_CLASS_EXPORT_KEY(Core::FMTdata)


#endif // FMTDATA_H_INCLUDED
