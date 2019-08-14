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

#ifndef PYEXPORTMODEL_H_INCLUDED
#define PYEXPORTMODEL_H_INCLUDED


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(simulate_overloads,simulate, 1, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(buildperiod_overloads, buildperiod, 0, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(move_solution_overloads, move_solution, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(get_outputs_overloads, get_outputs, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getLPoutputoverloads,getoutput, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(warmup_overloads, warmup, 1, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(evaluate_overloads, evaluate, 1, 2)


void exportModel()
    {
    namespace bp = boost::python;
    bp::object ModelsModule(bp::handle<>(bp::borrowed(PyImport_AddModule("FMT.Models"))));
    bp::scope().attr("Models") = ModelsModule;
    bp::scope ModelsModule_scope = ModelsModule;
    bp::scope().attr("__doc__") = ".. module:: Model\n"
    "\n"
    "   :platform: Unix, Windows\n"
    "   :synopsis: Module used to generate all kind of Models (M2/M3).\n"
    "\n";
    const char* py_FMTmodel_doc =
        " ``FMTmodel`` class.\n"
        "\n"
        "Base model class for building general WS model\n"
        "\n";
    class_<Models::FMTmodel,bases<Exception::FMTobject>>("FMTmodel",py_FMTmodel_doc)
            .def_readwrite("name",&Models::FMTmodel::name)
			.def_pickle(FMT_pickle_suite<FMTmodel>())
            .def("getyields",&Models::FMTmodel::getyields)
            .def("getarea",&Models::FMTmodel::getarea, getarea_overloads())
            .def("getthemes",&Models::FMTmodel::getthemes)
            .def("getactions",&Models::FMTmodel::getactions)
			.def("getoutputs", &Models::FMTmodel::getoutputs)
			.def("getconstraints", &Models::FMTmodel::getconstraints)
			.def("setconstraints", &Models::FMTmodel::setconstraints)
			.def("getactionaggregates", &Models::FMTmodel::getactionaggregates)
            .def("gettransitions",&Models::FMTmodel::gettransitions)
            .def("settransitions",&Models::FMTmodel::settransitions)
            .def("getlifespan",&Models::FMTmodel::getlifespan)
            .def("valid",&Models::FMTmodel::valid);
    define_pylist<Models::FMTmodel>();

    const char* py_FMTsesm_doc =
        " ``FMTsesmodel`` class.\n"
        "\n"
        "Model class used to do spatial explicit simulation\n"
        "Uses schedule to simulate harvest on rasters\n"
        "\n";

	boost::python::to_python_converter<map<string, double>, MapToDict<string, double>>();

    class_<Models::FMTsesmodel,bases<Models::FMTmodel>>("FMTsesmodel",py_FMTsesm_doc)
            .def(init<Models::FMTmodel>())
			.def_pickle(FMT_pickle_suite<FMTsesmodel>())
            .def("getmapping",&FMTsesmodel::getmapping)
			.def("getschedule", &FMTsesmodel::getschedule,
				"Get the non spatial schedule for the last period\n")
            .def("getdisturbances",&FMTsesmodel::getdisturbances,
                 "Get all the disturbances of the simulation\n"
                 "The disturbances stack present all the past disturbances"
                 /*args("self","disturbances")*/)
            .def("setinitialmapping",&FMTsesmodel::setinitialmapping,
                 "Set the initial mapping\n"
                 "Initial forest themes described to the model"
                 /*args("self","mapping")*/)
            .def("setspactions",&FMTsesmodel::setspactions,
				"Set a spatial actions list to the model\n"
				"Model should contain the relative transitions\n"
				"Actions will be simulated following the list ordering\n")
			.def("getschedule",&FMTsesmodel::getschedule,
				"Get the last simulated non spatial schedule\n")
			.def("getdisturbancestats", &FMTsesmodel::getdisturbancestats,
				"Get all disturbances stats (period,action,size,perimeter,Height,Width)\n")
            .def("simulate",&FMTsesmodel::simulate,
                 simulate_overloads(/*"Simulate a schedule based only on schedule (schedule_only = true)\n"
                                    "or using schedule and operability(schedule_only = false) and a (seed=0)",
                                    args("self","schedule","schedule_only=true","seed=0")*/));
    define_pylist<Models::FMTsesmodel>();


	const char* py_FMTlpmodel =
		" ``FMTlpmodel`` class.\n"
		"\n"
		"This class is used for M3 optimization\n"
		"\n";

	enum_<Models::FMTsolverinterface>("FMTsolverinterface")
		.value("CLP", Models::FMTsolverinterface::CLP)
		.value("MOSEK", Models::FMTsolverinterface::MOSEK)
		.value("CPLEX", Models::FMTsolverinterface::CPLEX)
		.value("GUROBI", Models::FMTsolverinterface::GUROBI);

	class_<Models::FMTlpmodel, bases<Models::FMTmodel>>("FMTlpmodel", py_FMTlpmodel)
		.def(init<Models::FMTmodel, Models::FMTsolverinterface>())
		.def_pickle(FMT_pickle_suite<FMTlpmodel>())
		.def("buildperiod", &Models::FMTlpmodel::buildperiod, buildperiod_overloads())
		//.def("eraseperiod", &Models::FMTlpmodel::eraseperiod)
		.def("boundsolution", &Models::FMTlpmodel::boundsolution)
		.def("getsolution", &Models::FMTlpmodel::getsolution)
		.def("setsolution", &Models::FMTlpmodel::setsolution)
		.def("setobjective", &Models::FMTlpmodel::setobjective)
		.def("setconstraint", &Models::FMTlpmodel::setconstraint)
		.def("solve", &Models::FMTlpmodel::solve)
		.def("specificinitialsolve", &Models::FMTlpmodel::specificinitialsolve)
		.def("getoutput", &Models::FMTlpmodel::getoutput,getLPoutputoverloads())
		//.def("getarea", &Models::FMTlpmodel::getarea)
		.def("__eq__", &Models::FMTlpmodel::operator ==)
		.def("__ne__", &Models::FMTlpmodel::operator !=)
		//.def("samegraph", &Models::FMTlpmodel::samegraph)
		.def("getstats", &Models::FMTlpmodel::getstats);
		//.def("setgraph", &Models::FMTlpmodel::setgraph);

	define_pylist<Models::FMTlpmodel>();

	enum_<Models::FMTsawarmuptype>("FMTsawarmuptype")
		.value("log", Models::FMTsawarmuptype::log)
		.value("delta", Models::FMTsawarmuptype::bigdelta)
		.value("logmax", Models::FMTsawarmuptype::logmax)
		.value("bootstrapmagic", Models::FMTsawarmuptype::bootstrapmagic);

	const char* py_FMTsamodel_doc =
        " ``FMTsamodel`` class.\n"
        "\n"
        "Model class used to do simulated annealing\n"
        "Need to use cooling schedule\n"
        "\n";
     class_<Models::FMTsamodel,bases<Models::FMTmodel>>("FMTsamodel",py_FMTsamodel_doc)
            .def(init<Models::FMTmodel>())
            .def("get_current_solution",&FMTsamodel::get_current_solution)
            .def("get_new_solution",&FMTsamodel::get_new_solution)
            .def("getspatialactions",&FMTsamodel::getspatialactions)
            .def("evaluate",&FMTsamodel::evaluate,evaluate_overloads())
            .def("setinitial_mapping",&FMTsamodel::setinitial_mapping,
                 "Set the initial mapping\n"
                 "Initial forest themes described to the model",
                 args("self","mapping"))
            .def("setspactions",&FMTsamodel::setspactions,
				"Set a spatial actions list to the model\n"
				"Model should contain the relative transitions\n"
				"Actions will be simulated following the list ordering\n")
            .def("set_min_max_moves",&FMTsamodel::set_min_max_moves)
            .def("cool_down",&FMTsamodel::cool_down,
                 "Reduce temperature according to the cooling schedule\n")
            .def("get_cool_schedule_type",&FMTsamodel::getcoolingscheduletype,
                 "Return cooling schedule type associated with the model\n")
            .def("setschedule",&FMTsamodel::setschedule,
                 "Set a cooling schedule by passing a FMTsaschedule\n")
            .def("buildperiod",&FMTsamodel::buildperiod,
                 "Build each period one by one randomly\n")
            .def("move",&FMTsamodel::move_solution,
                 move_solution_overloads())
            .def("acceptnew",&FMTsamodel::acceptnew,
                 "Accept new solution as current solution and empty the new solution")
            .def("write_outputs_at",&FMTsamodel::write_outputs_at,
                 "Input : Path were you want the file with the outputs\n"
                 "Write the for each constraint and period the output and the penalty in a file name outputs.csv")
            .def("get_events_mean_size",&FMTsamodel::get_events_mean_size)
            .def("get_outputs",&FMTsamodel::get_outputs,get_outputs_overloads())
            .def("write_solutions_events",&FMTsamodel::write_solutions_events)
            .def("get_number_moves",&FMTsamodel::get_number_moves)
            .def("warmup",&FMTsamodel::warmup,warmup_overloads());
    define_pylist<Models::FMTsamodel>();
    }

#endif // PYEXPORTMODEL_H_INCLUDED
