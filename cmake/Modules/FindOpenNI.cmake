#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindOpenNI.cmake                         **
#** Module : FindOpenNI                                             **
#** Authors: Michael Goetting                                       **
#**                                                                 **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

# Search OPENNI_ROOT first if it is set.
IF(OPENNI_ROOT)
  SET(_OPENNI_SEARCH_ROOT PATHS ${OPENNI_ROOT} ${OPENNI_ROOT}/lib NO_DEFAULT_PATH)
  LIST(APPEND _OPENNI_SEARCHES _OPENNI_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_OPENNI_SEARCH_NORMAL
     PATHS "/usr"
   )
LIST(APPEND _OPENNI_SEARCHES _OPENNI_SEARCH_NORMAL)
LIST(APPEND _OPENNI_LIBRARIES OpenNI)

# Set search path suffix
SET (_LIB_SEARCH_PATH_SUFFIXES "/lib")

# Try each search configuration
FOREACH(_PATH ${_OPENNI_SEARCHES})
  FIND_PATH(OPENNI_INCLUDE_DIR 
            NAMES XnOpenNI.h
	    PATHS ${${_PATH}}
	    PATH_SUFFIXES "include/ni" 	  
	    DOC "The path to OPENNI header files"
	    NO_DEFAULT_PATH)
  
    FOREACH(_lib ${_OPENNI_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY  
               NAMES ${_lib}
	       PATHS ${${_PATH}}
	       PATH_SUFFIXES ${_LIB_SEARCH_PATH_SUFFIXES}
	       NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()
	   
# Handle the QUIETLY and REQUIRED arguments and set OPENNI_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENNI REQUIRED_VARS 
				  OpenNI_LIBRARY
				  OPENNI_INCLUDE_DIR)

IF(OPENNI_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  FOREACH(_lib ${_OPENNI_LIBRARIES})
    LIST(APPEND _OPENNI_LIBRARIES_LIST ${${_lib}_LIBRARY})
  ENDFOREACH()

  LIST(REMOVE_DUPLICATES _OPENNI_LIBRARIES_LIST)
  SET(OPENNI_INCLUDE_DIRS ${OPENNI_INCLUDE_DIR})
  SET(OPENNI_LIBRARIES ${_OPENNI_LIBRARIES_LIST})
ENDIF()

MARK_AS_ADVANCED(OPENNI_INCLUDE_DIR)
