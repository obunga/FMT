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

#include "FMTareaparser.h"
#include "FMTsasolution.h"
#include "FMTsamodel.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <math.h>
#include <cmath>
#include <cfloat>

namespace Spatial
{

    FMTsasolution::FMTsasolution():FMTlayer<Graph::FMTgraph>(),solution_stats(),events(),objectivefunctionvalue(), outputscache(),constraint_outputs_penalties()
    {

    }

    FMTsasolution::FMTsasolution(const FMTforest& initialmap):events(),objectivefunctionvalue(), outputscache(),constraint_outputs_penalties()
    {
        FMTlayer<Graph::FMTgraph>::operator = (initialmap.copyextent<Graph::FMTgraph>());//Setting layer information
        for(std::map<FMTcoordinate,Core::FMTdevelopment>::const_iterator devit = initialmap.mapping.begin(); devit != initialmap.mapping.end(); ++devit)
        {
			std::vector<Core::FMTactualdevelopment> actdevelopment;
            actdevelopment.push_back(Core::FMTactualdevelopment (devit->second,initialmap.getcellsize()));
            Graph::FMTgraph local_graph(Graph::FMTgraphbuild::schedulebuild);
            std::queue<Graph::FMTvertex_descriptor> actives = local_graph.initialize(actdevelopment);
            mapping[devit->first] = local_graph;
            solution_stats += local_graph.getstats();
        }
    }

    FMTsasolution::FMTsasolution(const FMTsasolution& rhs):
            FMTlayer<Graph::FMTgraph>(rhs),
            solution_stats(rhs.solution_stats),
            events(rhs.events),
            objectivefunctionvalue(rhs.objectivefunctionvalue),
			outputscache(rhs.outputscache),
            constraint_outputs_penalties(rhs.constraint_outputs_penalties)
    {


    }

    FMTsasolution& FMTsasolution::operator = (const FMTsasolution& rhs)
    {
    if (this!=&rhs)
        {
        FMTlayer<Graph::FMTgraph>::operator = (rhs);
        solution_stats = rhs.solution_stats;
        events = rhs.events;
        objectivefunctionvalue = rhs.objectivefunctionvalue;
		if (outputscache.size() < rhs.outputscache.size())
			{
			outputscache = rhs.outputscache;
			}
        constraint_outputs_penalties = rhs.constraint_outputs_penalties;
        }
    return *this;
    }

    bool FMTsasolution::operator == (const FMTsasolution& rhs) const
    {
    for (std::map<FMTcoordinate,Graph::FMTgraph>::const_iterator mainsolutionit = this->mapping.begin();
                                                     mainsolutionit != this->mapping.end(); ++mainsolutionit)
        {
            if (mainsolutionit->second != rhs.mapping.at(mainsolutionit->first))
            {
                return false;
            }
        }
    return true;
    }

    bool FMTsasolution::operator != (const FMTsasolution& rhs) const
    {
        return (!(*this==rhs));
    }

// ++++++++++++++++++++++++++++++ Function to get info on the solution +++++++++++++++++++++++++++++++++++

    double FMTsasolution::getobjfvalue()const
    {
        return objectivefunctionvalue;
    }

    Graph::FMTgraphstats FMTsasolution::getsolution_stats() const
    {
        return solution_stats;
    }
    const std::vector<std::vector<std::vector<FMTevent<Graph::FMTgraph>>>>& FMTsasolution::getevents() const
    {
        return events;
    }

    void FMTsasolution::getstartstop(const Core::FMTconstraint& constraint,int& periodstart,int& periodstop) const
    {
		std::map<FMTcoordinate,Graph::FMTgraph>::const_iterator graphit = this->mapping.begin();
        const Graph::FMTgraph* local_graph = &graphit->second;
        local_graph->constraintlenght(constraint,periodstart,periodstop);
    }

