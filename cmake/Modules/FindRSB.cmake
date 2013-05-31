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
#** Authors: Christof Elbrechter                                    **
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

IF(NOT RSB_ROOT)
  IF(RSB_DIR)
    SET(RSB_ROOT ${RSB_DIR})
  ELSE()
    SET(RSB_ROOT /usr)
  ENDIF()
ENDIF()

FIND_PATH(RSB_INCLUDE_DIR 
  NAMES rsb/Factory.h rsb/Handler.h rsb/converter/Repository.h rsb/converter/ProtocolBufferConvert.h
  PATHS ${RSB_ROOT}/include ${RSB_ROOT}/include/rsb
  DOC "The path to RSB header files"
  NO_DEFAULT_PATH)

FIND_PATH(RSC_INCLUDE_DIR 
  NAMES rsc/logging/Logger.h
  PATHS ${RSB_ROOT}/include ${RSB_ROOT}/include/rsc
  DOC "The path to RSC header files"
  NO_DEFAULT_PATH)

IF(RSB_INCLUDE_DIR)
  MESSAGE(STATUS "Found RSB include dir: ${RSB_INCLUDE_DIR}")
ENDIF()

IF(RSC_INCLUDE_DIR)
  MESSAGE(STATUS "Found RSC include dir: ${RSC_INCLUDE_DIR}")
ENDIF()

FIND_LIBRARY(RSB_LIBRARY  
  NAMES rsbcore
  PATHS ${RSB_ROOT}
  PATH_SUFFIXES lib
  NO_DEFAULT_PATH)

IF(NOT RSB_LIBRARY)
  # new library layout, we need to link against libspread as well
  FIND_LIBRARY(RSB_LIBRARY  
    NAMES rsb
    PATHS ${RSB_ROOT}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
  IF(RSB_LIBRARY)
    FIND_LIBRARY(SPREAD_LIBRARY  
      NAMES spread
      PATHS ${RSB_ROOT} /usr
      PATH_SUFFIXES lib
      NO_DEFAULT_PATH)
    IF(NOT SPREAD_LIBRARY)
      MESSAGE(FATAL_ERROR "Not Found: libspread.so in ${RSB_ROOT}/lib and /usr/lib (the new rsb-library layout that uses librsb.so needs explicit linkage against libspread.so)")
    ENDIF()
  ENDIF()
ENDIF()

FIND_LIBRARY(RSC_LIBRARY  
  NAMES rsc
  PATHS ${RSB_ROOT}
  PATH_SUFFIXES lib
  NO_DEFAULT_PATH)

IF(RSB_LIBRARY AND RSC_LIBRARY)
  SET(RSB_LIBRARIES ${RSB_LIBRARY} ${RSC_LIBRARY} ${SPREAD_LIBRARY})
ENDIF()

# define mandatory arguments
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RSB REQUIRED_VARS 
				  RSB_LIBRARIES
				  RSB_INCLUDE_DIR
                                  RSB_INCLUDE_DIR)
IF(RSB_FOUND)
  INCLUDE_DIRECTORIES(${RSB_INCLUDE_DIR} ${RSC_INCLUDE_DIR})
ENDIF()

MARK_AS_ADVANCED(RSB_INCLUDE_DIR)
MARK_AS_ADVANCED(RSC_INCLUDE_DIR)
