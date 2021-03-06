/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMTtransition.h"

namespace Core{
FMTtransition::FMTtransition():name(){}

FMTtransition::FMTtransition(const std::string& lname) : FMTlist<FMTfork>(), name(lname)
	{

	}

bool FMTtransition::isleaking() const
	{
	for (const auto& forkobj : *this)
		{
		if (forkobj.second.sumprop() != 100)
			{
			return true;
			}
		}
	return false;
	}

FMTtransition::FMTtransition(const FMTtransition& rhs) : FMTlist<FMTfork>(rhs),name(rhs.name)
    {

    }
FMTtransition& FMTtransition::operator = (const FMTtransition& rhs)
    {
    if (this!=&rhs)
        {
        name = rhs.name;
        FMTlist<FMTfork>::operator = (rhs);
        }
    return *this;
    }

FMTtransition FMTtransition::single() const
    {
    FMTtransition newtra(*this);
	for (auto& forkobj : newtra)
		{
		forkobj.second = forkobj.second.single();
		}
    return newtra;
    }

bool FMTtransition::operator < (const FMTtransition& rhs) const
    {
    return name < rhs.name;
    }
FMTtransition::operator std::string() const
    {
	std::string line = "*CASE "+name+"\n";
    for (const auto& forkobj:*this)
        {
        line+="*SOURCE "+ std::string(forkobj.first);
        line+= std::string(forkobj.second);
        }
    return line;
    }
bool FMTtransition::operator == (const FMTtransition& rhs) const
    {
    return(name == rhs.name &&
		FMTlist<FMTfork>::operator==(rhs));
    }
bool FMTtransition::operator != (const FMTtransition& rhs) const
    {
    return !(*this==rhs);
    }

unsigned int FMTtransition::age_after(const std::vector<FMTdevelopment>& devs,
                               const FMTaction& action,
                               const FMTyields& ylds,
                               const std::vector<FMTtheme>& themes) const
    {
    unsigned int total_age = 0;
    unsigned int age_count = 0;
	try {
		for (const FMTdevelopment& dev : devs)
		{
			if (dev.operable(action, ylds))
			{
				const std::vector<FMTdevelopmentpath>newpaths = dev.operate(action, *this, ylds, themes);
				for (const FMTdevelopmentpath& path : newpaths)
				{
					total_age += path.development->age;
					++age_count;
				}
			}
		}
		if (age_count == 0)
		{
			return 0;
		}
	}catch (...)
		{
		_exhandler->raisefromcatch("for transition "+this->getname(),"FMTtransition::age_after", __LINE__, __FILE__, _section);
		}
    return (total_age/age_count);
    }

std::vector<FMTtheme>FMTtransition::getstaticthemes(const std::vector<FMTtheme>& themes) const
	{
	std::vector<FMTtheme>staticthemes = themes;
	std::vector<std::pair<FMTmask,FMTfork>>::const_iterator fork_it = this->begin();
	while (fork_it!= this->end() && !staticthemes.empty())
		{
		for (const FMTtransitionmask& trnmask : fork_it->second.getmasktrans())
			{
			const FMTmask msk = trnmask.getmask();
			staticthemes = msk.getstaticthemes(staticthemes);
			}
		++fork_it;
		}
	return staticthemes;
	}

const FMTfork* FMTtransition::getfork(const FMTdevelopment& dev,
                               const FMTyields& ylds) const
    {
	const std::vector<const FMTfork*> forks = this->findsets(dev.mask);
    if (!forks.empty())
        {
        for(const FMTfork* fork : forks)
            {
			const std::map<std::string,double>yields = ylds.getylds(dev,*fork);
            if (fork->allow(dev.period,dev.age,dev.lock,yields))
                {
                return fork;
                }
            }
        }
    return nullptr;
    }

 FMTmask FMTtransition::main_target(const std::vector<FMTdevelopment>& devs,
                            const FMTyields& ylds) const
    {
	 std::map<FMTmask,unsigned int>hits;
    for(const FMTdevelopment& dev : devs)
        {
        const FMTfork* fork = this->getfork(dev,ylds);
        if (fork)
            {
            for (const FMTtransitionmask& target :  fork->getmasktrans())
                {
                const FMTmask msktarget = target.getmask();
                if (hits.find(msktarget)==hits.end())
                    {
                    hits[msktarget] = 1;
                    }else{
                    ++hits[msktarget];
                    }
                }
            }
        }
    FMTmask bestmask;
    unsigned int besthit = 0;
    for(std::map<FMTmask,unsigned int>::const_iterator it = hits.begin();it!=hits.end();it++)
        {
        if(it->second > besthit)
            {
            besthit = it->second;
            bestmask = it->first;
            }
        }
    return bestmask;
    }
 std::map<std::string, std::vector<FMTdevelopment>> FMTtransition::attribute_targets(const std::vector<FMTdevelopment>& devs,
	 const FMTyields& ylds, const std::vector<FMTtheme>& themes) const
	{
	 std::map<std::string, std::vector<FMTdevelopment>>results;
	 for (const FMTdevelopment& dev : devs)
		{
		 const FMTfork* fork = this->getfork(dev, ylds);
		 if (fork)
			{
			 std::string key = this->name;
			 for (const FMTtheme& theme : themes)
				{
				key+= ("-"+dev.mask.get(theme));
				}
			if (results.find(key)==results.end())
				{
				results[key] = std::vector<FMTdevelopment>();
				}
			results[key].push_back(dev);
			}
		}
	 return results;
	 }

 FMTtransitioncomparator::FMTtransitioncomparator(std::string name) :transition_name(name) {}
 bool FMTtransitioncomparator::operator()(const FMTtransition& transition) const
	{
	return (transition.getname()  == transition_name);
	}

 FMTtransition FMTtransition::presolve(const FMTmask& basemask,
	 const std::vector<FMTtheme>& originalthemes,
	 const FMTmask& presolvedmask,
	 const std::vector<FMTtheme>& newthemes) const
	{
	FMTtransition newtransition(*this);
	try {
	newtransition.presolvelist(basemask, originalthemes, presolvedmask, newthemes);
	newtransition.update();
	if (!presolvedmask.empty())
		{
		for (auto& transitionobject : newtransition)
			{
			transitionobject.second = transitionobject.second.presolve(presolvedmask, newthemes);
			}
		}
	}catch (...)
		{
		_exhandler->raisefromcatch("for transition "+this->getname(),"FMTtransition::presolve", __LINE__, __FILE__, _section);
		}	
	return newtransition;
	}

}

BOOST_CLASS_EXPORT_IMPLEMENT(Core::FMTtransition)

