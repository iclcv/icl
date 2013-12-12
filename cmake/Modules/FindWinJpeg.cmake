#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindWinJpeg.cmake                        **
#** Module : FindWinJpeg                                            **
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

# Ask the root directory of jpeg.
# http://gnuwin32.sourceforge.net/packages/jpeg.htm
SET(JPEG_ROOT JPEG_ROOT CACHE PATH "Root directory of jpeg")

# Search JPEG_ROOT first.
IF(JPEG_ROOT)
  SET(_JPEG_SEARCH_ROOT PATHS ${JPEG_ROOT} NO_DEFAULT_PATH)
  LIST(APPEND _JPEG_SEARCHES _JPEG_SEARCH_ROOT)
ENDIF()

# Try each search configuration
FIND_PATH(JPEG_INCLUDE_DIR 
  NAMES jpeglib.h
  PATHS ${${_JPEG_SEARCHES}}
  PATH_SUFFIXES "include" 	  
  DOC "The path to JPEG header files"
  NO_DEFAULT_PATH)

FIND_LIBRARY(JPEG_LIBRARIES  
  NAMES jpeg.lib
  PATHS  ${${_JPEG_SEARCHES}}
  PATH_SUFFIXES "/lib"
  NO_DEFAULT_PATH)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(JPEG REQUIRED_VARS 
				  JPEG_LIBRARIES
				  JPEG_INCLUDE_DIR)

MARK_AS_ADVANCED(JPEG_INCLUDE_DIR)
