#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindPylon.cmake                          **
#** Module : FindPylon                                              **
#** Authors: Michael Goetting, Viktor Richter                       **
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

IF(WIN32)
  # Get environment variables
  IF(NOT PYLON_ROOT)
    SET(PYLON_ROOT $ENV{PYLON_ROOT} CACHE PATH "Root directory of Pylon")
  ENDIF()
  IF(NOT PYLON_GENICAM_ROOT)
    SET(PYLON_GENICAM_ROOT "$ENV{PYLON_GENICAM_ROOT}" CACHE PATH "Root directory of GenICam")
  ENDIF()
  SET(GENICAM_VER $ENV{PYLON_GENICAM_VERSION})

  # TODO: use _GenICamVersion.h for GenICam
  SET(PYLON_SUFFIX _MD_VC100)

  SET(_PYLON_SEARCHES ${PYLON_ROOT})
  LIST(APPEND _PYLON_SEARCHES ${PYLON_GENICAM_ROOT})
  SET(_PYLON_HEADERS PylonBase.h)
  LIST(APPEND _PYLON_HEADERS GenICamVersion.h)
  SET(_PYLON_LIBRARIES PylonBase${PYLON_SUFFIX})
  LIST(APPEND _PYLON_LIBRARIES
              PylonUtility${PYLON_SUFFIX}
              PylonGigE${PYLON_SUFFIX}_TL
              PylonBootstrapper
              GenApi${PYLON_SUFFIX}_${GENICAM_VER}
              GCBase${PYLON_SUFFIX}_${GENICAM_VER}
              MathParser${PYLON_SUFFIX}_${GENICAM_VER}
              log4cpp${PYLON_SUFFIX}_${GENICAM_VER}
              Log${PYLON_SUFFIX}_${GENICAM_VER})
ELSE(WIN32)
  # Search PYLON_ROOT first if it is set.
  IF(PYLON_ROOT)
    SET(_PYLON_SEARCH_ROOT PATHS ${PYLON_ROOT} ${PYLON_ROOT}/lib ${PYLON_ROOT}/ipp NO_DEFAULT_PATH)
    LIST(APPEND _PYLON_SEARCHES ${_PYLON_SEARCH_ROOT})
  ENDIF()

  # Normal search.
  SET(_PYLON_SEARCH_NORMAL PATHS "/usr" "/opt")

  LIST(APPEND _PYLON_SEARCHES ${_PYLON_SEARCH_NORMAL})
  LIST(APPEND _PYLON_HEADERS PylonBase.h GenICamVersion.h)
  LIST(APPEND _PYLON_LIBRARIES
              pylonbase
              pylonutility
              pylongigesupp
              GenApi_gcc40_v2_1
              GCBase_gcc40_v2_1
              xerces-c
              MathParser_gcc40_v2_1
              log4cpp_gcc40_v2_1
              Log_gcc40_v2_1)
ENDIF(WIN32)


# Set search path suffix
LIST(APPEND _HEADER_SEARCH_PATH_SUFFIXES "/include/pylon"
            "/genicam/library/CPP/include"
            "/library/CPP/include"
)

IF (ICL_64BIT)
    set (_LIB_SEARCH_PATH_SUFFIXES "/lib64")
    LIST(APPEND _LIB_SEARCH_PATH_SUFFIXES "/genicam/bin/Linux64_x64")
  LIST(APPEND _LIB_SEARCH_PATH_SUFFIXES "/lib/x64")
  LIST(APPEND _LIB_SEARCH_PATH_SUFFIXES "/library/cpp/lib/win32_x64")
ELSE()
  set (_LIB_SEARCH_PATH_SUFFIXES "/lib")
  LIST(APPEND _LIB_SEARCH_PATH_SUFFIXES "/genicam/bin/Linux32_i86")
  LIST(APPEND _LIB_SEARCH_PATH_SUFFIXES "/lib/Win32")
  LIST(APPEND _LIB_SEARCH_PATH_SUFFIXES "/library/cpp/lib/win32_i86")
