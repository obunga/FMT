/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMTtheme_H_INCLUDED
#define FMTtheme_H_INCLUDED

#include <boost/serialization/serialization.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/unordered_map.hpp>
#include <string>
#include <vector>
#include <map>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/string.hpp>
#include "FMTexception.h"
#include "FMTmask.h"
#include "FMTobject.h"



namespace Core
{
// DocString: FMTtheme
/**
FMTtheme hold multiple attributes for only one theme. FMTtheme gives the description of a part of a FMTmask
from the start bit to the start + theme.size() bit. It also hold the information about attribute aggregates.
FMTtheme is realy close to FMTmask class.
*/
class FMTtheme : public FMTobject
    {
    friend class FMTmask;
    protected:
		// DocString: FMTtheme::id
		/// Id is the id of the theme first theme in the landscape section is number 1
		int id;
		// DocString: FMTtheme::start
		/// Start is the bit location in the FMTmask at which the attributes of the theme starts.
        int start;
    private:
		// DocString: FMTtheme::serialize
		/**
		Serialize function is for serialization, used to do multiprocessing across multiple cpus (pickle in Pyhton)
		*/
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(id);
			ar & BOOST_SERIALIZATION_NVP(start);
			ar & BOOST_SERIALIZATION_NVP(aggregates);
			ar & BOOST_SERIALIZATION_NVP(source_aggregates);
			ar & BOOST_SERIALIZATION_NVP(valuenames);
			ar & BOOST_SERIALIZATION_NVP(indexes);
			ar & BOOST_SERIALIZATION_NVP(name);
		}
		// DocString: FMTtheme::aggregates
		///This map hold the aggregates (key) of the theme and all attributes of the theme (vector item)
        std::map<std::string, std::vector<std::string>>aggregates;
		// DocString: FMTtheme::source_aggregates
		///This map hold the aggregates (key) of the theme and all attributes or aggregates of the theme (vector item)
		std::map<std::string, std::vector<std::string>>source_aggregates;
		// DocString: FMTtheme::valuenames
		///This is the main map that keep track of the attributes for the theme and the relative name of the attribute
		std::map<std::string, std::string>valuenames;
		// DocString: FMTtheme::indexes
		///This data member is only used for the usage of the key word _index in the yield seciton
		std::map<std::string, std::map<std::string,double>>indexes;
		// DocString: FMTtheme::desagregate
		/**
		Using (aggregates) the function turns the map with aggregates as key and aggregates as value to a map with
		aggregates as key and attributes as values.
		*/
		std::map<std::string,std::vector<std::string>>desagregate(std::map<std::string, std::vector<std::string>>aggregates,const std::vector<std::string>&bases);
		// DocString: FMTtheme::name
		///The name of the FMTtheme if their's a name provided by the user.
		std::string name;
		// DocString: FMTtheme::strtobits
		/**
		Convert an attribute|aggregate|? (value) to a bitset for the entire theme size.
		*/
		boost::dynamic_bitset<> strtobits(const std::string& value) const;
		// DocString: FMTtheme::bitstostr
		/**
		Convert a bitset to an attribute|aggregate|? (value) for the entire theme size.
		*/
		std::string bitstostr(const boost::dynamic_bitset<>& bits) const;
	public:
		// DocString: FMTtheme()
		/**
		Default constructor for FMTtheme.
		*/
        FMTtheme();
		// DocString: ~FMTtheme()
		/**
		Default destructor for FMTtheme.
		*/
		~FMTtheme() = default;
		// DocString: FMTtheme(const std::map<std::string,std::vector<std::string>>&,const  std::map<std::string,std::string>&,const int&,const int&,const std::string&)
		/**
		FMTtheme main constructor.
		*/
        FMTtheme(const std::map<std::string, std::vector<std::string>>&laggregates,
			    const  std::map<std::string,std::string>&lvaluenames, 
				const int& lid,const int& lstart, const std::string& lname);
		// DocString: FMTtheme(const std::map<std::string,std::vector<std::string>>&,const std::map<std::string,std::string>&,const std::map<std::string,std::map<std::string,double>>&,const int&, const int&,const std::string&)
		/**
		FMTtheme constructor to use for FMTtheme indexing.
		*/
        FMTtheme(const std::map<std::string, std::vector<std::string>>&laggregates,
				 const std::map<std::string, std::string>&lvaluenames,
                 const std::map<std::string, std::map<std::string,double>>& indexing,
				const int& lid,const int& lstart,const std::string& lname);
		// DocString: FMTtheme(const std::vector<std::string>&, const int&,const int&,const std::string&)
		/**
		A more simple constructor for FMTtheme without aggregates and indexing.
		*/
		FMTtheme(const std::vector<std::string>& lvaluenames, const int& lid, const int& lstart, const std::string& lname);
		// DocString: FMTtheme(const FMTtheme&)
		/**
		Copy constructor for FMTtheme.
		*/
		FMTtheme(const FMTtheme& rhs);
		// DocString: FMTtheme::operator=
		/**
		Copy assignment for FMTtheme.
		*/
		FMTtheme& operator = (const FMTtheme& rhs);
		// DocString: FMTtheme::isattribute
		/**
		Return true if the (value) is an attribute of the FMTtheme.
		*/
		inline bool isattribute(const std::string& value) const
			{
			return (valuenames.find(value) != valuenames.end());
			}
		// DocString: FMTtheme::isaggregate
		/**
		Return true if the (value) is an aggregate of the FMTtheme.
		*/
		inline bool isaggregate(const std::string& value) const
			{
			return (aggregates.find(value) != aggregates.end());
			}
		// DocString: FMTtheme::isindex
		/**
		Return true if the (value) is an index of the FMTtheme (only use in yields section).
		*/
        bool isindex(const std::string& value) const;
		// DocString: FMTtheme::useindex
		/**
		Return true if the FMTtheme uses index.
		*/
        bool useindex() const;
		// DocString: FMTtheme::getindex
		/**
		Get the index for the given (attribute) of the index (value).
		*/
        double getindex(const std::string& attribute,const std::string& value) const;
		// DocString: FMTtheme::inaggregate
		/**
		Check if the (value) is part of the (aggregate).
		*/
        bool inaggregate(const std::string& value,const std::string& aggregate);
		// DocString: FMTtheme::isvalid
		/**
		Check if the (value) is a valid attribute | aggregate | ? for the FMTtheme.
		*/
		bool isvalid(const std::string& value) const;
		// DocString: FMTtheme::operator==
		/**
		Comparison operator of FMTtheme.
		*/
		bool operator == (const FMTtheme& rhs) const;
		// DocString: FMTtheme::size
		/**
		Return the size of FMTtheme can be 0 if the theme has no attribute.
		*/
		inline size_t size() const
			{
			return valuenames.size();
			}
		// DocString: FMTtheme::getstart
		/**
		Return the bit location of the mask at which the theme starts.
		*/
		size_t getstart() const;

