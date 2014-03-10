#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindDL.cmake                             **
#** Module : FindDL                                                 **
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

INCLUDE(FindPackageHandleStandardArgs)

# ---------------------------------------------------------------------
# Start main part here
# ---------------------------------------------------------------------

# Search DL_ROOT first if it is set.
IF(DL_ROOT)
  SET(_DL_SEARCH_ROOT PATHS ${DL_ROOT} ${DL_ROOT}/lib NO_DEFAULT_PATH)
  LIST(APPEND _DL_SEARCHES _DL_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_DL_SEARCH_NORMAL
     PATHS "/usr"
   )

LIST(APPEND _DL_SEARCHES _DL_SEARCH_NORMAL)

# Try each search configuration
FIND_PATH(DL_INCLUDE_DIRS 
  NAMES dlfcn.h
  PATHS ${${_DL_SEARCHES}}
  PATH_SUFFIXES "include/" 	  
  DOC "The path to DL header files"
  NO_DEFAULT_PATH)

FIND_LIBRARY(DL_LIBRARIES  
  NAMES dl
  PATHS  ${${_DL_SEARCHES}}
  PATH_SUFFIXES "/lib" "/lib/i386-linux-gnu" "/lib/x86_64-linux-gnu"
  NO_DEFAULT_PATH)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(DL REQUIRED_VARS 
				  DL_LIBRARIES
				  DL_INCLUDE_DIRS)

MARK_AS_ADVANCED(DL_INCLUDE_DIRS)
