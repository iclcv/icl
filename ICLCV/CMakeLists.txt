file(GLOB SOURCES src/ICLCV/*.cpp)
file(GLOB HEADERS src/ICLCV/*.h)

if(NOT QT_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/HeartrateDetector.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/HeartrateDetector.h)
endif()

if(NOT IPP_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/ViewBasedTemplateMatcher.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/TemplateTracker.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/ViewBasedTemplateMatcher.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/TemplateTracker.h)
endif()

if(NOT OPENCV_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/CheckerboardDetector.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/ORBFeatureDetector.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenSurfLib.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/LensUndistortionCalibrator.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenCVCamCalib.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/CheckerboardDetector.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/ORBFeatureDetector.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenSurfLib.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/LensUndistortionCalibrator.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenCVCamCalib.h)
else()
  if(NOT OPENCV_FEATURES_2D_FOUND)
    list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/ORBFeatureDetector.cpp)
    list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/ORBFeatureDetector.h)
  endif()
  if(NOT OpenCV_VERSION VERSION_LESS "4.0")
    list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenSurfLib.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/LensUndistortionCalibrator.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenCVCamCalib.cpp)
    list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenSurfLib.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/LensUndistortionCalibrator.h
                             ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/OpenCVCamCalib.h)
  endif()
endif()

if(NOT OPENCL_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/CLSurfLib.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/CLSurfLibKernels.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/CLSurfLib.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLCV/CLSurfLibKernels.h)
endif()

icl_add_module(ICLCV
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLIO
  PKGCONFIG_DEPS ICLIO-${SO_VERSION}
  HAS_DEMOS HAS_APPS)

# HeartrateDetector needs ICLQt headers but we can't link against ICLQt (circular dep)
if(QT_FOUND)
  target_include_directories(ICLCV PRIVATE ${ICL_SOURCE_DIR}/ICLQt/src)
endif()
