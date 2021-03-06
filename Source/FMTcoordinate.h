/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMTCOORD_H_INCLUDED
#define FMTCOORD_H_INCLUDED

#include <memory>
#include <boost/unordered_map.hpp>
#include <vector>
#include <array>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/array.hpp>
#include <string>
#include <set>

namespace Spatial
    {
    class FMTcoordinate
        {
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
			{
			ar & BOOST_SERIALIZATION_NVP(x);
			ar & BOOST_SERIALIZATION_NVP(y);
			}
        unsigned int x,y;
    public:
        FMTcoordinate();
        ~FMTcoordinate()=default;
        FMTcoordinate(unsigned int lx, unsigned int ly);
        FMTcoordinate(const FMTcoordinate& rhs);
        FMTcoordinate at(unsigned int id) const;
        double distance(const FMTcoordinate& coord) const;
        bool within(unsigned int ldistance,const FMTcoordinate& coord) const;
        void upenveloppe(std::vector<FMTcoordinate>& enveloppe) const;
        FMTcoordinate& operator = (const FMTcoordinate& rhs);
        unsigned int getx() const;
        unsigned int gety() const;
        bool operator == (const FMTcoordinate& rhs) const;
        bool operator < (const FMTcoordinate& rhs) const;
        operator std::string() const;
        std::set<FMTcoordinate> getneighbors(const unsigned int& nsize,const bool& circle=false) const;
        };
    }

namespace boost {

  template <>
  struct hash<Spatial::FMTcoordinate>
  {
    std::size_t operator()(const Spatial::FMTcoordinate& coord) const
    {
    unsigned int x = coord.getx();
    unsigned int y = coord.gety();
    return (boost::hash<unsigned int>()(x) && hash<unsigned int>()(y));
    }
  };

}

#endif // FMTCOORD_H_INCLUDED
