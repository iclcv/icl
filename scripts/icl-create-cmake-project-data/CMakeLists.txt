#*********************************************************************
#**                                                                 **
#** File   : CMakeLists.txt                                         **
#** Authors: Viktor Richter                                         **
#**                                                                 **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.LGPL **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the license requirements will   **
#** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************

cmake_minimum_required(VERSION 2.8.2)

# this sets the projects name
project(@projectname)

# the projects version number
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)

# some output
message(STATUS "")
message(STATUS "==============================================================")
message(STATUS "Configuring ${PROJECT_NAME}:")

# checking build type, setting to release when not set
IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "No Specific build type specified: using Release")
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

# some compiler flags for the different build settings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# search for icl
include(cmake/FindPkgConfig.cmake)
if(NOT ICL_VERSION)
  SET(ICL_VERSION @current_icl_version)
endif()

PKGCONFIG("ICL-${ICL_VERSION}" HAVE_ICL ICL_INCLUDE_DIRS ICL_DEFINES ICL_LINK_DIRS ICL_LIBS)

IF(HAVE_ICL)
  LIST(APPEND REQUIRED_LIBRARIES_LIST ICL_LIBS})
ELSE()
  message(STATUS "Could not find ICL-${ICL_VERSION}")
  message(STATUS "Please ensure that ICL-${ICL_VERSION}.pc is in your PKG_CONFIG_PATH")
  message(STATUS "You can specify the ICL-Version to use via cmake arguments.")
  message(STATUS "\t the argument -DICL_VERSION=8.0 will change the version to 8.0")
  message(FATAL_ERROR "\nThis project will not build without ICL")
ENDIF()

message(STATUS "Looking for doxygen")
find_program(DOXYGEN_BIN NAMES doxygen)
if(NOT "${DOXYGEN_BIN}" STREQUAL "DOXYGEN_BIN-NOTFOUND")
  message(STATUS "found Doxygen, a documentation can be built with 'make doc'")
  add_custom_target(doc
    COMMAND ${DOXYGEN_BIN} ${PROJECT_BINARY_DIR}/Doxyfile
    COMMENT "Creating html and tex documentation in doc/"
  )
  install(
    DIRECTORY ${PROJECT_BINARY_DIR}/doc/html 
    DESTINATION "share/doc/${PROJECT_NAME}/"
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_EXECUTE OWNER_EXECUTE GROUP_EXECUTE
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE 
 )
else()
  message(STATUS "  doxygen not found, documentation will not be available")
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/Doxyfile.in"
  "${PROJECT_BINARY_DIR}/Doxyfile"
)

# get all header files
#FILE(GLOB HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
# get all sources
#FILE(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
# get all apps
#FILE(GLOB APPS "${PROJECT_SOURCE_DIR}/apps/*.cpp")

# add all headers
#include_directories("${PROJECT_SOURCE_DIR}/src/")
#include_directories(${ICL_INCLUDE_DIRS})
#include_directories(${PCL_INCLUDE_DIRS})

# add icl libs
#link_directories(${ICL_LINK_DIRS})
#link_directories(${PCL_LIBRARY_DIRS})

#add icl defines
#add_definitions(${ICL_DEFINES})
#add_definitions(${PCL_DEFINITIONS})

#create library
#add_library(${PROJECT_NAME} SHARED ${SOURCES})
#target_link_libraries(${PROJECT_NAME}
#  ${ICL_LIBS}
#)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(app)