	std::vector<double> FMTsasolution::getgraphsoutputs(const Models::FMTmodel& model, const Core::FMTconstraint& constraint,
                                           const int& periodstart,const int& periodstop) const
    // Return sum of all graphs outputs related to constraint
    {
		std::vector<double>periods_values(periodstop-periodstart+1,0);
        const std::vector<double> solutions(1,this->getcellsize());
        for(std::map<FMTcoordinate,Graph::FMTgraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
            {
               const Graph::FMTgraph* local_graph = &graphit->second;
			   const size_t hashvalue = local_graph->hash(constraint.Core::FMToutput::hash());
			   std::vector<double>graphvalues(periodstop - periodstart + 1, 0);
				if (outputscache.find(hashvalue)!= outputscache.end())
					{
					graphvalues = outputscache.at(hashvalue);
				}else {
				for (int period = 1; period < local_graph->size()-1;++period)
					{
					const std::map<std::string, double> output = local_graph->getoutput(model, constraint, period, &solutions[0], Graph::FMToutputlevel::totalonly);
					const double totalperiod = output.at("Total");
					graphvalues[period - 1] += totalperiod;
					}
				outputscache[hashvalue] = graphvalues;
				}
				for (int period = periodstart; period <= periodstop; ++period)
					{
					periods_values[period-1] += graphvalues[period-1];
					}
				
			}
        return periods_values;
    }

double FMTsasolution::getgraphspenalties(const Models::FMTsamodel& model, const Core::FMTconstraint& constraint,
                                         const double& coef, std::vector<double>& output_vals, std::vector<double>& penalties_vals)

    //Find output by period and evaluate penalties for each. Finally sum penalties
    {
        int periodstart=0;
        int periodstop=0;
        getstartstop(constraint,periodstart,periodstop);
        double sumpenalties=0;//Sum of penalties for each period
        output_vals = getgraphsoutputs(model,constraint,periodstart,periodstop);
        int period = periodstart;
        for  (const double& value : output_vals)
        {
            double lower = 0;
            double upper = 0;
            constraint.getbounds(lower,upper,period);
            double penalties = this->applypenalty(upper,lower,value,coef,FMTsapenaltytype::exponential);
            penalties_vals.push_back(penalties);
            sumpenalties+=penalties;
            period++;
        }
        return sumpenalties;
    }

    double FMTsasolution::getspatialpenalties(const Models::FMTsamodel& model, const Core::FMTconstraint& constraint,
                                            const double& coef, std::vector<double>& output_vals, std::vector<double>& penalties_vals) const //Use of spatial actions as objectives
    // See to add greenup eventualy and set by bounds and period the shit
    {
		std::vector<FMTspatialaction> spatialactions = model.getspatialactions();
        double spatialpenalties = 0;
        double lower=0;
        double upper=0;
		std::vector<FMTspatialaction>::iterator actionit = std::find_if(spatialactions.begin(),spatialactions.end(),[constraint] (const FMTspatialaction& spaction) {return spaction.getname() == constraint.name;});
        const int action_id = static_cast<int>(std::distance(spatialactions.begin(), actionit));
        const FMTspatialaction spaction = spatialactions.at(action_id);
        size_t period = 1;
        for (const std::vector<std::vector<FMTevent<Graph::FMTgraph>>>& period_actions : events)
        {
            if (!period_actions.empty())
            {
                constraint.getbounds(lower,upper,static_cast<int>(period));
				std::vector<FMTevent<Graph::FMTgraph>> action_events = period_actions.at(action_id);
                if(!action_events.empty())
                {
                    if (constraint.getconstrainttype()==Core::FMTconstrainttype::FMTspatialadjacency)
                    {
						std::vector<FMTevent<Graph::FMTgraph>> potential_neigbors;
                        //Getting events that can be considerate as neighbors
                        for (std::vector<std::vector<FMTevent<Graph::FMTgraph>>>::const_iterator action_events_it = period_actions.begin();action_events_it != period_actions.end();++action_events_it) //To navigate through neighbors
                        {
                            for (std::vector<std::string>::const_iterator neighbors_it = spaction.neighbors.begin();neighbors_it!=spaction.neighbors.end();++neighbors_it)
                            {
                                if (spatialactions.at(action_events_it-period_actions.begin()).getname() == *neighbors_it)//Where iterator is - begin == index so we compare the name of the action and neighbors in neighbors list
                                {
                                    const std::vector<FMTevent<Graph::FMTgraph>> value = *action_events_it;
                                    potential_neigbors.insert(potential_neigbors.end(),value.begin(),value.end());//Insert FMTevent<FMTgraph> of each action in neighbors
                                }
                            }
                        }
                        double output_val = 0;
                        double penalties_val = 0;
                        for (const FMTevent<Graph::FMTgraph>& event : action_events)
                        {
                            if (!event.empty())
                            {
                                const double event_val = static_cast<double>(event.minimaldistance(potential_neigbors, static_cast<unsigned int>(lower)));
                                if (event_val > lower)
                                {
                                    output_val+=1;
                                }
                                //Fix FMTconstraint and change upper lower for constraint
                                penalties_val += applypenalty(std::numeric_limits<double>::infinity(),lower,event_val,coef,FMTsapenaltytype::linear);
                            }
                        }
                        output_vals.push_back((output_val/action_events.size()));
                        penalties_vals.push_back(penalties_val);
                        spatialpenalties+=penalties_val;
                    }
                    if (constraint.getconstrainttype()==Core::FMTconstrainttype::FMTspatialsize)
                    {
                        const double maxsize = static_cast<double>(upper);
                        const double minsize = static_cast<double>(lower);
                        double output_val = 0;
                        double penalties_val = 0;
                        for (const FMTevent<Graph::FMTgraph>& event : action_events)
                        {
                            if (!event.empty())
                            {
                                const double event_val = static_cast<double>(event.elements.size());
                                if (event_val > upper || event_val < lower)
                                {
                                    output_val+=1;
                                }
                                //Fix FMTconstraint and change upper lower for constraint and coef
                                penalties_val += applypenalty(maxsize,minsize,event_val,coef,FMTsapenaltytype::linear);
                            }
                        }
                        output_vals.push_back((output_val/action_events.size()));
                        penalties_vals.push_back(penalties_val);
                        spatialpenalties+=penalties_val;
                    }
                }
            }
            period++;
        }
        return spatialpenalties;
    }

    FMTforest FMTsasolution::getforestperiod(const int& period) const
    {
        FMTforest forest(this->copyextent<Core::FMTdevelopment>());//Setting layer information
        for(std::map<FMTcoordinate,Graph::FMTgraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
        {
            const Graph::FMTgraph* local_graph = &graphit->second;
            const std::vector<double> solutions(1,this->getcellsize());
			std::vector<Core::FMTactualdevelopment> actdev = local_graph->getperiodstopdev(period,&solutions[0]);//
            forest.mapping[graphit->first]=Core::FMTdevelopment(actdev.front());
        }
        return forest;
    }

// ++++++++++++++++++++++++++++++ Function to manipulate solution +++++++++++++++++++++++++++++++++++

	Graph::FMTgraphstats FMTsasolution::buildperiod(const Models::FMTmodel& model, std::default_random_engine& generator)
        {
            Graph::FMTgraphstats periodstats = Graph::FMTgraphstats();
			std::vector<std::vector<FMTevent<Graph::FMTgraph>>> events_id(model.getactions().size());
            for(std::map<FMTcoordinate,Graph::FMTgraph>::iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)//change const_iterator to iterator because graph is modified
            {
                Graph::FMTgraph* local_graph = &graphit->second;
                std::queue<Graph::FMTvertex_descriptor> actives = local_graph->getactiveverticies();
                Graph::FMTgraphstats stats = local_graph->randombuild(model,actives,generator,events_id,graphit->first);
                periodstats += local_graph->getstats();
            }
            events.push_back(events_id);
            solution_stats += periodstats;
            return periodstats;
        }

    FMTsasolution FMTsasolution::perturb(Models::FMTsamodel& model, std::default_random_engine& generator,FMTsamovetype movetype,const double min_ratio, const double max_ratio) const
    {
        FMTsasolution newsolution(*this);
        switch(movetype)
        {
        case FMTsamovetype::shotgun ://Create function shotgun move
        {
			std::vector<size_t> map_lenght(this->mapping.size());
            size_t i = 0;
            for (size_t& lsize : map_lenght)
            {
                lsize = i;
                ++i;
            }
            int min_cells = std::max(int((map_lenght.size()-1)*min_ratio),1);
            int max_cells = std::max(int((map_lenght.size()-1)*max_ratio),1);
			std::shuffle(map_lenght.begin(),map_lenght.end(),generator);//shuffle all indices in the map
			std::uniform_int_distribution<int> celldistribution(min_cells,max_cells);
            const int numbercells = celldistribution(generator);//Get number of cell to perturb
			std::vector<size_t> ChangedId;
			ChangedId.reserve(numbercells);
 
            for (int id = 0; id<numbercells; ++id)
            {
				std::map<FMTcoordinate,Graph::FMTgraph>::const_iterator luckygraph = this ->mapping.begin();
                std::advance(luckygraph,map_lenght.at(id));
				std::uniform_int_distribution<int> perioddistribution(1,luckygraph->second.size()-2);//period to change
                const int period = perioddistribution(generator);
                newsolution.solution_stats -= luckygraph->second.getstats();
                Graph::FMTgraph newgraph = luckygraph -> second.perturbgraph(model,generator,newsolution.events,luckygraph->first,period);//perturb cell
                newsolution.solution_stats += newgraph.getstats();
                newsolution.mapping[luckygraph->first] = newgraph;
                ChangedId.push_back(map_lenght.at(id));
            }
            bool mapid = model.setmapidmodified(ChangedId);
            break;
        }
        case FMTsamovetype::cluster :
            break;//to do list before 2025

        default :
            break;
        }
        return newsolution;
    }

    double FMTsasolution::exponentialpenalty(const double& xvalue,
                        const double& xlowerbound, const double& xupperbound,
                        const double& maxpenalty,
                        double curvelength,
                        double slope) const
    {
    //https://en.wikipedia.org/wiki/Exponential_distribution
    //change slope between 1 to 2 (1  = more linear)
    //If no curvelength find one
    if (!curvelength)
        {
         if (xlowerbound != -DBL_MAX)
            {
            curvelength = std::abs(xlowerbound);
            }else if(xupperbound != DBL_MAX)
                {
                curvelength = std::abs(xupperbound);
                }
        }
    if (curvelength != 0)
        {
        double basefive = 0;
        if (xvalue < xlowerbound)
            {
            basefive=(5.0/curvelength)*xvalue;
            if (xvalue < 0)
                {
                basefive=5-(5.0/curvelength)*(std::abs(xvalue)- std::abs(xlowerbound));
                }
            }else if(xvalue > xupperbound)
                {
                basefive = 5-((5.0/curvelength)*(xvalue-xupperbound));
                }else{
                    return 0;
                }
        return (slope*std::exp(-slope*(basefive)))*maxpenalty;
        }
    return 0;
    }

    double FMTsasolution::applypenalty(const double& upper,const double& lower, const double& value, const double& coef, const FMTsapenaltytype penalty_type)const
    {
        switch(penalty_type)
        {
            case FMTsapenaltytype::linear :
                {
                    double penaltyvalue = 0;
                    if (!std::isinf(lower))//If lower is not infinite
                    {
                        if(value<lower)//If value under lower bound penalize
                        {
                            penaltyvalue = -value+lower;
                        }
                    }
                    if (!std::isinf(upper))//If upper is not infinite
                    {
                        if (value>upper)//If value higher than upper bound penalize
                        {
                            penaltyvalue = value-upper;
                        }
                    }
                    return penaltyvalue*coef;
                }

            case FMTsapenaltytype::exponential:
                {
                    double lowerl = lower;
                    double upperl = upper;
                    double maxpenalty = 0;
                    if (std::isinf(lower))//If lower is infinite
                    {
                        lowerl = -DBL_MAX;
                        maxpenalty = upper*coef;

                    }
                        if (std::isinf(upper))//If upper is infinite
                    {
                        upperl = DBL_MAX;
                        maxpenalty = lower*coef;
                    }
                    else
                    {
                        maxpenalty = (std::max(lower,upper))*coef;
                    }
                    return exponentialpenalty(value,lowerl,upperl,maxpenalty);
                }

            default :
                break;
        }
    return 0;
    }

    double FMTsasolution::evaluate(const Models::FMTsamodel& model)
    {
		std::vector<Core::FMTconstraint> constraints = model.getconstraints();
        Core::FMTconstraint objective = constraints.front();
        double penalty_sense = 1;
		std::vector<std::string>penalties = objective.getpenalties(penalty_sense);//Return values in penalty
        if (penalties.empty())
            {
                _exhandler->raise(Exception::FMTexc::FMTunsupported_objective,FMTwssect::Optimize,"No penalties",__LINE__, __FILE__);
            }
        else
            {
                constraints.erase(constraints.begin());
				std::vector<std::string>::const_iterator all_it = std::find(penalties.begin(), penalties.end(), "ALL");
                double penalty_value = 0;
                for (const Core::FMTconstraint& constraint : constraints)
                {
                    Core::FMTconstrainttype type = constraint.getconstrainttype();
                    double value = 0;
					std::string name;
					std::string cname = boost::replace_all_copy(std::string(constraint),"\n","");
                    boost::replace_all(cname,","," ");
                    double coef = 1;
                    constraint.getgoal(name,coef);
                    if (!name.empty())
                    {
						std::vector<std::string>::const_iterator name_it = std::find(penalties.begin(), penalties.end(), name);
                        if (all_it != penalties.end() || name_it != penalties.end())
                        {
							std::vector<double> output_vals;
							std::vector<double> penalties_vals;
                            if ( type == Core::FMTconstrainttype::FMTspatialadjacency  || type == Core::FMTconstrainttype::FMTspatialsize || type == Core::FMTconstrainttype::FMTspatialgreenup )//Case spatial are in the section optimize
                            {
                                value = this->getspatialpenalties(model,constraint,coef, output_vals, penalties_vals);
                            }
                            else
                            {
                                value = this->getgraphspenalties(model,constraint,coef, output_vals, penalties_vals);//apply weight in applypenalty
                            }
                            constraint_outputs_penalties[cname+ std::to_string(constraint.getconstrainttype())] = std::pair<std::vector<double>, std::vector<double>>(output_vals,penalties_vals);
                        }
                        else//Case spatialobjectives are set with spatial action
                            {
							std::vector<double> output_vals;
							std::vector<double> penalties_vals;
                                if ( type == Core::FMTconstrainttype::FMTspatialadjacency  || type == Core::FMTconstrainttype::FMTspatialsize || type == Core::FMTconstrainttype::FMTspatialgreenup )
                                {
                                    value = this->getspatialpenalties(model,constraint,coef, output_vals, penalties_vals);
                                }
                                constraint_outputs_penalties[cname+ std::to_string(constraint.getconstrainttype())] = std::pair<std::vector<double>, std::vector<double>>(output_vals,penalties_vals);
                            }
                    }
                    penalty_value += value;
                }
                objectivefunctionvalue = penalty_value;
                return penalty_value;
            }
        return 0;
        }

    void FMTsasolution::write_events(const std::vector<Core::FMTaction> model_actions,const std::string out_path,const std::string addon) const
    //Add to parser
    {
      //One layer per action per period
        if (!events.empty())
        {
            int period = 1;//No events on period 0
            for (std::vector<std::vector<FMTevent<Graph::FMTgraph>>> period_v : events)
            {
                if (!period_v.empty())
                {
                    int action_id = 0;
                    for (std::vector<FMTevent<Graph::FMTgraph>> action_id_v : period_v)//Vector of events in action_id
                    {
                        if (!action_id_v.empty())
                        {
                            FMTlayer<int> action_layer(this->copyextent<int>());//Setting layer information
                            int event_id = 1; //To write in the map
							std::map<int, std::string> event_map;
                            for (FMTevent<Graph::FMTgraph> event : action_id_v)//The event
                            {
                                if (!event.empty())
                                {
 
                                    for (std::map<FMTcoordinate,const Graph::FMTgraph*>::const_iterator coordit = event.elements.begin(); coordit != event.elements.end(); ++coordit)
                                    {

                                        action_layer.mapping[coordit->first]=event_id;
                                    }
                                    event_map[event_id] = "Event_"+ std::to_string(event_id);
                                    event_id++;
                                }
                            }
                            if (action_layer.mapping.size()>0)
                            {
								const std::string action_name = model_actions.at(action_id).getname();
								const std::string out_location = out_path+action_name+"_"+addon+"_events_period_"+std::to_string(period)+".tif";
							#ifdef FMTWITHGDAL
								WSParser::FMTareaparser parser;
                                parser.writelayer(action_layer,out_location,event_map);
							#endif 
                            }
                        }
                        action_id++;
                    }
                }
                period++;
            }
        }
    }

	bool FMTsasolution::copyfromselected(const FMTsasolution& rhs, const std::vector<size_t>& selected)
		{
		std::map<FMTcoordinate, Graph::FMTgraph>::iterator baseit;
		std::map<FMTcoordinate, Graph::FMTgraph>::const_iterator newvalueit;
		//No location check sooo make sure you copy the same kind of solution...
		solution_stats = rhs.solution_stats;
		events = rhs.events;
		objectivefunctionvalue = rhs.objectivefunctionvalue;
		if (outputscache.size()<rhs.outputscache.size())
			{
			outputscache = rhs.outputscache;
			}
		constraint_outputs_penalties = rhs.constraint_outputs_penalties;
		if (this->size() == rhs.size())
			{
			for (const size_t& selection : selected)
				{
				baseit = std::next(this->mapping.begin(), selection);
				newvalueit = std::next(rhs.mapping.begin(), selection);
				baseit->second = newvalueit->second;
				}
			return true;
			}
		return false;
		}

	bool FMTsasolution::swapfromselected(FMTsasolution& rhs, const std::vector<size_t>& selected)
		{
		std::map<FMTcoordinate, Graph::FMTgraph>::iterator baseit;
		std::map<FMTcoordinate, Graph::FMTgraph>::iterator newvalueit;
		//No location check sooo make sure you copy the same kind of solution...
		solution_stats = rhs.solution_stats;
		events.swap(rhs.events);
		objectivefunctionvalue = rhs.objectivefunctionvalue;
		if (outputscache.size() < rhs.outputscache.size())
		{
			outputscache.swap(rhs.outputscache);
		}
		constraint_outputs_penalties.swap(rhs.constraint_outputs_penalties);
		if (this->size() == rhs.size())
		{
			for (const size_t& selection : selected)
			{
				baseit = std::next(this->mapping.begin(), selection);
				newvalueit = std::next(rhs.mapping.begin(), selection);
				baseit->second.swap(newvalueit->second);
			}
			return true;
		}
		return false;
		}

}
