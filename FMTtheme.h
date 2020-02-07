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

#ifndef FMTtheme_H_INCLUDED
#define FMTtheme_H_INCLUDED

#include <boost/serialization/serialization.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/unordered_map.hpp>
#include <string>
#include <vector>
#include <map>
#include <boost/serialization/nvp.hpp>
#include "FMTexception.h"
#include "FMTmask.h"


namespace Core
{
class FMTtheme
    {

    friend class FMTmask;
    protected:
        int id,start;
    private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(id);
			ar & BOOST_SERIALIZATION_NVP(start);
			ar & BOOST_SERIALIZATION_NVP(aggregates);
			ar & BOOST_SERIALIZATION_NVP(source_aggregates);
			ar & BOOST_SERIALIZATION_NVP(valuenames);
			ar & BOOST_SERIALIZATION_NVP(name);
		}
        std::map<std::string, std::vector<std::string>>aggregates;
		std::map<std::string, std::vector<std::string>>source_aggregates;
		std::map<std::string, std::string>valuenames;
		std::map<std::string, std::map<std::string,double>>indexes;
		std::map<std::string,std::vector<std::string>>desagregate(std::map<std::string, std::vector<std::string>>aggregates,const std::vector<std::string>&bases);
    public:
		std::string name;
        FMTtheme();
        FMTtheme(const std::map<std::string, std::vector<std::string>>&laggregates,
			    const  std::map<std::string,std::string>&lvaluenames, 
				const int& lid,const int& lstart, const std::string& lname);
        FMTtheme(const std::map<std::string, std::vector<std::string>>&laggregates,
				 const std::map<std::string, std::string>&lvaluenames,
                 const std::map<std::string, std::map<std::string,double>>& indexing,
				const int& lid,const int& lstart,const std::string& lname);
		FMTtheme(const FMTtheme& rhs);
		FMTtheme& operator = (const FMTtheme& rhs);
        FMTtheme(const std::vector<std::string>& lvaluenames,const int& lid,const int& lstart,const std::string& lname);
        bool usefull() const;
        bool isattribute(const std::string& value) const;
        bool isindex(const std::string& value) const;
        bool useindex() const;
        double getindex(const std::string& attribute,const std::string& value) const;
        bool isaggregate(const std::string& value) const;
        bool inaggregate(const std::string& value,const std::string& aggregate);
		bool isvalid(const std::string& value) const;
		bool operator == (const FMTtheme& rhs) const;
        size_t size() const;
		int getid() const;
		bool empty() const;
		std::map<std::string, std::string>getvaluenames() const;
        boost::dynamic_bitset<> strtobits(const std::string& value) const;
		std::string bitstostr(const boost::dynamic_bitset<>& bits) const;
		std::vector<std::string>getattributes(const std::string& value, bool aggregate_source = false) const;
        operator std::string() const;
    };
}
#endif // FMTtheme_H_INCLUDED