		// DocString: FMTtheme::getid
		/**
		Getter for the FMTtheme id.
		*/
		inline int getid() const
			{
			return id;
			}
		// DocString: FMTtheme::getname
		/**
		Getter for the FMTtheme name.
		*/
		inline std::string getname() const
			{
			return name;
			}
		// DocString: FMTtheme::empty
		/**
		Check if the FMTtheme is empty (no attribute).
		*/
		inline bool empty() const
			{
			return valuenames.empty();
			}
		// DocString: FMTtheme::getvaluenames
		/**
		Getter the mapping of the attributes of the FMTtheme (attribute for key and attribute name for value).
		*/
		std::map<std::string, std::string>getvaluenames() const;
		// DocString: FMTtheme::getattributes
		/**
		Get the attributes of a aggregate (value) for the FMTtheme if aggregate_source == true then the
		aggregate source map is used.
		*/
		std::vector<std::string>getattributes(const std::string& value, bool aggregate_source = false) const;
		// DocString: FMTtheme::presolve
		/**
		The function presolve the FMTtheme so it removes non used attributes base on the basemask and fill up the 
		selected mask we the selected attribute is the presolved FMTtheme is not empty and size > 1 then
		it gets an newid and a newstart and increment both.
		*/
		FMTtheme presolve(const FMTmask& basemask,int& newid,int& newstart, FMTmask& selected) const;
		// DocString: FMTtheme::operator std::string
		/**
		Return a string representation of the FMTtheme seen in a landscape file 
		*/
        operator std::string() const;
    };

	// DocString: FMTthemecomparator
	/**
	FMTthemecomparator to check two themes are the same.
	*/
	class FMTthemecomparator
	{
		// DocString: FMTthemecomparator::base_theme
		///The theme that we are looking for
		FMTtheme base_theme;
	public:
		// DocString: FMTthemecomparator(const FMTtheme&)
		/**
		FMTthemecomparator constructor ltheme_mask is the theme of that we want to match.
		*/
		FMTthemecomparator(const FMTtheme& lbase_theme);
		// DocString: FMTthemecomparatorr::operator()(const FMTtheme&)
		/**
		Matching test operator for FMTthemecomparator.
		*/
		bool operator()(const FMTtheme& theme) const;

	};



}

BOOST_CLASS_EXPORT_KEY(Core::FMTtheme)

#endif // FMTtheme_H_INCLUDED
