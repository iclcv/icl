#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLCore/CMakeLists.txt                                 **
#** Module : ICLCore                                                **
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

SET(SOURCES src/ICLCore/BayerConverter.cpp
            src/ICLCore/CCFunctions.cpp
	    src/ICLCore/CCLUT.cpp
	    src/ICLCore/Color.cpp
	    src/ICLCore/Converter.cpp
	    src/ICLCore/CoreFunctions.cpp
	    src/ICLCore/FixedConverter.cpp
	    src/ICLCore/ImageRenderer.cpp
	    src/ICLCore/ImageSerializer.cpp
	    src/ICLCore/ImgBase.cpp
	    src/ICLCore/ImgBorder.cpp
	    src/ICLCore/ImgBuffer.cpp
	    src/ICLCore/Img.cpp
	    src/ICLCore/ImgParams.cpp
	    src/ICLCore/Line32f.cpp
	    src/ICLCore/Line.cpp
	    src/ICLCore/LineSampler.cpp
	    src/ICLCore/ConvexHull.cpp
	    src/ICLCore/AbstractCanvas.cpp
	    src/ICLCore/PseudoColorConverter.cpp)

SET(HEADERS src/ICLCore/BayerConverter.h
            src/ICLCore/CCFunctions.h
	    src/ICLCore/CCLUT.h
	    src/ICLCore/Channel.h
	    src/ICLCore/ChromaAndRGBClassifier.h
	    src/ICLCore/ChromaClassifier.h
	    src/ICLCore/Color.h
	    src/ICLCore/Converter.h
	    src/ICLCore/CoreFunctions.h
	    src/ICLCore/Core.h
	    src/ICLCore/FixedConverter.h
	    src/ICLCore/ImageRenderer.h
	    src/ICLCore/ImageSerializer.h
	    src/ICLCore/ImgBase.h
	    src/ICLCore/ImgBorder.h
	    src/ICLCore/ImgBuffer.h
	    src/ICLCore/Img.h
	    src/ICLCore/ImgIterator.h
	    src/ICLCore/ImgParams.h
	    src/ICLCore/Line32f.h
	    src/ICLCore/Line.h
	    src/ICLCore/LineSampler.h
	    src/ICLCore/Parable.h
	    src/ICLCore/PixelRef.h
	    src/ICLCore/ConvexHull.h
	    src/ICLCore/AbstractCanvas.h
	    src/ICLCore/PseudoColorConverter.h
	    src/ICLCore/Types.h
	    src/ICLCore/DataSegment.h
	    src/ICLCore/DataSegmentBase.h)

IF(OpenCV_FOUND)
  LIST(APPEND SOURCES src/ICLCore/OpenCV.cpp)
  LIST(APPEND HEADERS src/ICLCore/OpenCV.h)
  # if the error also occurs on OSX use 'MATCHES "Clang"' instead
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Related to narrowing issues https://github.com/opencv/opencv/issues/8192
    set_source_files_properties(src/ICLCore/OpenCV.cpp PROPERTIES COMPILE_FLAGS -Wno-narrowing)
  endif()
ENDIF()

# ---- Library build instructions ----
IF(WIN32)
INCLUDE_DIRECTORIES(BEFORE src
                           ${ICL_SOURCE_DIR}/3rdparty/zlib
                           ${ICL_SOURCE_DIR}/3rdparty/libpng
                           ${ICL_SOURCE_DIR}/3rdparty/libjpeg
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src)
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
                           ${CMAKE_BINARY_DIR}/src
                           ${ICL_SOURCE_DIR}/ICLUtils/src
                           ${ICL_SOURCE_DIR}/ICLMath/src)
ENDIF()

ADD_LIBRARY(ICLCore SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLCore ICLMath
                              ${ICLCore_3RDPARTY_LIBRARIES})

SET_TARGET_PROPERTIES(ICLCore
                      PROPERTIES
		      VERSION ${SO_VERSION})

# ---- Build examples ----
IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF()

IF(BUILD_DEMOS)
  ADD_SUBDIRECTORY(demos)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ICLCore
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
#$(OpenCV_LIBS) contains the lib-names only -> prepend libdir
#SET(OpenCV_PKG_LIBS "${OpenCV_")
#FOREACH(L ${OpenCV_LIBRARIES})
#  SET(OpenCV_PKG_LIBS "${OpenCV_PKG_LIBS} ${OpenCV_LIB_DIR}/lib${L}.so")
#ENDFOREACH()

CREATE_PKGCONFIG(NAME ICLCore
                 LIBRARY_DEPS ${ICLCore_3RDPARTY_LIBRARIES}
                 PKGCONFIG_DEPS ICLMath-${SO_VERSION})

IF(BUILD_TESTS)
 CONFIGURE_GTEST(ICLCore ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
