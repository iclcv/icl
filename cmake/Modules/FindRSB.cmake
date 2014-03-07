#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindRSB.cmake                            **
#** Module : FindRSB                                                **
#** Authors: Christof Elbrechter, Sergius Gaulik                    **
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

INCLUDE(FindPackageHandleStandardArgs)

# Ask the root directory of rsb.
SET(RSB_ROOT RSB_ROOT CACHE PATH "Root directory of rsb")

# Ask the root directory of rsc.
SET(RSC_ROOT RSC_ROOT CACHE PATH "Root directory of rsc")

IF(NOT RSB_ROOT)
  IF(RSB_DIR)
    SET(RSB_ROOT ${RSB_DIR})
  ELSE()
    IF(NOT WIN32)
      SET(RSB_ROOT /usr)
    ENDIF(NOT WIN32)
  ENDIF()
ENDIF()

FIND_PATH(RSB_INCLUDE_DIR 
  NAMES rsb/Factory.h rsb/Handler.h rsb/converter/Repository.h rsb/converter/ProtocolBufferConvert.h
  PATHS ${RSB_ROOT}/include ${RSB_ROOT}/include/rsb ${RSB_ROOT}/include/rsb0.9 ${RSB_ROOT}/include/rsb0.10 ${RSB_ROOT}/include/rsb0.11
  DOC "The path to RSB header files"
  NO_DEFAULT_PATH)

FIND_PATH(RSC_INCLUDE_DIR 
  NAMES rsc/logging/Logger.h
  PATHS ${RSC_ROOT}/include ${RSC_ROOT}/include/rsc ${RSC_ROOT}/include/rsc0.9
        ${RSC_ROOT}/include/rsc0.10 ${RSC_ROOT}/include/rsc0.11
        ${RSB_ROOT}/include ${RSB_ROOT}/include/rsc
  DOC "The path to RSC header files"
  NO_DEFAULT_PATH)

IF(RSB_INCLUDE_DIR)
  MESSAGE(STATUS "Found RSB include dir: ${RSB_INCLUDE_DIR}")
ENDIF()

IF(RSC_INCLUDE_DIR)
  MESSAGE(STATUS "Found RSC include dir: ${RSC_INCLUDE_DIR}")
ENDIF()

# old library layout
FIND_LIBRARY(RSB_LIBRARY  
  NAMES rsbcore
  PATHS ${RSB_ROOT}
  PATH_SUFFIXES lib
  NO_DEFAULT_PATH)

get_filename_component(FILE_NAME ${RSB_LIBRARY} NAME_WE)
IF(FILE_NAME STREQUAL librsbcore OR FILE_NAME STREQUAL rsbcore)
  MESSAGE(STATUS "resulting rsb-lib(old library layout): ${RSB_LIBRARY}")
ELSE()
  FIND_LIBRARY(RSB_LIBRARY
    NAMES rsb
    PATHS ${RSB_ROOT}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
  MESSAGE(STATUS "resulting rsb-lib: ${RSB_LIBRARY}")

  IF(RSB_LIBRARY)
    # new library layout, we need to link against libspread as well
    FIND_LIBRARY(SPREAD_LIBRARY  
      NAMES libspread spread
      PATHS ${RSB_ROOT}/../spread /usr ${RSB_ROOT}
      PATH_SUFFIXES lib bin
      NO_DEFAULT_PATH)
    MESSAGE(STATUS "resulting spread: ${SPREAD_LIBRARY}")
    IF(RSB_LIBRARY AND NOT SPREAD_LIBRARY)
      IF(WIN32)
        MESSAGE(FATAL_ERROR "Not Found: libspread.lib in ${RSB_ROOT}/../spread/lib and ${RSB_ROOT}/../spread/bin (the new rsb-library layout that uses rsb.lib needs explicit linkage against libspread.lib)")
      ELSE(WIN32)
        MESSAGE(FATAL_ERROR "Not Found: libspread.so in ${RSB_ROOT}/lib and /usr/lib (the new rsb-library layout that uses librsb.so needs explicit linkage against libspread.so)")
      ENDIF(WIN32)
    ENDIF()
    
    IF(WIN32)
      # Windows needs some boost include files and libraries
      FIND_PATH(BOOST_INCLUDE_DIR 
        NAMES boost/shared_ptr.hpp
        PATHS ${RSB_ROOT}/../boost/include/boost-1_54 /usr
        DOC "The path to boost header files"
        NO_DEFAULT_PATH)

      IF(BOOST_INCLUDE_DIR)
        MESSAGE(STATUS "Found boost include dir: ${BOOST_INCLUDE_DIR}")
      ENDIF()
      
      FOREACH(L filesystem regex chrono date_time system thread)
        FIND_LIBRARY(BOOST_${L}_LIBRARY
          NAMES "libboost_${L}-vc100-mt-gd-1_54" "libboost_${L}-vc110-mt-gd-1_54"
          PATHS ${RSB_ROOT}/../boost /usr
          PATH_SUFFIXES lib
          NO_DEFAULT_PATH)
        
        IF(NOT ${BOOST_${L}_LIBRARY} STREQUAL "BOOST_${L}_LIBRARY-NOTFOUND")
          LIST(APPEND BOOST_LIBRARIES ${BOOST_${L}_LIBRARY})
        ELSE()
          MESSAGE(STATUS "Not Found: libboost_${L}-vc100-mt-gd-1_54.lib in ${RSB_ROOT}/../boost/lib")
        ENDIF()
      ENDFOREACH()
    ENDIF(WIN32)
  ENDIF(RSB_LIBRARY)
ENDIF()

FIND_LIBRARY(RSC_LIBRARY  
  NAMES rsc rsc0.9 rsc0.10 rsc0.11
  PATHS ${RSC_ROOT} ${RSB_ROOT}
  PATH_SUFFIXES lib
  NO_DEFAULT_PATH)
MESSAGE(STATUS "resulting rsc-lib: ${RSC_LIBRARY}")

IF(RSB_LIBRARY AND RSC_LIBRARY)
  SET(RSB_LIBRARIES ${RSB_LIBRARY} ${RSC_LIBRARY} ${SPREAD_LIBRARY})
ENDIF()

# define mandatory arguments
IF(WIN32)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RSB REQUIRED_VARS 
                                  RSB_LIBRARIES
                                  BOOST_LIBRARIES
                                  RSB_INCLUDE_DIR
                                  RSC_INCLUDE_DIR
                                  BOOST_INCLUDE_DIR)
ELSE(WIN32)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(RSB REQUIRED_VARS 
                                    RSB_LIBRARIES
                                    RSB_INCLUDE_DIR
                                    RSC_INCLUDE_DIR)
ENDIF(WIN32)

IF(RSB_FOUND)
  INCLUDE_DIRECTORIES(${RSB_INCLUDE_DIR} ${RSC_INCLUDE_DIR} ${BOOST_INCLUDE_DIR})
ENDIF()

#MARK_AS_ADVANCED(RSB_INCLUDE_DIR)
#MARK_AS_ADVANCED(RSC_INCLUDE_DIR)
