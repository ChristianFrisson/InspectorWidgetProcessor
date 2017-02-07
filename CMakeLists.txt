#=============================================================================
# CMake integration Copyright 2015 Christian Frisson.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

project(InspectorWidgetProcessor)

cmake_minimum_required(VERSION 3.0)

# add some find scripts
list(APPEND CMAKE_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
file(GLOB CMAKEDIRLIST ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*)
foreach(CMAKEDIR ${CMAKEDIRLIST})
	if(IS_DIRECTORY ${CMAKEDIR})
		list(APPEND CMAKE_MODULE_PATHS "${CMAKEDIR}")
	endif()
endforeach()
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATHS}")
set(InspectorWidgetProcessor_VERSION 0.1pre)

if(${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})
	message(FATAL_ERROR "In-source building forbidden: create a 'Builds' subdirectory itself containing a directory describing your system and architecture.")
endif()

######################
#       OPTIONS      #
######################

option(BUILD_SHARED_LIBS "Build InspectorWidgetProcessor libraries as shared" ON)
option(USE_DEBUG "Use Debug" ON)
option(BUILD_CLI "Build InspectorWidgetProcessor CLI " OFF)
option(BUILD_TESTS "Build InspectorWidgetProcessor tests " OFF)

if(USE_DEBUG)
	message( "\nConfigured for Debug Build")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_DEBUG -DTI_DEBUG=1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_DEBUG -DTI_DEBUG=1")
	if(NOT WIN32)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
	endif()
	set(CMAKE_BUILD_TYPE "Debug")
	set(CMAKE_VERBOSE_MAKEFILE ON)
else(USE_DEBUG)
	message( "\nConfigured for Release Build")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTI_DEBUG=0 -O3")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTI_DEBUG=0 -O3")
	set(CMAKE_BUILD_TYPE "MinSizeRel")#Debug|Release|RelWithDebInfo|MinSizeRel
	set(CMAKE_VERBOSE_MAKEFILE ON)#CF
	if(APPLE)
		set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/release")
	endif()
endif(USE_DEBUG)

#########################
# EXTERNAL DEPENDENCIES #
#########################

#Threads
find_package(Threads)

# OpenCV
if(APPLE)
set(OpenCV_MIN_VERSION "3.0.0")
else()
set(OpenCV_MIN_VERSION "2.0.0")
endif()
find_path(OpenCV_DIR NAMES opencv-config.cmake OpenCVConfig.cmake PATH_SUFFIXES lib/cmake) # for MacPorts
if(WIN32)
	if(EXISTS $ENV{OPENCV_DIR})
		set(OpenCV_DIR $ENV{OPENCV_DIR} )
		find_path(OpenCV_DIR NAMES opencv-config.cmake OpenCVConfig.cmake)
	else(EXISTS $ENV{OPENCV_DIR})
		message("Please set OPENCV_DIR environnement variable to top folder containing OpenCVConfig.cmake")
	endif(EXISTS $ENV{OPENCV_DIR})
endif(WIN32)
if(APPLE)
find_package(OpenCV ${OpenCV_MIN_VERSION} REQUIRED COMPONENTS core features2d imgproc imgcodecs objdetect video highgui)
else()
find_package(OpenCV ${OpenCV_MIN_VERSION} REQUIRED COMPONENTS core features2d imgproc objdetect video highgui)
endif()
if(OpenCV_FOUND)
	if(OpenCV_opencv_highgui_EXTRA_DEPS_OPT AND CMAKE_TOOLCHAIN_FILE AND MINGW)
		string(REGEX MATCH ".*QtCore.*" OpenCV_WITH_Qt "${OpenCV_opencv_highgui_EXTRA_DEPS_OPT}")
		if(OpenCV_WITH_Qt)
			message("OpenCV with Qt")
		endif()
	endif()

	message("OpenCV found in ${OpenCV_INCLUDE_DIRS}")

	foreach(OpenCV_INCLUDE_DIR ${OpenCV_INCLUDE_DIRS})
		include_directories(${OpenCV_INCLUDE_DIR})
	endforeach(OpenCV_INCLUDE_DIR)
	if(MINGW OR WIN32)
		set(OpenCV_LIBRARIES ${OpenCV_LIBS_OPT} ${OpenCV_EXTRA_LIBS_OPT})
	else()
		set(OpenCV_LIBRARIES ${OpenCV_LIBS})
	endif()
	message("Found OpenCV ${OpenCV_VERSION} libraries ${OpenCV_LIBRARIES} and headers ${OpenCV_INCLUDE_DIRS}")
	#message( "OpenCV link directories: ${OpenCV_LIB_DIRECTORY}")
	foreach(OpenCV_LIB_DIRECTORY ${OpenCV_LIB_DIRS})
		add_linked_directory(${OpenCV_LIB_DIRECTORY})
	endforeach(OpenCV_LIB_DIRECTORY)
