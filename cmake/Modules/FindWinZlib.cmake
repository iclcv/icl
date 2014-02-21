#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindWinZlib.cmake                        **
#** Module : FindWinZlib                                            **
#** Authors: Michael Goetting, Sergius Gaulik                       **
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

# ---------------------------------------------------------------------
# Start main part here
# ---------------------------------------------------------------------

# Ask the root directory of zlib.
SET(ZLIB_ROOT ZLIB_ROOT CACHE PATH "Root directory of zlib")

# Search ZLIB_ROOT first.
IF(ZLIB_ROOT)
  SET(_ZLIB_SEARCH_ROOT PATHS ${ZLIB_ROOT} NO_DEFAULT_PATH)
  LIST(APPEND _ZLIB_SEARCHES _ZLIB_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_ZLIB_SEARCH_NORMAL
     PATHS "${CMAKE_INSTALL_PREFIX}/../zlib"
   )

LIST(APPEND _ZLIB_SEARCHES _ZLIB_SEARCH_NORMAL)

# Try each search configuration
FOREACH(_PATH ${_ZLIB_SEARCHES})
  FIND_PATH(ZLIB_INCLUDE_DIRS 
    NAMES zlib.h
    PATHS ${${_PATH}}
    PATH_SUFFIXES "include" 	  
    DOC "The path to ZLIB header files"
    NO_DEFAULT_PATH)

  FIND_LIBRARY(ZLIB_LIBRARIES
    NAMES zlib.lib zdll.lib
    PATHS  ${${_PATH}}
    PATH_SUFFIXES "lib"
    NO_DEFAULT_PATH)
ENDFOREACH()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZLIB REQUIRED_VARS 
				  ZLIB_LIBRARIES
				  ZLIB_INCLUDE_DIRS)
