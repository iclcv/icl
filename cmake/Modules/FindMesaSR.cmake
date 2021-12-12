#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindMesaSR.cmake                         **
#** Module : FindMesaSR                                             **
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

# Search MESASR_ROOT first if it is set.
IF(MESASR_ROOT)
  SET(_MESASR_SEARCH_ROOT PATHS ${MESASR_ROOT} ${MESASR_ROOT}/lib ${MESASR_ROOT}/ipp NO_DEFAULT_PATH)
  LIST(APPEND _MESASR_SEARCHES _MESASR_SEARCH_ROOT)
ENDIF()

# Normal search.
SET(_MESASR_SEARCH_NORMAL
     PATHS "/usr"
           "/opt/MESASR"
   )

LIST(APPEND _MESASR_SEARCHES _MESASR_SEARCH_NORMAL)
LIST(APPEND _MESASR_LIBRARIES mesasr)

# Try each search configuration
FOREACH(_PATH ${_MESASR_SEARCHES})
  FIND_PATH(MESASR_INCLUDE_DIR
            NAMES libMesaSR.h
	    PATHS ${${_PATH}}
	    PATH_SUFFIXES "/include"
	    DOC "The path to MESA Imaging library"
	    NO_DEFAULT_PATH)

    FOREACH(_lib ${_MESASR_LIBRARIES})
      FIND_LIBRARY(${_lib}_LIBRARY
               NAMES ${_lib}
	       PATHS ${${_PATH}}
	       PATH_SUFFIXES lib
	       NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()

# Handle the QUIETLY and REQUIRED arguments and set MESASR_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MESASR REQUIRED_VARS
				  mesasr_LIBRARY
				  MESASR_INCLUDE_DIR)

IF(MESASR_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  FOREACH(_lib ${_MESASR_LIBRARIES})
    LIST(APPEND _MESASR_LIBRARIES_LIST ${${_lib}_LIBRARY})
  ENDFOREACH()

  SET(MESASR_INCLUDE_DIRS ${MESASR_INCLUDE_DIR})
  SET(MESASR_LIBRARIES ${_MESASR_LIBRARIES_LIST})
ENDIF()

MARK_AS_ADVANCED(MESASR_INCLUDE_DIR)
