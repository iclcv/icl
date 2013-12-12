#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindWinPthreads.cmake                    **
#** Module : FindWinPthreads                                        **
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

# Ask the root directory of pthreads.
SET(PTHREADS_ROOT PTHREADS_ROOT CACHE PATH "Root directory of pthreads")

# Search PTHREADS_ROOT first.
IF(PTHREADS_ROOT)
  SET(_PTHREADS_SEARCH_ROOT PATHS ${PTHREADS_ROOT} NO_DEFAULT_PATH)
  LIST(APPEND _PTHREADS_SEARCHES _PTHREADS_SEARCH_ROOT)
ENDIF()

# Try each search configuration
FIND_PATH(PTHREADS_INCLUDE_DIR 
  NAMES pthread.h
  PATHS ${${_PTHREADS_SEARCHES}}
  PATH_SUFFIXES "include" 	  
  DOC "The path to PTHREADS header files"
  NO_DEFAULT_PATH)

FIND_LIBRARY(PTHREADS_LIBRARIES  
  NAMES pthreadVC2.lib
  PATHS  ${${_PTHREADS_SEARCHES}}
  PATH_SUFFIXES "/lib"
  NO_DEFAULT_PATH)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PTHREADS REQUIRED_VARS 
				  PTHREADS_LIBRARIES
				  PTHREADS_INCLUDE_DIR)

MARK_AS_ADVANCED(PTHREADS_INCLUDE_DIR)
