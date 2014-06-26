#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindIPP.cmake                            **
#** Module : FindIPP                                                **
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

# ---- Detect IPP version ----
function(get_ipp_version _INCLUDE_DIR)
  set(_VERSION_STR)
  set(_MAJOR)
  set(_MINOR)
  set(_BUILD)

  # read IPP version info from file
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR1 REGEX "IPP_VERSION_MAJOR")
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR2 REGEX "IPP_VERSION_MINOR")
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR3 REGEX "IPP_VERSION_BUILD")
  file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR4 REGEX "IPP_VERSION_STR")
  
  if (NOT STR3)
    file(STRINGS ${_INCLUDE_DIR}/ippversion.h STR3 REGEX "IPP_VERSION_UPDATE")
  endif(NOT STR3)
  
  # extract info and assign to variables
  string(REGEX MATCHALL "[0-9]+" _MAJOR ${STR1})
  string(REGEX MATCHALL "[0-9]+" _MINOR ${STR2})
  string(REGEX MATCHALL "[0-9]+" _BUILD ${STR3})
  string(REGEX MATCHALL "[0-9]+[.]+[0-9]+[^\"]+|[0-9]+[.]+[0-9]+" _VERSION_STR ${STR4})
  
  # export info to parent scope
  set(IPP_VERSION_STR   ${_VERSION_STR} PARENT_SCOPE)
  set(IPP_VERSION_MAJOR ${_MAJOR}       PARENT_SCOPE)
  set(IPP_VERSION_MINOR ${_MINOR}       PARENT_SCOPE)
  set(IPP_VERSION_BUILD ${_BUILD}       PARENT_SCOPE)
  
  MESSAGE(STATUS "           IPP Version: ${_MAJOR}.${_MINOR}.${_BUILD} [${_VERSION_STR}]")
  
  return()
endfunction()


# ---------------------------------------------------------------------
# Start main part here
# ---------------------------------------------------------------------

IF(ICL_64BIT)
  SET(_IPP_LIB_SUF "ipp/lib/intel64")
ELSE(ICL_64BIT)
  SET(_IPP_LIB_SUF "ipp/lib/ia32")
ENDIF(ICL_64BIT)

ICL_FIND_PACKAGE(NAME IPP
                 HEADERS "ipp.h"
                 LIBS "ippcore;ippi;ipps;ippcv;ippm;ippcc;iomp5"
                 PATHS "/opt/IPP"
                 HEADER_PATH_SUFFIXES "ipp/include"
                 LIB_PATH_SUFFIXES "${_IPP_LIB_SUF}")

IF(IPP_FOUND)
  IF(EXISTS "${IPP_INCLUDE_DIRS}/ippversion.h")
    get_ipp_version(${IPP_INCLUDE_DIRS})
  ENDIF()
ENDIF()
