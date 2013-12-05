#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindEigen3.cmake                         **
#** Module : FindEigen3                                             **
#** Authors: Michael Goetting                                       **
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

# Search EIGEN3_ROOT first if it is set.
IF(EIGEN3_ROOT)
  SET(_EIGEN3_SEARCH_ROOT PATHS ${EIGEN3_ROOT} ${EIGEN3_ROOT}/lib ${EIGEN3_ROOT}/ipp NO_DEFAULT_PATH)
  LIST(APPEND _EIGEN3_SEARCHES _EIGEN3_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_EIGEN3_SEARCH_NORMAL
     PATHS "/usr"
   )
LIST(APPEND _EIGEN3_SEARCHES _EIGEN3_SEARCH_NORMAL)

# Try each search configuration
FOREACH(_PATH ${_EIGEN3_SEARCHES})
  FIND_PATH(EIGEN3_INCLUDE_DIR 
            NAMES Eigen/Eigen
	    PATHS ${${_PATH}}
	    PATH_SUFFIXES "include/eigen3" 	  
	    DOC "The path to EIGEN3 header files"
	    NO_DEFAULT_PATH)  
ENDFOREACH()
	   
# Handle the QUIETLY and REQUIRED arguments and set EIGEN3_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EIGEN3 REQUIRED_VARS 
				  EIGEN3_INCLUDE_DIR)

IF(EIGEN3_FOUND)
  SET(EIGEN3_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
ENDIF()

MARK_AS_ADVANCED(EIGEN3_INCLUDE_DIR)
