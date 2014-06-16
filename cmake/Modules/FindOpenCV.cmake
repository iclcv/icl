#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindOpenCV.cmake                         **
#** Module : FindOpenCV                                             **
#** Authors: Michael Goetting, Christof Elbrechter                  **
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

# Ask the root directory of OpenCV.
SET(OPENCV_ROOT OPENCV_ROOT CACHE PATH "Root directory of OpenCV")

IF(EXISTS "${OPENCV_ROOT}")
  SET(OpenCV_DIR ${OPENCV_ROOT})
ENDIF()

IF(EXISTS "${OpenCV_DIR}/OpenCVConfig.cmake")  
  # Include the standard CMake script
  INCLUDE("${OpenCV_DIR}/OpenCVConfig.cmake")
  
  # somehow OpenCV writes two times the path to the libraries in the OpenCV_LIB_DIR variable
  LIST(REMOVE_DUPLICATES OpenCV_LIB_DIR)

  # Search for a specific version
  SET(CVLIB_SUFFIX "${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}")

  SET(OpenCV_LIBRARIES "")

  IF(WIN32)
    FOREACH(L ${OpenCV_LIBS})
      LIST(APPEND OpenCV_LIBRARIES "${OpenCV_LIB_DIR}/${L}${CVLIB_SUFFIX}.lib")
    ENDFOREACH()
  ELSE(WIN32)
    FOREACH(L ${OpenCV_LIBS})
      LIST(APPEND OpenCV_LIBRARIES "${OpenCV_LIB_DIR}/lib${L}.so")
    ENDFOREACH()
  ENDIF(WIN32)

  SET(OpenCV_OLD_LIBS_NOT_FOUND "TRUE")
ELSE()
  IF(NOT WIN32)
  # find headers
  FIND_PATH(OpenCV_INCLUDE_DIRS "cv.h" "cxcore.h" "highgui.h"
            PATHS "${OpenCV_DIR}"
            PATH_SUFFIXES "include" "include/opencv")
  
  # Initiate the variable before the loop
  SET(GLOBAL OpenCV_LIBRARIES "")

  IF(EXISTS ${OpenCV_DIR})
    SET(OpenCV_LIB_DIR "${OpenCV_DIR}")
  ELSE()
    SET(OpenCV_LIB_DIR "/usr")
  ENDIF()

  # strategy: search for old style libraries first
  # only if these were not found: search for new style
  # libraries 

  SET(OpenCV_OLD_LIBS_NOT_FOUND "FALSE")
  SET(OpenCV_NEW_LIBS_NOT_FOUND "FALSE")

  # find old style libraries
  FOREACH(L cxcore cv ml highgui cvaux)
    MESSAGE(STATUS "checking for ${L} in ${OpenCV_LIB_DIR}")
    FIND_LIBRARY(OpenCV_${L}_LIBRARY
      NAMES "lib${L}" "${L}"
      PATHS "${OpenCV_LIB_DIR}"
      PATH_SUFFIXES "lib" ${ARCH_DEPENDENT_LIB_PATHS}
      NO_DEFAULT_PATH)
    
    IF(NOT ${OpenCV_${L}_LIBRARY} STREQUAL "OpenCV_${L}_LIBRARY-NOTFOUND")
      LIST(APPEND OpenCV_LIBRARIES ${OpenCV_${L}_LIBRARY})
    ELSE()
      SET(OpenCV_OLD_LIBS_NOT_FOUND "TRUE")
    ENDIF()
  ENDFOREACH()  
  
  IF(${OpenCV_OLD_LIBS_NOT_FOUND} STREQUAL "FALSE")
    MESSAGE(STATUS "Found (old style) OpenCV libs: ${OpenCV_LIBRARIES}")
  ELSE()
    # old style libraries not found -> search for new style libraries
    SET(OpenCV_LIBRARIES "")

    FOREACH(L core highgui imgproc video ml calib3d)
      FIND_LIBRARY(OpenCV_${L}_LIBRARY
        NAMES "libopencv_${L}" "opencv_${L}"
        PATHS "${OpenCV_LIB_DIR}"
        PATH_SUFFIXES "lib" ${ARCH_DEPENDENT_LIB_PATHS}
        NO_DEFAULT_PATH)
      
      IF(NOT ${OpenCV_${L}_LIBRARY} STREQUAL "OpenCV_${L}_LIBRARY-NOTFOUND")
        LIST(APPEND OpenCV_LIBRARIES ${OpenCV_${L}_LIBRARY})
      ELSE()
        SET(OpenCV_NEW_LIBS_NOT_FOUND "TRUE")
      ENDIF()
    ENDFOREACH()  
    
    IF(${OpenCV_NEW_LIBS_NOT_FOUND} STREQUAL "FALSE")
      MESSAGE(STATUS "Found OpenCV libs: ${OpenCV_LIBRARIES}") 
    ENDIF()
  
  ENDIF()
  
  IF(${OpenCV_NEW_LIBS_NOT_FOUND} STREQUAL "TRUE" AND 
     ${OpenCV_OLD_LIBS_NOT_FOUND} STREQUAL "TRUE")
   MESSAGE(CRITICAL_ERROR "Not Found OpenCV Libraries (neither old- nor new-style)")
  ENDIF()
      
  ENDIF(NOT WIN32)
ENDIF()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenCV REQUIRED_VARS
                                  OpenCV_LIBRARIES
                                  OpenCV_INCLUDE_DIRS)
