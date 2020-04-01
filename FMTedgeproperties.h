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

#ifndef FMTedgeproperties_H_INCLUDED
#define FMTedgeproperties_H_INCLUDED

#include "FMTaction.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <vector>
#include <string>

namespace Graph
	{
	class FMTedgeproperties
		{
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
			{
			ar & BOOST_SERIALIZATION_NVP(action);
			ar & BOOST_SERIALIZATION_NVP(variableID);
			ar & BOOST_SERIALIZATION_NVP(proportion);
			}
		int action;
		int variableID;
		double proportion;
		public:
			~FMTedgeproperties()=default;
			FMTedgeproperties();
			FMTedgeproperties(const FMTedgeproperties& rhs);
			FMTedgeproperties& operator = (const FMTedgeproperties& rhs);
			bool isaction(const std::vector<Core::FMTaction>& actions,
				const Core::FMTaction& rhsaction) const;
			inline int	getvariableID() const
				{
				return variableID;
				}
			void setvariableID(const int& newvariableID);
			void setactionID(const int& newid);
			inline int getactionID() const
				{
				return action;
				}
			inline  double getproportion() const
				{
				return proportion;
				}
			FMTedgeproperties(int laction,
				int lvariableID,
				double lproportion);
			bool operator == (const FMTedgeproperties& rhs) const;
			bool operator != (const FMTedgeproperties& rhs) const;
			bool operator < (const FMTedgeproperties& rhs) const;
			inline std::string variablename() const;
			inline const int* getvariableptr() const
				{
				return &variableID;
				}
			inline const int* getactionptr() const
				{
				return &action;
				}
			
		};
	}

#endif