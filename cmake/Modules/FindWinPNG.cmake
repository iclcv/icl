#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindWinPNG.cmake                         **
#** Module : FindWinPNG                                             **
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

# Ask the root directory of png.
# http://gnuwin32.sourceforge.net/packages/jpeg.htm
SET(PNG_ROOT PNG_ROOT CACHE PATH "Root directory of png")

# Search PNG_ROOT first.
IF(PNG_ROOT)
  SET(_PNG_SEARCH_ROOT PATHS ${PNG_ROOT} NO_DEFAULT_PATH)
  LIST(APPEND _PNG_SEARCHES _PNG_SEARCH_ROOT)
ENDIF()

# Try each search configuration
FIND_PATH(PNG_INCLUDE_DIRS 
  NAMES png.h
  PATHS ${${_PNG_SEARCHES}}
  PATH_SUFFIXES "" 	  
  DOC "The path to PNG header files"
  NO_DEFAULT_PATH)

FIND_LIBRARY(PNG_LIBRARIES  
  NAMES PNG
  PATHS  ${${_PNG_SEARCHES}}
  PATH_SUFFIXES "/projects/vstudio/Release Library"
  NO_DEFAULT_PATH)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PNG REQUIRED_VARS 
				  PNG_LIBRARIES
				  PNG_INCLUDE_DIRS)

MARK_AS_ADVANCED(PNG_INCLUDE_DIRS)
