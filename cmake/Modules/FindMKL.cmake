#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindMKL.cmake                            **
#** Module : FindMKL                                                **
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

# mkl_types.h;mkl_cblas.h" "mkl_intel_lp64;mkl_intel_thread;mkl_core
INCLUDE(FindPackageHandleStandardArgs)

# Search MKL_ROOT first if it is set.
IF(MKL_ROOT)
  SET(_MKL_SEARCH_ROOT PATHS ${MKL_ROOT} ${MKL_ROOT}/mkl NO_DEFAULT_PATH)
  LIST(APPEND _MKL_SEARCHES _MKL_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_MKL_SEARCH_NORMAL
     PATHS "/opt/MKL"
     PATHS "/opt/MKL/lib"
     PATHS "/opt/MKL/mkl"
   )

# Set search paths and libraries
IF (ICL_64BIT)
  LIST(APPEND _MKL_LIBRARIES mkl_intel_lp64)
  set (_LIB_SEARCH_PATH_SUFFIXES "/lib/intel64")
ELSE()
  LIST(APPEND _MKL_LIBRARIES mkl_intel)  
  set (_LIB_SEARCH_PATH_SUFFIXES "/lib/ia32")
ENDIF()

LIST(APPEND _MKL_SEARCHES _MKL_SEARCH_NORMAL)
LIST(APPEND _MKL_LIBRARIES mkl_intel_thread mkl_core iomp5)

# Try each search configuration
FOREACH(_PATH ${_MKL_SEARCHES})
  FIND_PATH(MKL_INCLUDE_DIR 
            NAMES mkl_types.h        
	    PATHS ${${_PATH}}
	    PATH_SUFFIXES "include" 	  
	    DOC "The path to Intel(R) MKL header files"
	    NO_DEFAULT_PATH)
  
    FOREACH(_lib ${_MKL_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY  
               NAMES ${_lib}
	       PATHS ${${_PATH}}
	       PATH_SUFFIXES ${_LIB_SEARCH_PATH_SUFFIXES}
	       NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()

# Handle the QUIETLY and REQUIRED arguments and set MKL_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MKL REQUIRED_VARS 
				  mkl_intel_thread_LIBRARY 
				  mkl_core_LIBRARY 
				  iomp5_LIBRARY
				  MKL_INCLUDE_DIR)

IF(MKL_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  SET(_MKL_LIBRARIES_LIST)
  FOREACH(_lib ${_MKL_LIBRARIES})
    LIST(APPEND _MKL_LIBRARIES_LIST "${${_lib}_LIBRARY}")
  ENDFOREACH()

  SET(MKL_INCLUDE_DIRS ${MKL_INCLUDE_DIR})
  SET(MKL_LIBRARIES ${_MKL_LIBRARIES_LIST})
ENDIF()

MARK_AS_ADVANCED(MKL_INCLUDE_DIR)
