#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindGLEW.cmake                           **
#** Module : FindGLEW                                               **
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

# Search GLEW_ROOT first if it is set.
IF(GLEW_ROOT)
  SET(_GLEW_SEARCH_ROOT PATHS ${GLEW_ROOT} ${GLEW_ROOT}/lib NO_DEFAULT_PATH)
  LIST(APPEND _GLEW_SEARCHES _GLEW_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_GLEW_SEARCH_NORMAL
     PATHS "/usr"
   )

LIST(APPEND _GLEW_SEARCHES _GLEW_SEARCH_NORMAL)

# Try each search configuration
FIND_PATH(GLEW_INCLUDE_DIRS 
  NAMES glew.h
  PATHS ${${_GLEW_SEARCHES}}
  PATH_SUFFIXES "include/GL" 	  
  DOC "The path to GLEW header files"
  NO_DEFAULT_PATH)

FIND_LIBRARY(GLEW_LIBRARIES  
  NAMES GLEW
  PATHS  ${${_GLEW_SEARCHES}}
  PATH_SUFFIXES "/lib" "/lib/i386-linux-gnu" "/lib/x86_64-linux-gnu"
  NO_DEFAULT_PATH)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW REQUIRED_VARS 
				  GLEW_LIBRARIES
				  GLEW_INCLUDE_DIRS)

MARK_AS_ADVANCED(GLEW_INCLUDE_DIRS)
