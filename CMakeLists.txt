#[[
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
]]


cmake_minimum_required(VERSION 3.6)
project(FMT VERSION 0.9.1 LANGUAGES CXX)



set(FMTMAJOR "-DFMTMAJOR=${PROJECT_VERSION_MAJOR}")
add_definitions("${FMTMAJOR}")
set(FMTMINOR "-DFMTMINOR=${PROJECT_VERSION_MINOR}")
add_definitions("${FMTMINOR}")
set(FMTPATCH "-DFMTPATCH=${PROJECT_VERSION_PATCH}")
add_definitions("${FMTPATCH}")
message("Generating Build file for FMT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(FMTSOURCEDIR "${CMAKE_CURRENT_SOURCE_DIR}/Source/")
set(FMTINCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/Source/")


if (DEFINED MOSEK_DIR)
	set(ENV{MOSEK_DIR} "${MOSEK_DIR}")
endif(DEFINED MOSEK_DIR)

if (DEFINED OSI_DIR)
	set(ENV{OSI_DIR} "${OSI_DIR}")
endif(DEFINED OSI_DIR)

if (DEFINED BOOST_DIR)
	set(ENV{BOOSTROOT} "${BOOST_DIR}")
endif(DEFINED BOOST_DIR)

if (DEFINED GDAL_DIR)
	set(ENV{GDAL_DIR} "${GDAL_DIR}")
endif(DEFINED GDAL_DIR)

if (DEFINED R_DIR)
	set(ENV{R_DIR} "${R_DIR}")
endif(DEFINED R_DIR)

if (DEFINED GEOS_DIR)
	set(ENV{GEOS_DIR} "${GEOS_DIR}")
endif(DEFINED GEOS_DIR)

if (DEFINED PYTHON_INCLUDE)
	set(ENV{PYTHON_DIR} "${PYTHON_INCLUDE}")
endif(DEFINED PYTHON_INCLUDE)

if (DEFINED PYTHON_DIR)
	set(ENV{PYTHON_DIR} "${PYTHON_DIR}")
endif(DEFINED PYTHON_DIR)


if (MSVC)
	add_definitions("-DFMT_EXPORTS")
	add_definitions("-D_WINDOWS")
	add_definitions("-D_USRDLL")
	add_definitions("/bigobj")
	add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif(MSVC)

add_definitions("-DBOOST_PYTHON_STATIC_LIB")
add_definitions("-DBOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS")
add_definitions("-DHAVE_SNPRINTF")

FILE(GLOB FMTSOURCEFILES ${FMTSOURCEDIR}*.cpp)
add_library(FMT SHARED ${FMTSOURCEFILES})
include_directories(${FMTINCLUDEDIR})
set_target_properties(FMT PROPERTIES PREFIX "")


set_property(TARGET FMT PROPERTY CXX_STANDARD 11)
set_property(TARGET FMT PROPERTY CXX_STANDARD_REQUIRED ON)

<<<<<<< HEAD
=======
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
    message(FATAL_ERROR "Insufficient g++ version you need at least 4.9.0")
  endif()
endif()

>>>>>>> Replanning
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/modules)

if (NOT MSVC AND DEFINED ENV{R_DIR})
	find_package(R REQUIRED)
	if (R_FOUND)
		message("Found R include here: ${R_INCLUDE_DIR}")
		set(FMTWITHR "-DFMTWITHR")
		add_definitions("${FMTWITHR}")
		include_directories(${R_INCLUDE_DIR})
		set(ENV{RCPP_DIR} "$ENV{R_DIR}")
		find_package(Rcpp REQUIRED)
		if (RCPP_FOUND)
			message("Found Rcpp include here: ${RCPP_INCLUDE_DIR}")
			include_directories(${RCPP_INCLUDE_DIR})
			get_filename_component(R_DLL_DIR ${R_DLL} DIRECTORY)
			target_link_libraries(FMT "-L${R_DLL_DIR}")
			foreach(linkerflags ${R_LINKER_FLAGS})
				target_link_libraries(FMT ${linkerflags})
			endforeach()
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols")
		endif()
	endif()
endif()

<<<<<<< HEAD


if (NOT DEFINED ENV{R_DIR} AND DEFINED ENV{PYTHON_DIR})
	if(NOT DEFINED PYTHON_INCLUDE)
		FILE(GLOB_RECURSE PYTHON_INCLUDE $ENV{PYTHON_DIR}include/Python.h)
	endif(NOT DEFINED PYTHON_INCLUDE)
	get_filename_component(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE} DIRECTORY)
