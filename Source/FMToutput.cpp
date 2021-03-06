/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMToutput.h"

namespace Core{


FMToutput::FMToutput(const std::string& lname,const std::string& ldescription,const int& ltheme_target,
	std::vector<FMToutputsource>& lsources, std::vector<FMToperator>& loperators):
	FMTobject(),
    sources(lsources),
    operators(loperators),
	theme_target(ltheme_target),
    name(lname),
    description(ldescription)

    {

    }
FMToutput::FMToutput(const std::string& lname) :FMTobject(), sources(),operators(), theme_target(-1),name(lname),description()
    {

    }
FMToutput::FMToutput() : FMTobject(),sources(),operators(), theme_target(-1),name(),description()
    {

    }

FMToutput::FMToutput(const FMToutput& rhs) :
	FMTobject(rhs),
    sources(rhs.sources),
    operators(rhs.operators),
	theme_target(rhs.theme_target),
    name(rhs.name),
    description(rhs.description)
    {

    }
FMToutput& FMToutput::operator = (const FMToutput& rhs)
    {
    if (this!=&rhs)
        {
		FMTobject::operator=(rhs);
        name = rhs.name;
        sources = rhs.sources;
		theme_target =rhs.theme_target;
        operators = rhs.operators;
        description = rhs.description;
        }
    return *this;
    }
FMToutput& FMToutput::operator +=(const FMToutput& rhs)
	{
	if (!this->name.empty())
		{
		this->name = this->name + "+" + rhs.name;
		this->description = this->description + "+" + rhs.description;
	}else {
		this->name = rhs.name;
		this->description = rhs.description;
		}
    if (!this->sources.empty())
		{
		this->operators.push_back(FMToperator("+"));
		}
	this->sources.insert(this->sources.end(), rhs.sources.begin(), rhs.sources.end());

	this->operators.insert(this->operators.end(),rhs.operators.begin(),rhs.operators.end());
	if (this->theme_target != rhs.theme_target)
		{
		this->theme_target = -1;
		}
	return *this;
	}
FMToutput& FMToutput::operator -=(const FMToutput& rhs)
	{
	if (!this->name.empty())
		{
		this->name = this->name + "-" + rhs.name;
		this->description = this->description + "-" + rhs.description;
	}else {
		this->name = rhs.name;
		this->description = rhs.description;
		}
    if (!this->sources.empty())
		{
		this->operators.push_back(FMToperator("-"));
		}
	this->sources.insert(this->sources.end(), rhs.sources.begin(), rhs.sources.end());

	if (this->theme_target != rhs.theme_target)
		{
		this->theme_target = -1;
		}
	for (const FMToperator& rhsop : rhs.operators)
		{
		operators.push_back(rhsop.reverse());
		}
	return *this;
	}

FMToutput& FMToutput::operator  *= (const double& rhs)
	{
	if (!this->name.empty())
		{
		this->name =  this->name+"*"+ std::to_string(rhs);
		this->description = this->description +"*" + std::to_string(rhs);
		}
	if (!sources.empty())
		{
		std::vector<FMToutputsource>new_sources;
		std::vector<FMToperator>new_operators;
		size_t location = 0;
		for (FMToutputsource& source : sources)
			{
			new_sources.push_back(source);
			if (source.isvariable() || source.isvariablelevel())
				{
                new_operators.push_back(FMToperator("*"));
				new_sources.push_back(FMToutputsource(FMTotar::val, rhs));
				}
			if (location < operators.size())
				{
				new_operators.push_back(operators.at(location));
				}

			++location;
			}
		sources = new_sources;
		operators = new_operators;
		}
	return *this;
	}

FMToutput& FMToutput::operator /=(const double& rhs)
	{
	if (!this->name.empty())
		{
		this->name = this->name + "/" + std::to_string(rhs);
		this->description = this->description + "/" + std::to_string(rhs);
		}
	if (!sources.empty())
	{
		std::vector<FMToutputsource>new_sources;
		std::vector<FMToperator>new_operators;
		size_t location = 0;
		for (FMToutputsource& source : sources)
		{
			new_sources.push_back(source);
			if (location < operators.size())
			{
				new_operators.push_back(operators.at(location));
			}
			if (source.isvariable() || source.isvariablelevel())
			{
				new_sources.push_back(FMToutputsource(FMTotar::val, rhs));
				new_operators.push_back(FMToperator("/"));
			}
			++location;
		}
		sources = new_sources;
		operators = new_operators;
	}
	return *this;
	}


FMToutput::operator std::string() const
    {
	std::string line = "*OUTPUT ";
	if (islevel())
		{
		line = "*LEVEL ";
		}
	line += name;
	if (theme_target!=-1)
		{
		line += " (_TH" + std::to_string(theme_target + 1) + ")";
		}
	line+=" " + description + "\n";
	if (!sources.empty() && ((islevel() && sources.at(0).getaction().empty()) || (!islevel())))
		{
		line += "*SOURCE ";
		for (size_t id = 0; id < sources.size(); ++id)
			{
			line += std::string(sources[id]) + " ";
			if (id < operators.size())
			{
				line += std::string(operators[id]) + " ";
			}
			}
		}
    line+="\n";
    return line;
    }
bool FMToutput::empty() const
	{
	return sources.empty();
	}
bool FMToutput::islevel() const
	{
	for (const FMToutputsource& src : sources)
		{
		if (src.gettarget() == FMTotar::level)
			{
			return true;
			}
		}
    return false;
	}

bool FMToutput::isconstant() const
	{
	for (const FMToutputsource& src : sources)
		{
		if (src.gettarget() != FMTotar::val)
			{
			return false;
			}
		}
	return true;
	}

double FMToutput::getconstantvalue() const
	{
	double value = 0;
	std::vector<double>values;
	if (isconstant())
		{
		for (const FMToutputsource& src : sources)
			{
			values.push_back(src.getvalue());
			}
		value = shuntingyard(values, this->operators);
		}
	return value;
	}


bool FMToutput::containslevel() const
	{
	for (const FMToutputsource& src : sources)
		{
		if (src.islevel())
			{
			return true;
			}
		}
	return false;
	}

bool FMToutput::linear() const
	{
	if (sources.size() > 1 && (find(operators.begin(), operators.end(), FMToperator("*")) != operators.end() ||
		find(operators.begin(), operators.end(), FMToperator("/")) != operators.end()))
		{
		int location = 0;
		int lastnonlinear = -1;
		for (const FMToutputsource& source : sources)
			{
			if (source.isvariable() && lastnonlinear!=-1 && location == (lastnonlinear+1))
				{
				return false;
				}
			if (source.isvariable() && location < static_cast<int>(operators.size()) &&
				(operators[location] == FMToperator("*") || operators[location] == FMToperator("/")))
				{
				lastnonlinear = location;
				}
			++location;
			}

		}
	return true;
	}
double FMToutput::shuntingyard(const std::vector<double>& sourcevalues,const std::vector<FMToperator>& simple_operators) const
	{
	try {
		std::map<std::string, double>mapping;
		size_t id = 0;
		std::vector<std::string>expression_inputs;
		std::string test = "";
		for (const double& value : sourcevalues)
		{
			expression_inputs.push_back(std::to_string(value));
			test += std::to_string(value);
			if (id < simple_operators.size())
			{
				expression_inputs.push_back(simple_operators[id]);
				test += simple_operators[id];
			}
			++id;
		}
		const FMTexpression newexpression(expression_inputs);
		return newexpression.shuntingyard(mapping);
	}catch (...)
		{
		_exhandler->raisefromcatch("for " + std::string(*this),"FMToutput::shuntingyard", __LINE__, __FILE__, _section);
		}
	return 0;
	}

FMToutput FMToutput::boundto(const std::vector<FMTtheme>& themes, const FMTperbounds& bound,const std::string& specialbound, std::string attribute) const
	{
	FMToutput newoutput(*this);
	try {
		if (!attribute.empty())
		{
			newoutput.name = newoutput.name + "(" + attribute + ")";
		}
		if (!bound.empty())
		{
			if (specialbound.empty() && bound.getlower() == bound.getupper())//single bounded
			{
				newoutput.name = newoutput.name + "[" + std::to_string(bound.getlower()) + "]";
			}
			else if (!specialbound.empty())
			{
				std::string name = specialbound;
				name += "(" + newoutput.name;
				if (!(bound.getupper() == std::numeric_limits<double>::max() && bound.getlower() == 1))
				{
					name += ",";
					name += std::to_string(bound.getlower()) + "..";
					name += std::to_string(bound.getupper());
				}
				name += ")";
				newoutput.name = name;
			}
		}
		if (!newoutput.islevel())
		{
			for (FMToutputsource& source : newoutput.sources)
			{
				if (source.isvariable())
				{
					if (!bound.empty())
					{
						source.setbounds(bound);
					}
					if (!attribute.empty())
					{
						FMTmask oldmask = FMTmask(source.getmask());
						oldmask.set(themes.at(theme_target), attribute);
						source.setmask(oldmask);
					}
					if (!specialbound.empty() && specialbound == "_AVG")
					{
						source.setaverage();
					}
				}
			}
		}
	}catch (...)
		{
		_exhandler->raisefromcatch("for "+std::string(*this),"FMToutput::boundto", __LINE__, __FILE__,_section);
		}
	return newoutput;
	}

std::vector<FMToutputnode> FMToutput::getnodes(const std::vector<FMTactualdevelopment>&area,
											   const std::vector<Core::FMTaction>&actions,
											   const FMTyields& yields,
											   double multiplier) const
	{
	//set a expression and get the nodes! check if the node is positive or negative accross the equation!!!
	std::vector<FMToutputnode>nodes;
	try {
		size_t src_id = 0;
		size_t op_id = 0;
		FMToutputsource main_source;
		FMToutputsource main_factor(FMTotar::val,1);
		double constant = 1;
		nodes.reserve(sources.size());
		for (const FMToutputsource& source : sources)
		{
			if (((source.isvariable() && !source.canbededucedtoconstant()) || source.islevel()))
			{
				if (!main_source.getmask().empty() || main_source.isvariablelevel())
				{
					if (!((main_factor.isconstant() && main_factor.getvalue() == 0) || constant == 0))
					{
						if (main_source.isaverage())
							{
							constant *= multiplier;
							}
						nodes.emplace_back(main_source, main_factor, constant);
					}
				}
				main_factor = FMToutputsource(FMTotar::val, 1);
				main_source = source;
				constant = 1;
			}
			if (src_id != 0 && (op_id < operators.size()) && !operators.at(op_id).isfactor())
			{
				constant *= operators.at(op_id).call(0, 1);
			}
			if (source.istimeyield())
			{
				main_factor = source;
			}
			else if ((op_id < operators.size()) && operators.at(op_id).isfactor())
			{
				if (source.isconstant())
				{
					const double value = source.getvalue();
					constant = operators.at(op_id).call(constant, value);
				}
				else if (source.canbededucedtoconstant())
				{
					const double value = source.getconstantvalue(area, actions, yields);
					constant = operators.at(op_id).call(constant, value);
				}
			}
			if (src_id > 0)
			{
				++op_id;
			}
			++src_id;
		}
		if (main_source.isvariablelevel() || main_source.isvariable())
		{
			if (!((main_factor.isconstant() && main_factor.getvalue() == 0) || constant == 0))
			{
				if (main_source.isaverage())
				{
					constant *= multiplier;
				}
				nodes.emplace_back(main_source, main_factor, constant);
			}


		}
	}catch (...)
		{
		_exhandler->raisefromcatch(
			"","FMToutput::getnodes", __LINE__, __FILE__,_section);
		}
	return nodes;
	}

bool FMToutput::issingleperiod() const
	{
	for (const FMToutputsource& source : this->sources)
		{
		if (source.emptyperiod() || (source.getperiodlowerbound() != source.getperiodupperbound()))
			{
			return false;
			}
		}
	return true;
	}

bool FMToutput::hasaverage() const
{
return (name.find("_AVG") != std::string::npos);
}

int FMToutput::gettargetperiod() const
	{
	int target = -1;
	if (issingleperiod())
		{
		return sources.at(0).getperiodlowerbound();
		}
	return target;
	}

size_t FMToutput::hash() const
	{
	size_t seed = 0;
	for (const FMToperator& op : operators)
		{
		boost::hash_combine(seed,op.hash());
		}
	for (const FMToutputsource& src : sources)
		{
		boost::hash_combine(seed, src.hash());
		boost::hash_combine(seed, src.getyield());
		}
	return seed;
	}


bool FMToutput::operator == (const FMToutput& rhs) const
    {
    return (name == rhs.name ||
		(!sources.empty() && !rhs.empty() &&
		description == rhs.description &&
		theme_target == rhs.theme_target &&
		sources == rhs.sources &&
		operators == rhs.operators));
    }

bool FMToutput::operator != (const FMToutput& rhs) const
    {
    return !(*this== rhs);
    }

size_t FMToutput::size() const
	{
	return sources.size();
	}

FMToutput FMToutput::presolve(const FMTmask& basemask,
	const std::vector<FMTtheme>& originalthemes,
	const FMTmask& presolvedmask,
	const std::vector<FMTtheme>& newthemes,
	const std::vector<FMTaction>& actions, const FMTyields& yields) const
	{
	FMToutput newoutput(*this);
	try {
		std::vector<FMToutputsource>newsources;
		std::vector<FMToperator>newoperators;
		size_t operatorid = 0;
		int lastnotpushed = -10;
		if (!presolvedmask.empty())
		{
			newoutput.theme_target = -1;
		}
		for (size_t sourceid = 0; sourceid < sources.size(); ++sourceid)
		{
			bool pushedsource = true;
			const std::string yieldname = sources.at(sourceid).getyield();
			if (sources.at(sourceid).isvariable())
			{
				const std::string actionname = sources.at(sourceid).getaction();
				if ((!sources.at(sourceid).getmask().isnotthemessubset(basemask, originalthemes)) &&
					(actionname.empty() ||
						std::find_if(actions.begin(), actions.end(), FMTactioncomparator(actionname, true)) != actions.end()) &&
						(yieldname.empty() || !yields.isnullyld(yieldname)))
				{
					FMToutputsource newsource = sources.at(sourceid);
					if (!presolvedmask.empty())
					{
						newsource = newsource.presolve(presolvedmask, newthemes);
					}
					newsources.push_back(newsource);
				}
				else {
					pushedsource = false;
					lastnotpushed = static_cast<int>(sourceid);
				}
			}
			else if (!sources.at(sourceid).isvariable() && (sources.at(sourceid).islevel() || (sources.at(sourceid).istimeyield() && !yields.isnullyld(yieldname)) ||
				(sources.at(sourceid).isconstant() && lastnotpushed != static_cast<int>(sourceid - 1))))
			{
				pushedsource = true;
				newsources.push_back(sources.at(sourceid));
			}
			else {
				pushedsource = false;
			}

			if (operatorid < operators.size() && pushedsource && sourceid>0)
			{
				newoperators.push_back(operators.at(operatorid));
			}
			if (sourceid > 0)
			{
				++operatorid;
			}
			if (pushedsource)
			{
				lastnotpushed = -10;
			}
		}
		newoutput.sources = newsources;
		newoutput.operators = newoperators;
	}catch (...)
		{
		_exhandler->raisefromcatch("for "+std::string(*this),"FMToutput::presolve", __LINE__, __FILE__, _section);
		}
	return newoutput;
	}

std::vector<std::string> FMToutput::getdecomposition(const std::vector<FMTtheme>& themes) const
	{
	std::vector<std::string>validdecomp;
	if (theme_target!=-1)
		{
		int srcid = 0;
		for (const FMToutputsource& source : sources)
			{
			if (source.isvariable())
				{
				const FMTmask srcmask = source.getmask();
				std::vector<std::string>unique_selection;
				for (const FMTmask& decmask : srcmask.decompose(themes[theme_target]))
					{
					unique_selection.push_back(decmask.get(themes[theme_target]));
					}
				if (srcid==0)
					{
					validdecomp = unique_selection;
				}else {
					std::vector<std::string>newvalid;
					std::set_intersection(validdecomp.begin(), validdecomp.end(),
						unique_selection.begin(), unique_selection.end(), back_inserter(newvalid));
					validdecomp = newvalid;
				}
				++srcid;
				}

			}
		}
	return validdecomp;
	}

FMToutput FMToutput::intersectwithmask(const Core::FMTmask& mask) const
	{
	FMToutput newoutput(*this);
	for (FMToutputsource& source : newoutput.sources)
		{
		if (source.isvariable())
			{
			source.setmask(source.getmask().getintersect(mask));
			}
		}
	return newoutput;
	}

FMTmask FMToutput::getvariableintersect() const
{
	FMTmask mask;
	for (const FMToutputsource& source :sources)
	{
		if (source.isvariable())
		{
			if (mask.empty())
				{
				mask = source.getmask();
			}else {
				mask = mask.getintersect(source.getmask());
			}
			
		}
	}
	return mask;
}


void FMToutput::setperiod(const int& newperiod)
	{
	for (FMToutputsource& source : sources)
		{
		source.setbounds(FMTperbounds(FMTsection::Outputs, newperiod, newperiod));
		}
	}

bool FMToutput::isactionbased() const
	{
	for (const FMToutputsource& source : sources)
		{
		if (!source.getaction().empty())
			{
			return true;
			}
		}
	return false;
	}


bool FMToutput::isinventory() const
	{
	for (const FMToutputsource& source : sources)
	{
		if (source.isinventory())
		{
			return true;
		}
	}
	return false;
	}

FMTtheme FMToutput::targettheme(const std::vector<FMTtheme>& themes) const
	{
	if (theme_target>=0)
		{
		return themes[theme_target];
		}
	return FMTtheme();
	}


FMToutputcomparator::FMToutputcomparator(std::string name) : output_name(name)
	{

	}

bool FMToutputcomparator::operator()(const FMToutput& output) const
	{
	return output_name == output.getname();
	}


}

BOOST_CLASS_EXPORT_IMPLEMENT(Core::FMToutput)