else(OpenCV_FOUND)
	message(FATAL_ERROR "OpenCV not found.")
endif(OpenCV_FOUND)

# Tesseract
find_package(Tesseract)
if(Tesseract_FOUND)
	message("Tesseract found: headers in ${Tesseract_INCLUDE_DIR} and library ${Tesseract_LIBRARY}")
else()
	message(FATAL_ERROR "Tesseract not found")
endif()

find_package(TBB)
if(TBB_FOUND)
	message("TBB found: headers in ${TBB_INCLUDE_DIR} and library ${TBB_LIBRARY}")
else()
	message("TBB not found")
endif()

# git
find_program(GIT NAMES git)
if(GIT)
	set(GIT_FOUND ON CACHE STRING "git found" FORCE)
	message("Found git: ${GIT}")
else()
	message(FATAL_ERROR "Could not find git. Please install git.\n")
endif()

# NODE
find_program(NODE NAMES node)
if(NODE)
	set(NODE_FOUND ON CACHE STRING "node found" FORCE)
	message("Found node: ${NODE}")
else()
	message(FATAL_ERROR "Could not find node.js. Please install node.js so that the 'node' command is available.\n")
endif()

# NPM
set(NPM_DEPENDECIES "cmake-js") #bower;grunt;http-server
find_program(NPM NAMES npm)
if(NPM)
	message("Found npm: ${NPM}")
	set(NPM_FOUND ON CACHE STRING "npm found" FORCE)
	foreach(NPM_DEPENDENCY ${NPM_DEPENDECIES})
		#message("Checking ${NPM_DEPENDENCY}")
		if(NOT NPM_${NPM_DEPENDENCY}_FOUND)
			#exec_program(${NPM} ARGS "list --parseable -g ${NPM_DEPENDENCY}" OUTPUT_VARIABLE NPM_DEP_OUT RESULT_VARIABLE NPM_DEP_RES)
			#string(REGEX MATCH "npm ERR! code" NPM_DEP_NOT_FOUND ${NPM_DEP_OUT})
			find_program(DEP_${NPM_DEPENDENCY} NAMES ${NPM_DEPENDENCY})
			set(NPM_DEP_OUT ${DEP_${NPM_DEPENDENCY}})
			if(NPM_DEP_OUT)#if(NOT NPM_DEP_NOT_FOUND)
				set(NPM_${NPM_DEPENDENCY}_FOUND ON CACHE STRING "${NPM_DEPENDENCY} found" FORCE)
				set(NPM_${NPM_DEPENDENCY} "${NPM_DEP_OUT}" CACHE STRING "${NPM_DEPENDENCY} path" FORCE)
			else()
				message(FATAL_ERROR "Couldn't find ${NPM_DEPENDENCY}. Please install it globally with: npm install ${NPM_DEPENDENCY} -g")
			endif()
		endif()
		message("Found ${NPM_DEPENDENCY}: ${NPM_${NPM_DEPENDENCY}}")
	endforeach()
else()
	message(FATAL_ERROR "Could not find npm. Please install npm\n")
endif()

#########################
# INTERNAL DEPENDENCIES #
#########################

