#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindLibFreenect.cmake                    **
#** Module : FindLibFreenect                                        **
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

# Search LIBFREENECT_ROOT first if it is set.
IF(LIBFREENECT_ROOT)
  SET(_LIBFREENECT_SEARCH_ROOT PATHS ${LIBFREENECT_ROOT} ${LIBFREENECT_ROOT}/lib ${LIBFREENECT_ROOT}/ipp NO_DEFAULT_PATH)
  LIST(APPEND _LIBFREENECT_SEARCHES ${_LIBFREENECT_SEARCH_ROOT})
ENDIF()

# Normal search.
SET(_LIBFREENECT_SEARCH_NORMAL PATHS "/usr")

LIST(APPEND _LIBFREENECT_SEARCHES ${_LIBFREENECT_SEARCH_NORMAL})
LIST(APPEND _LIBFREENECT_LIBRARIES freenect freenect_sync)

# Try each search configuration
FOREACH(_PATH ${_LIBFREENECT_SEARCHES})
  SET(_PATH_EXTEND "${_PATH}/include")
  
  FIND_PATH(LIBFREENECT_INCLUDE_DIR 
            NAMES libfreenect.h        
	    PATHS ${_PATH_EXTEND}
	    PATH_SUFFIXES "/libfreenect"
	    DOC "The path to LIBFREENECT header files"
	    NO_DEFAULT_PATH)
  
    FOREACH(_lib ${_LIBFREENECT_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY  
               NAMES ${_lib}
	       PATHS ${_PATH}
	       PATH_SUFFIXES "/lib" "/lib/i386-linux-gnu" "/lib/x86_64-linux-gnu"
	       NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()
	   
# Handle the QUIETLY and REQUIRED arguments and set LIBFREENECT_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBFREENECT REQUIRED_VARS 
				  freenect_LIBRARY
				  freenect_sync_LIBRARY
				  LIBFREENECT_INCLUDE_DIR)

IF(LIBFREENECT_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  FOREACH(_lib ${_LIBFREENECT_LIBRARIES})
    LIST(APPEND _LIBFREENECT_LIBRARIES_LIST ${${_lib}_LIBRARY})
  ENDFOREACH()

  LIST(REMOVE_DUPLICATES _LIBFREENECT_LIBRARIES_LIST)
  SET(LIBFREENECT_INCLUDE_DIRS ${LIBFREENECT_INCLUDE_DIR})
  SET(LIBFREENECT_LIBRARIES ${_LIBFREENECT_LIBRARIES_LIST})
ENDIF()

MARK_AS_ADVANCED(LIBFREENECT_INCLUDE_DIR)
