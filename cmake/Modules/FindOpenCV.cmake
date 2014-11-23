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

SET(OPENCV_VERSION_MAJOR 0)
SET(OPENCV_VERSION_MINOR 0)
SET(OPENCV_VERSION_PATCH 0)

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
  SET(OpenCV_OLD_LIBS_NOT_FOUND "FALSE")

  # Search for old style libraries first
  ICL_FIND_PACKAGE(NAME OpenCV
                   HEADERS "opencv/cv.h;opencv/cxcore.h;opencv/highgui.h"
                   LIBS "cxcore;cv;ml;highgui;cvaux"
                   PATHS ${OpenCV_LIB_DIR}
                   OPTIONAL)

  # If not found search for new style libraries
  IF(NOT OPENCV_FOUND)
    SET(OpenCV_OLD_LIBS_NOT_FOUND "TRUE")

    ICL_FIND_PACKAGE(NAME OpenCV
                     HEADERS "opencv/cv.h;opencv/cxcore.h;opencv/highgui.h;features2d/features2d.hpp"
                     LIBS "opencv_core;opencv_highgui;opencv_imgproc;opencv_video;opencv_ml;opencv_calib3d;opencv_features2d"
                     PATHS ${OpenCV_LIB_DIR})

    IF(OPENCV_FOUND)
      SET(OPENCV_VERSION_FILE "${OPENCV_INCLUDE_DIRS}/opencv2/core/version.hpp")
      MESSAGE(STATUS "OpenCV version file is ${OPENCV_VERSION_FILE}")
      SET(CV_VEPO) 
      SET(CV_VMAJ) 
      SET(CV_VMIN)
      FILE(STRINGS ${OPENCV_VERSION_FILE} CV_VEPO REGEX "#define CV_VERSION_EPOCH")
      FILE(STRINGS ${OPENCV_VERSION_FILE} CV_VMAJ REGEX "#define CV_VERSION_MAJOR")
      FILE(STRINGS ${OPENCV_VERSION_FILE} CV_VMIN REGEX "#define CV_VERSION_MINOR")

      STRING(REGEX MATCHALL "[0-9]+" OPENCV_VERSION_MAJOR ${CV_VEPO})
      STRING(REGEX MATCHALL "[0-9]+" OPENCV_VERSION_MINOR ${CV_VMAJ})
      STRING(REGEX MATCHALL "[0-9]+" OPENCV_VERSION_PATCH ${CV_VMIN})

      MESSAGE(STATUS "Found OpenCV version ${OPENCV_VERSION_EPOCH}.${OPENCV_VERSION_MAJOR}.${OPENCV_VERSION_MINOR}")
    ENDIF()

  ENDIF(NOT OPENCV_FOUND)

  SET(OpenCV_INCLUDE_DIRS ${OPENCV_INCLUDE_DIRS})
  SET(OpenCV_LIBRARIES ${OPENCV_LIBRARIES})
ENDIF()