find_path(RAPIDJSON_DIR PATH_SUFFIXES ${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-rapidjson/include)
find_path(PARTIALCSVPARSER_DIR PATH_SUFFIXES ${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-PartialCsvParser/include)
find_path(LIBUIOHOOK_DIR PATH_SUFFIXES ${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-libuiohook/include)
find_path(PEGTL_DIR PATH_SUFFIXES ${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-PEGTL)
if(NOT RAPIDJSON_DIR OR NOT PARTIALCSVPARSER_DIR OR NOT LIBUIOHOOK_DIR OR NOT PEGTL_DIR)
	execute_process(COMMAND ${GIT} submodule update --init ${CMAKE_CURRENT_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE GIT_OUT RESULT_VARIABLE GIT_RES)
endif()

set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/github-InspectorWidget-rapidjson)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-rapidjson/include)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-PartialCsvParser/include)

set(BUILD_APPS OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/github-InspectorWidget-libuiohook)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-libuiohook/include)

add_subdirectory(3rdparty/github-InspectorWidget-PEGTL)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-PEGTL)

add_subdirectory(3rdparty/github-InspectorWidget-pugixml)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/github-InspectorWidget-pugixml/src)

######################
# INTERNAL LIBRARIES #
######################

include_directories(lib)
add_subdirectory(lib)

######################
# INTERNAL LIBRARIES #
######################

if(BUILD_CLI)
include_directories(cli)
add_subdirectory(cli)
endif()

######################
#    NODEJS ADDON    #
######################

# Get and apply the cmake-js flags by executing it
execute_process(COMMAND ${NODE} ${NPM_cmake-js} print-configure -d ${CMAKE_CURRENT_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE PC_OUT RESULT_VARIABLE PC_RES)
#message("${NPM_cmake-js} print-configure -d ${CMAKE_CURRENT_SOURCE_DIR}: ${PC_OUT} // ${PC_RES}")
string(REGEX REPLACE ";" "§" PC_OUT "${PC_OUT}") # hack to handle lists passed as flags
string(REGEX MATCHALL " -D[A-Z_]+=\"[^\"]+\"" FLAGS "${PC_OUT}")
foreach(FLAG ${FLAGS})
	string(REGEX REPLACE "§" ";" FLAG "${FLAG}") # hack to handle lists passed as flags
	string(REGEX REPLACE "^ -D" "" FLAG "${FLAG}") # remove the CMake flag prefix
	string(REGEX MATCH "^[^=]+" CMAKE_FLAG_NAME "${FLAG}") # split the flag name
	string(REGEX MATCH "=.+$" CMAKE_FLAG_VALUE "${FLAG}") # split the flag value
	string(REGEX MATCH "[^=\"]+.+[^\"]" CMAKE_FLAG_VALUE "${CMAKE_FLAG_VALUE}") # remove inverted commas and equal sign
	string(FIND ${CMAKE_FLAG_NAME} "COMPILER" COMPILER) # check if the flag regards the compiler
	if(COMPILER GREATER "-1" OR CMAKE_FLAG_NAME STREQUAL "CMAKE_LIBRARY_OUTPUT_DIRECTORY")
		#set(${CMAKE_FLAG_NAME} ${CMAKE_FLAG_VALUE})
	else()
		set(${CMAKE_FLAG_NAME} "${${CMAKE_FLAG_NAME}} ${CMAKE_FLAG_VALUE}")
	endif()
endforeach()
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release")

# Essential include files to build a node addon,
# you should add this line in every CMake.js based project.
include_directories(${CMAKE_JS_INC})
#message("CMAKE_JS_INC ${CMAKE_JS_INC}")

# Check if nodes modules are installed
string(REGEX MATCH "node_modules" CMAKE_JS_INC_NODE_MODULES "${CMAKE_JS_INC}") # split the flag name
if(NOT CMAKE_JS_INC_NODE_MODULES)
	message(FATAL_ERROR "Please first run 'npm install' on the source directory")
endif()

# Declare the source files location
file(GLOB SOURCE_FILES "*.cc" "*.h")

# This line will tell CMake that we're building a shared library
# from the above source files
# named after the project's name
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# This line will give our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential library files to link to a node addon,
# you should add this line in every CMake.js based project.
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
#message("CMAKE_JS_LIB ${CMAKE_JS_LIB}")

target_link_libraries(${PROJECT_NAME} InspectorWidgetProcessorLibrary)

message("[X] ${PROJECT_NAME}")

######################
#        TESTS       #
######################

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()
