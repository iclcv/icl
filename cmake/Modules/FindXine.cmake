#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindXine.cmake                           **
#** Module : FindXine                                               **
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

# Search XINE_ROOT first if it is set.
IF(XINE_ROOT)
  SET(_XINE_SEARCH_ROOT PATHS ${XINE_ROOT} ${XINE_ROOT}/lib ${XINE_ROOT}/ipp NO_DEFAULT_PATH)
  LIST(APPEND _XINE_SEARCHES _XINE_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_XINE_SEARCH_NORMAL
     PATHS "/usr"
   )
LIST(APPEND _XINE_SEARCHES _XINE_SEARCH_NORMAL)
LIST(APPEND _XINE_LIBRARIES xine)

# Try each search configuration
FOREACH(_PATH ${_XINE_SEARCHES})
  FIND_PATH(XINE_INCLUDE_DIR 
            NAMES xine.h        
	    PATHS ${${_PATH}}
	    PATH_SUFFIXES "/include" 	  
	    DOC "The path to XINE header files"
	    NO_DEFAULT_PATH)
  
    FOREACH(_lib ${_XINE_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY  
               NAMES ${_lib}
	       PATHS ${${_PATH}}
	       PATH_SUFFIXES "/lib"
	       NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()
	   
# Handle the QUIETLY and REQUIRED arguments and set XINE_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XINE REQUIRED_VARS 
				  xine_LIBRARY
				  XINE_INCLUDE_DIR)

IF(XINE_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  FOREACH(_lib ${_XINE_LIBRARIES})
    LIST(APPEND _XINE_LIBRARIES_LIST ${${_lib}_LIBRARY})
  ENDFOREACH()

  LIST(REMOVE_DUPLICATES _XINE_LIBRARIES_LIST)
  SET(XINE_INCLUDE_DIRS ${XINE_INCLUDE_DIR})
  SET(XINE_LIBRARIES ${_XINE_LIBRARIES_LIST})
ENDIF()

MARK_AS_ADVANCED(XINE_INCLUDE_DIR)