=======
if ((NOT DEFINED ENV{R_DIR}) AND (DEFINED ENV{PYTHON_DIR}))
	#if(NOT DEFINED PYTHON_INCLUDE)
	#	FILE(GLOB_RECURSE PYTHON_INCLUDE $ENV{PYTHON_DIR}include/Python.h)
	#	if(PYTHON_INCLUDE)
    #        list(GET PYTHON_INCLUDE 0 FIRSTINCLUDE)
    #        get_filename_component(PYTHON_INCLUDE_DIR ${FIRSTINCLUDE} DIRECTORY)
    #    endif(PYTHON_INCLUDE)
	#endif(NOT DEFINED PYTHON_INCLUDE)
>>>>>>> Replanning
	find_package(PythonLibs)
	if (PythonLibs_FOUND)
		string(REGEX MATCH "^([0-9]*)(.)([0-9]*)(.)([0-9]*)" dummy ${PYTHONLIBS_VERSION_STRING})
		set(PYTHONNAMING "python${CMAKE_MATCH_1}${CMAKE_MATCH_3}")
<<<<<<< HEAD
		set(FMTWITHPYTHON "-DFMTWITHPYTHON")
		add_definitions("${FMTWITHPYTHON}")
		message("Found Python include here: ${PYTHON_INCLUDE_DIRS}")
=======
		set(PYTHONMAJOR "${CMAKE_MATCH_1}")
		set(FMTWITHPYTHON "-DFMTWITHPYTHON")
		add_definitions("${FMTWITHPYTHON}")
		message("Found Python include here: ${PYTHON_INCLUDE_DIRS}")
		message("Found Python library here: ${PYTHON_LIBRARIES}")
>>>>>>> Replanning
		include_directories(${PYTHON_INCLUDE_DIRS})
		target_link_libraries(FMT ${PYTHON_LIBRARIES})
		set_target_properties(FMT PROPERTIES SUFFIX ".pyd")
	else()
		message("Python not found")
	endif(PythonLibs_FOUND)
endif()

<<<<<<< HEAD