ENDIF()


# Try each search configuration
FOREACH(_PATH ${_PYLON_SEARCHES})

  FOREACH(_header ${_PYLON_HEADERS})
    FIND_PATH(${_header}_PYLON_INCLUDE_DIR
              NAMES ${_header}
              PATHS ${_PATH}
              PATH_SUFFIXES ${_HEADER_SEARCH_PATH_SUFFIXES}
              DOC "The path to PYLON header files"
              NO_DEFAULT_PATH)
  ENDFOREACH()

  FOREACH(_lib ${_PYLON_LIBRARIES})
    FIND_LIBRARY(${_lib}_LIBRARY
                 NAMES ${_lib}
                 PATHS ${_PATH}
                 PATH_SUFFIXES ${_LIB_SEARCH_PATH_SUFFIXES}
                 NO_DEFAULT_PATH)
    ENDFOREACH()
ENDFOREACH()

# Handle the QUIETLY and REQUIRED arguments and set PYLON_FOUND to TRUE if
# all listed variables are TRUE
IF(WIN32)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(PYLON REQUIRED_VARS
                                    PylonBase${PYLON_SUFFIX}_LIBRARY
                                    PylonUtility${PYLON_SUFFIX}_LIBRARY
                                    PylonGigE${PYLON_SUFFIX}_TL_LIBRARY
                                    PylonBootstrapper_LIBRARY
                                    GenApi${PYLON_SUFFIX}_${GENICAM_VER}_LIBRARY
                                    GCBase${PYLON_SUFFIX}_${GENICAM_VER}_LIBRARY
                                    MathParser${PYLON_SUFFIX}_${GENICAM_VER}_LIBRARY
                                    log4cpp${PYLON_SUFFIX}_${GENICAM_VER}_LIBRARY
                                    Log${PYLON_SUFFIX}_${GENICAM_VER}_LIBRARY
                                    PylonBase.h_PYLON_INCLUDE_DIR
                                    GenICamVersion.h_PYLON_INCLUDE_DIR)
ELSE(WIN32)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(PYLON REQUIRED_VARS
                                    pylonbase_LIBRARY
                                    pylonutility_LIBRARY
                                    GenApi_gcc40_v2_1_LIBRARY
                                    GCBase_gcc40_v2_1_LIBRARY
                                    pylongigesupp_LIBRARY
                                    xerces-c_LIBRARY
                                    MathParser_gcc40_v2_1_LIBRARY
                                    log4cpp_gcc40_v2_1_LIBRARY
                                    Log_gcc40_v2_1_LIBRARY
                                    PylonBase.h_PYLON_INCLUDE_DIR
                                    GenICamVersion.h_PYLON_INCLUDE_DIR)
ENDIF(WIN32)

IF(PYLON_FOUND)
  # HACK: Until FIND_LIBRARY could handle multiple libraries
  FOREACH(_header ${_PYLON_HEADERS})
    SET(_temp_ ${${_header}_PYLON_INCLUDE_DIR})
    STRING(REGEX REPLACE "/include/pylon" "/include" _temp_ ${_temp_})
    LIST(APPEND _PYLON_HEADERS_LIST ${_temp_})
  ENDFOREACH()

  FOREACH(_lib ${_PYLON_LIBRARIES})
    LIST(APPEND _PYLON_LIBRARIES_LIST ${${_lib}_LIBRARY})
  ENDFOREACH()

  SET(PYLON_INCLUDE_DIRS ${_PYLON_HEADERS_LIST})

  LIST(REMOVE_DUPLICATES _PYLON_LIBRARIES_LIST)
  SET(PYLON_LIBRARIES ${_PYLON_LIBRARIES_LIST})
ENDIF()

MARK_AS_ADVANCED(PYLON_INCLUDE_DIR)
