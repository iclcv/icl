#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLFilter/CMakeLists.txt                               **
#** Module : ICLFilter                                              **
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

SET(SOURCES src/ICLFilter/AffineOp.cpp
            src/ICLFilter/BinaryArithmeticalOp.cpp
	    src/ICLFilter/BinaryCompareOp.cpp
	    src/ICLFilter/BinaryLogicalOp.cpp
	    src/ICLFilter/BinaryOp.cpp
            src/ICLFilter/CannyOp.cpp
	    src/ICLFilter/ChamferOp.cpp
	    src/ICLFilter/ColorDistanceOp.cpp
	    src/ICLFilter/ColorSegmentationOp.cpp
	    src/ICLFilter/ConvolutionKernel.cpp
	    src/ICLFilter/ConvolutionOp.cpp
	    src/ICLFilter/DynamicConvolutionOp.cpp
	    src/ICLFilter/FFTOp.cpp
	    src/ICLFilter/GaborOp.cpp
	    src/ICLFilter/GradientImage.cpp
	    src/ICLFilter/IFFTOp.cpp
	    src/ICLFilter/ImageSplitter.cpp
	    src/ICLFilter/InplaceArithmeticalOp.cpp
	    src/ICLFilter/InplaceLogicalOp.cpp
	    src/ICLFilter/IntegralImgOp.cpp
	    src/ICLFilter/LUTOp.cpp
	    src/ICLFilter/LUTOp3Channel.cpp
	    src/ICLFilter/LocalThresholdOp.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_8u_true.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_16s_true.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_32s_true.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_32f_true.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_64f_true.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_8u_false.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_16s_false.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_32s_false.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_32f_false.cpp
	    src/ICLFilter/LocalThresholdOpHelpers_64f_false.cpp
	    src/ICLFilter/MedianOp.cpp
	    src/ICLFilter/MirrorOp.cpp
	    src/ICLFilter/MorphologicalOp.cpp
	    src/ICLFilter/MotionSensitiveTemporalSmoothing.cpp
	    src/ICLFilter/NeighborhoodOp.cpp
	    src/ICLFilter/OpROIHandler.cpp
	    src/ICLFilter/ThresholdOp.cpp
	    src/ICLFilter/UnaryArithmeticalOp.cpp
	    src/ICLFilter/UnaryCompareOp.cpp
	    src/ICLFilter/UnaryLogicalOp.cpp
	    src/ICLFilter/UnaryOp.cpp
	    src/ICLFilter/UnaryOpPipe.cpp
	    src/ICLFilter/WarpOp.cpp
	    src/ICLFilter/WeightChannelsOp.cpp
	    src/ICLFilter/WeightedSumOp.cpp
	    src/ICLFilter/ImageRectification.cpp
		src/ICLFilter/DitheringOp.cpp
		src/ICLFilter/BilateralFilterOp.cpp)

SET(HEADERS src/ICLFilter/AffineOp.h
            src/ICLFilter/BaseAffineOp.h
            src/ICLFilter/BinaryArithmeticalOp.h
	    src/ICLFilter/BinaryCompareOp.h
	    src/ICLFilter/BinaryLogicalOp.h
	    src/ICLFilter/BinaryOp.h
            src/ICLFilter/CannyOp.h
	    src/ICLFilter/ChamferOp.h
	    src/ICLFilter/ColorDistanceOp.h
	    src/ICLFilter/ColorSegmentationOp.h
	    src/ICLFilter/ConvolutionKernel.h
	    src/ICLFilter/ConvolutionOp.h
	    src/ICLFilter/DynamicConvolutionOp.h
	    src/ICLFilter/FFTOp.h
	    src/ICLFilter/Filter.h
	    src/ICLFilter/GaborOp.h
	    src/ICLFilter/GradientImage.h
	    src/ICLFilter/IFFTOp.h
	    src/ICLFilter/ImageSplitter.h
	    src/ICLFilter/InplaceArithmeticalOp.h
	    src/ICLFilter/InplaceLogicalOp.h
	    src/ICLFilter/InplaceOp.h
	    src/ICLFilter/IntegralImgOp.h
	    src/ICLFilter/LUT2D.h
	    src/ICLFilter/LUTOp.h
	    src/ICLFilter/LUTOp3Channel.h
	    src/ICLFilter/LocalThresholdOp.h
	    src/ICLFilter/LocalThresholdOpHelpers.h
	    src/ICLFilter/MedianOp.h
	    src/ICLFilter/MirrorOp.h
	    src/ICLFilter/MorphologicalOp.h
	    src/ICLFilter/MotionSensitiveTemporalSmoothing.h
	    src/ICLFilter/NeighborhoodOp.h
	    src/ICLFilter/OpROIHandler.h
	    src/ICLFilter/RotateOp.h
	    src/ICLFilter/ScaleOp.h
	    src/ICLFilter/ThresholdOp.h
	    src/ICLFilter/TranslateOp.h
	    src/ICLFilter/UnaryArithmeticalOp.h
	    src/ICLFilter/UnaryCompareOp.h
	    src/ICLFilter/UnaryLogicalOp.h
	    src/ICLFilter/UnaryOp.h
	    src/ICLFilter/UnaryOpPipe.h
	    src/ICLFilter/UnaryOpWork.h
	    src/ICLFilter/WarpOp.h
	    src/ICLFilter/WeightChannelsOp.h
			src/ICLFilter/WeightedSumOp.h
			src/ICLFilter/ImageRectification.h
			src/ICLFilter/DitheringOp.h
			src/ICLFilter/BilateralFilterOp.h)

# opencl kernel integration
SET(KERNEL )
LIST(APPEND KERNEL src/ICLFilter/OpenCL/BilateralFilterOp.cl)
foreach(kernel_file ${KERNEL})
	get_filename_component(kernel_source_name ${kernel_file} NAME_WE)
	string(REGEX REPLACE ".cl$" "Kernel.h" kernel_header_file "${kernel_file}")
	CREATE_CL_HEADER("${kernel_file}" "${kernel_header_file}" "${kernel_source_name}KernelSource" filter)
	LIST(APPEND HEADERS ${kernel_header_file})
endforeach()

IF(IPP_FOUND)
  LIST(APPEND SOURCES src/ICLFilter/WienerOp.cpp
		      src/ICLFilter/ProximityOp.cpp)

  LIST(APPEND HEADERS src/ICLFilter/WienerOp.h
			  src/ICLFilter/ProximityOp.h)
ENDIF()

# ---- Library build instructions ----
IF(WIN32)
INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/3rdparty/zlib
                           ${ICL_SOURCE_DIR}/3rdparty/libpng
                           ${ICL_SOURCE_DIR}/3rdparty/libjpeg
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src)
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src)
ENDIF()

ADD_LIBRARY(ICLFilter SHARED ${SOURCES} ${HEADERS} ${KERNEL})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLFilter ICLCore)

SET_TARGET_PROPERTIES(ICLFilter
                      PROPERTIES
		      VERSION ${SO_VERSION})

# ---- Build examples/ demos/ apps ----
IF(BUILD_DEMOS)
  ADD_SUBDIRECTORY(demos)
ENDIF()

IF(BUILD_APPS)
  ADD_SUBDIRECTORY(apps)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ICLFilter
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
CREATE_PKGCONFIG(NAME ICLFilter
                 PKGCONFIG_DEPS ICLCore-${SO_VERSION})

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLFilter ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