find_package(Boost REQUIRED)
file(GLOB_RECURSE Boost_LIBRARIES_PATHS $ENV{BOOSTROOT}*${CMAKE_STATIC_LIBRARY_SUFFIX})
foreach(boostlibpath ${Boost_LIBRARIES_PATHS})
	get_filename_component(libname ${boostlibpath} NAME)
	if( NOT ("${libname}" MATCHES ".dll.a") AND
	("${libname}" MATCHES "libboost" AND NOT "${libname}" MATCHES "-s") AND
	(("${libname}" MATCHES "zlib") OR
	("${libname}" MATCHES "system") OR
	("${libname}" MATCHES "serialization") OR
	("${libname}" MATCHES "iostreams") OR
	(PythonLibs_FOUND AND "${libname}" MATCHES "${PYTHONNAMING}"))
	)
		if (((("${libname}" MATCHES "d-x64-") OR ("${libname}" MATCHES "gd-x64")) AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")) OR
		((NOT (("${libname}" MATCHES "d-x64-") OR ("${libname}" MATCHES "gd-x64"))) AND("${libname}" MATCHES "-x64-") AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")) OR
		CYGWIN )
			list(APPEND Boost_LIBRARIES ${boostlibpath})
		endif()
	endif()
endforeach()
if (NOT MSVC)
	target_link_libraries(FMT "-lbz2")
	target_link_libraries(FMT "-lz")
ENDIF(NOT MSVC)
=======
#First try for decent installation of boost
if (DEFINED PYTHONNAMING)
        if("${PYTHONMAJOR}" STREQUAL "2")
            set(BPY "python")
        else()
            set(BPY "python${PYTHONMAJOR}")
        endif("${PYTHONMAJOR}" STREQUAL "2")
        find_package(Boost COMPONENTS system filesystem serialization iostreams "${BPY}") #add zlib
    else()
        find_package(Boost COMPONENTS system filesystem serialization iostreams) #add zlib
endif(DEFINED PYTHONNAMING)

if (Boost_FOUND)
    set(Complete_boost "complete")
endif(Boost_FOUND)

#the hard way
if (NOT DEFINED Boost_FOUND)
    find_package(Boost REQUIRED)
    file(GLOB_RECURSE Boost_LIBRARIES_PATHS $ENV{BOOSTROOT}*${CMAKE_STATIC_LIBRARY_SUFFIX})
    foreach(boostlibpath ${Boost_LIBRARIES_PATHS})
        get_filename_component(libname ${boostlibpath} NAME)
        if( NOT ("${libname}" MATCHES ".dll.a") AND
        ("${libname}" MATCHES "libboost" AND NOT "${libname}" MATCHES "-s") AND
        (("${libname}" MATCHES "zlib") OR
        ("${libname}" MATCHES "system") OR
        ("${libname}" MATCHES "serialization") OR
        ("${libname}" MATCHES "iostreams") OR
        (PythonLibs_FOUND AND "${libname}" MATCHES "${PYTHONNAMING}"))
        )
            if (((("${libname}" MATCHES "d-x64-") OR ("${libname}" MATCHES "gd-x64")) AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")) OR
            ((NOT (("${libname}" MATCHES "d-x64-") OR ("${libname}" MATCHES "gd-x64"))) AND("${libname}" MATCHES "-x64-") AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")) OR
            CYGWIN )
                list(APPEND Boost_LIBRARIES ${boostlibpath})
                set(Complete_boost "complete")
            endif()
        endif()
    endforeach()
endif(NOT DEFINED Boost_FOUND)

if (NOT Complete_boost)
    message(FATAL_ERROR "Boost not found!")
endif(NOT Complete_boost)


if (NOT MSVC)
	target_link_libraries(FMT "-lbz2")
	target_link_libraries(FMT "-lz")
ENDIF(NOT MSVC)
>>>>>>> Replanning



message("Found Boost include here: ${Boost_INCLUDE_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})
message("Found Boost libraries here: ${Boost_LIBRARIES}")
target_link_libraries(FMT ${Boost_LIBRARIES})



if (DEFINED ENV{GDAL_DIR})
<<<<<<< HEAD
	if(CYGWIN)
		find_package(GDAL)
		if(GDAL_FOUND)
			exec_program(${GDAL_CONFIG} ARGS --dep-libs OUTPUT_VARIABLE GDAL_CONFIG_LIBS)
		endif(GDAL_FOUND)
	else()
		find_package(GDALCUSTOM)
	endif(CYGWIN)
	if (GDAL_FOUND)
		set(FMTWITHGDAL "-DFMTWITHGDAL")
		message("Found GDAL include here: ${GDAL_INCLUDE_DIR}")
		add_definitions("${FMTWITHGDAL}")
		include_directories(${GDAL_INCLUDE_DIR})
		message("Found GDAL library here: ${GDAL_LIBRARY}")
		target_link_libraries(FMT ${GDAL_LIBRARY})
		if (NOT MSVC)
			foreach(linkerflags ${GDAL_LINKER_FLAGS})
				target_link_libraries(FMT ${linkerflags})
			endforeach()
		endif(NOT MSVC)
		if (WIN32 AND DEFINED GDAL_DLL)
			message("GDAL DLL found here: ${GDAL_DLL}")
			LIST(APPEND FMTDEPENDENCIES ${GDAL_DLL})
		endif()
		link_directories(${GDAL_LIBRARY_DIRS})
		if ( NOT CYGWIN)
			find_package(GEOS)
			if (GEOS_FOUND)
				message("Found GEOS libraries here: ${GEOS_LIBRARIES}")
				target_link_libraries(FMT ${GEOS_LIBRARIES})
				if (WIN32 AND DEFINED GEOS_DLL)
					message("GEOS DLL found here: ${GEOS_DLL}")
					LIST(APPEND FMTDEPENDENCIES ${GEOS_DLL})
				endif()
			endif(GEOS_FOUND)
		endif( NOT CYGWIN)
	else()
		message("GDAL not found take a look at (https://github.com/OSGeo/gdal)")
		message("Compile GDAL with GEOS for more functionalities (https://github.com/libgeos/geos)")
	endif(GDAL_FOUND)
=======
    find_package(GDAL)
    if(GDAL_FOUND)
        exec_program(${GDAL_CONFIG} ARGS --dep-libs OUTPUT_VARIABLE GDAL_CONFIG_LIBS)
        exec_program("gdalinfo" ARGS --version OUTPUT_VARIABLE GDAL_FULLVERSION)
        string(REGEX MATCH "(GDAL)([ ]*)([0-9])(\.)([0-9])(\.)([0-9])(,)(.+)" dummy ${GDAL_FULLVERSION})
        set(GDAL_MAJOR "${CMAKE_MATCH_3}")
        set(GDAL_MINOR "${CMAKE_MATCH_5}")
        FILE(GLOB_RECURSE GDAL_DATAEL $ENV{GDAL_DIR}ellipsoid.csv)
        if (NOT GDAL_DATAEL)
            if (EXISTS "/usr/share/gdal/")
                set(GDAL_DATA "/usr/share/gdal/")
            endif()
        else()
            get_filename_component(GDAL_DATAEL ${GDAL_DATA} DIRECTORY)
        endif()
        if ("${GDAL_MAJOR}.${CMAKE_MATCH_5}" VERSION_LESS "2.3")
             message(WARNING "You need at least GDAL 2.3 to get GDAL functionalities")
             unset(GDAL_FOUND)
        endif()
        if(WIN32 AND NOT CYGWIN)
            #Go find the DLL
            FILE(GLOB_RECURSE GDAL_POTDLL $ENV{GDAL_DIR}gdal*.dll)
            list(GET GDAL_POTDLL 0 GDAL_DLL)
        endif(WIN32 AND NOT CYGWIN)
    else()
        find_package(GDALCUSTOM)
    endif(GDAL_FOUND)
        if (GDAL_FOUND)
            set(FMTWITHGDAL "-DFMTWITHGDAL")
            message("Found GDAL include here: ${GDAL_INCLUDE_DIR}")
            add_definitions("${FMTWITHGDAL}")
            include_directories(${GDAL_INCLUDE_DIR})
            message("Found GDAL library here: ${GDAL_LIBRARY}")
            target_link_libraries(FMT ${GDAL_LIBRARY})
            if (NOT MSVC)
                foreach(linkerflags ${GDAL_LINKER_FLAGS})
                    target_link_libraries(FMT ${linkerflags})
                endforeach()
            endif(NOT MSVC)
            if (WIN32 AND DEFINED GDAL_DLL)
                message("GDAL DLL found here: ${GDAL_DLL}")
                LIST(APPEND FMTDEPENDENCIES ${GDAL_DLL})
            endif()
            link_directories(${GDAL_LIBRARY_DIRS})
            if (NOT GDAL_CONFIG_LIBS OR (NOT GDAL_CONFIG_LIBS MATCHES "lgeos_c"))
                find_package(GEOS)
                if (GEOS_FOUND)
                    message("Found GEOS libraries here: ${GEOS_LIBRARIES}")
                    target_link_libraries(FMT ${GEOS_LIBRARIES})
                    if (WIN32 AND DEFINED GEOS_DLL)
                        message("GEOS DLL found here: ${GEOS_DLL}")
                        LIST(APPEND FMTDEPENDENCIES ${GEOS_DLL})
                    endif()
                endif(GEOS_FOUND)
            endif()
        else()
            message("GDAL not found take a look at (https://github.com/OSGeo/gdal)")
            message("Compile GDAL with GEOS for more functionalities (https://github.com/libgeos/geos)")
        endif(GDAL_FOUND)
>>>>>>> Replanning
endif(DEFINED ENV{GDAL_DIR})


if (DEFINED ENV{OSI_DIR})
	find_package(OSI)
	if (OSI_FOUND)
		set(FMTWITHOSI "-DFMTWITHOSI")
		add_definitions("${FMTWITHOSI}")
		message("Found OSI include here: ${OSI_INCLUDE_DIRS}")
		include_directories(${OSI_INCLUDE_DIRS})
		message("Found OSI library here: ${OSI_LIBRARIES}")
		target_link_libraries(FMT ${OSI_LIBRARIES})
		if (NOT MSVC)
			foreach(linkerflags ${OSI_LINKER_FLAGS})
				target_link_libraries(FMT ${linkerflags})
			endforeach()
		endif(NOT MSVC)
<<<<<<< HEAD
		find_package(MOSEK)
		if (DEFINED ENV{MOSEK_DIR})
=======
		if (DEFINED ENV{MOSEK_DIR} AND (MSVC OR OSI_MSK_LIBRARY))
            find_package(MOSEK)
>>>>>>> Replanning
			if (MOSEK_FOUND)
				set(FMTWITHMOSEK "-DFMTWITHMOSEK")
				add_definitions("${FMTWITHMOSEK}")
				message("Found Mosek include here: ${MOSEK_INCLUDE_DIR}")
				include_directories(${MOSEK_INCLUDE_DIR})
				message("Found Mosek library here: ${MOSEK_LIB}")
				target_link_libraries(FMT ${MOSEK_LIB})
				include_directories(${OSIMSK_INCLUDE})
				if (NOT MSVC)
					foreach(linkerflags ${MOSEK_LINKER_FLAGS})
						target_link_libraries(FMT ${linkerflags})
					endforeach()
				else()
					target_sources(FMT PRIVATE ${OSIMSK_DEFINITION})
				endif(NOT MSVC)
				if (WIN32 AND DEFINED MOSEK_DLL)
					message("MOSEK DLL found here: ${MOSEK_DLL}")
					LIST(APPEND FMTDEPENDENCIES ${MOSEK_DLL})
				endif(WIN32 AND DEFINED MOSEK_DLL)
			endif(MOSEK_FOUND)
<<<<<<< HEAD
		endif(DEFINED ENV{MOSEK_DIR})
=======
		endif(DEFINED ENV{MOSEK_DIR} AND (MSVC OR OSI_MSK_LIBRARY))
>>>>>>> Replanning
	else()
		message("Get OSI and CLP throught CBC project: (https://github.com/coin-or/Cbc)")
	endif(OSI_FOUND)
endif(DEFINED ENV{OSI_DIR})


<<<<<<< HEAD

#install part
if (NOT MSVC)
	install (CODE
		"#Copy gdal data
		file(COPY \"${GDAL_DATA}\" DESTINATION \"${CMAKE_BINARY_DIR}\")
		file(RENAME \"${CMAKE_BINARY_DIR}/data\" \"${CMAKE_BINARY_DIR}/GDAL_DATA\")
		set(DEPENDENCIES \"${FMTDEPENDENCIES}\")
		#Copy dependencies
		foreach(FMTdep \${DEPENDENCIES}\)
			get_filename_component(depname \"\${FMTdep}\" NAME)
			file(COPY \"\${FMTdep}\" DESTINATION \"${CMAKE_BINARY_DIR}\")
		endforeach()")
else()
	install (CODE
		"#Copy gdal data
		file(COPY \"${GDAL_DATA}\" DESTINATION \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}\")
		file(RENAME \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/data\" \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/GDAL_DATA\")
		set(DEPENDENCIES \"${FMTDEPENDENCIES}\")
		#Copy dependencies
		foreach(FMTdep \${DEPENDENCIES}\)
			get_filename_component(depname \"\${FMTdep}\" NAME)
			file(COPY \"\${FMTdep}\" DESTINATION \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}\")
		endforeach()")
endif(NOT MSVC)
=======
#install part
if (NOT MSVC)
	install (CODE
        if (GDAL_FOUND)
            "#Copy gdal data
            file(COPY \"${GDAL_DATA}\" DESTINATION \"${CMAKE_BINARY_DIR}\")
            file(RENAME \"${CMAKE_BINARY_DIR}/data\" \"${CMAKE_BINARY_DIR}/GDAL_DATA\")
        endif()
        if (FMTDEPENDENCIES)
            set(DEPENDENCIES \"${FMTDEPENDENCIES}\")
            #Copy dependencies
            foreach(FMTdep \${DEPENDENCIES}\)
                get_filename_component(depname \"\${FMTdep}\" NAME)
                file(COPY \"\${FMTdep}\" DESTINATION \"${CMAKE_BINARY_DIR}\")
            endforeach()
        endif()")
else()
	install (CODE
        if (GDAL_FOUND)
            "#Copy gdal data
            file(COPY \"${GDAL_DATA}\" DESTINATION \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}\")
            file(RENAME \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/data\" \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/GDAL_DATA\")
        endif()
        if (FMTDEPENDENCIES)
            set(DEPENDENCIES \"${FMTDEPENDENCIES}\")
            #Copy dependencies
            foreach(FMTdep \${DEPENDENCIES}\)
                get_filename_component(depname \"\${FMTdep}\" NAME)
                file(COPY \"\${FMTdep}\" DESTINATION \"${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}\")
            endforeach()
		endif()")
endif(NOT MSVC)
>>>>>>> Replanning