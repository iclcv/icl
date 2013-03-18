#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLFilter/src/CMakeLists.txt                           **
#** Module : ICLFilter                                              **
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

SET(SOURCES src/ICLFilter/AffineOp.cpp
            src/ICLFilter/BinaryArithmeticalOp.cpp
	    src/ICLFilter/BinaryCompareOp.cpp
	    src/ICLFilter/BinaryLogicalOp.cpp
	    src/ICLFilter/BinaryOp.cpp
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
	    src/ICLFilter/LocalThresholdOp.cpp
	    src/ICLFilter/LUTOp3Channel.cpp
	    src/ICLFilter/LUTOp.cpp
	    src/ICLFilter/MedianOp.cpp
	    src/ICLFilter/MirrorOp.cpp
	    src/ICLFilter/MorphologicalOp.cpp
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
	    src/ICLFilter/WeightedSumOp.cpp)

SET(HEADERS src/ICLFilter/AffineOp.h
            src/ICLFilter/BaseAffineOp.h
            src/ICLFilter/BinaryArithmeticalOp.h
	    src/ICLFilter/BinaryCompareOp.h
	    src/ICLFilter/BinaryLogicalOp.h
	    src/ICLFilter/BinaryOp.h
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
	    src/ICLFilter/IntegralImgOp.h
	    src/ICLFilter/LocalThresholdOp.h
	    src/ICLFilter/LUT2D.h
	    src/ICLFilter/LUTOp3Channel.h
	    src/ICLFilter/LUTOp.h
	    src/ICLFilter/MedianOp.h
	    src/ICLFilter/MirrorOp.h
	    src/ICLFilter/MorphologicalOp.h
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
	    src/ICLFilter/WeightedSumOp.h)

IF(IPP_FOUND)
  LIST(APPEND SOURCES src/ICLFilter/WienerOp.cpp
                      src/ICLFilter/CannyOp.cpp
		      src/ICLFilter/ProximityOp.cpp)

  LIST(APPEND HEADERS src/ICLFilter/WienerOp.h
                      src/ICLFilter/CannyOp.h
		      src/ICLFilter/ProximityOp.h)	    
ENDIF()

INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src
                           ${ICL_SOURCE_DIR}/ICLCore/src)

ADD_LIBRARY(ICLFilter SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLFilter ICLCore)

# ---- Install specifications ----
INSTALL(TARGETS ICLFilter
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
