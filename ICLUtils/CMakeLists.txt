#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLUtils/CMakeLists.txt                                **
#** Module : ICLUtils                                               **
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

SET(SOURCES src/ICLUtils/ConfigFile.cpp
            src/ICLUtils/Configurable.cpp
		src/ICLUtils/ConsoleProgress.cpp
	    src/ICLUtils/CLBuffer.cpp
	    src/ICLUtils/CLImage2D.cpp
	    src/ICLUtils/CLKernel.cpp
		src/ICLUtils/CLProgram.cpp
		src/ICLUtils/CLDeviceContext.cpp
		src/ICLUtils/CLMemoryAssistant.cpp
	    src/ICLUtils/Exception.cpp
	    src/ICLUtils/File.cpp
	    src/ICLUtils/FPSEstimator.cpp
	    src/ICLUtils/FPSLimiter.cpp
	    src/ICLUtils/MultiThreader.cpp
	    src/ICLUtils/MultiTypeMap.cpp
	    src/ICLUtils/Mutex.cpp
	    src/ICLUtils/Point32f.cpp
	    src/ICLUtils/Point.cpp
	    src/ICLUtils/ProcessMonitor.cpp
	    src/ICLUtils/ProgArg.cpp
	    src/ICLUtils/PugiXML.cpp
	    src/ICLUtils/Random.cpp
	    src/ICLUtils/Range.cpp
	    src/ICLUtils/Rect32f.cpp
	    src/ICLUtils/Rect.cpp
	    src/ICLUtils/Semaphore.cpp
	    src/ICLUtils/SignalHandler.cpp
	    src/ICLUtils/Size32f.cpp
	    src/ICLUtils/Size.cpp
	    src/ICLUtils/SmartPtr.cpp
	    src/ICLUtils/SteppingRange.cpp
	    src/ICLUtils/StringUtils.cpp
	    src/ICLUtils/StrTok.cpp
	    src/ICLUtils/TextTable.cpp
	    src/ICLUtils/Thread.cpp
	    src/ICLUtils/Time.cpp
	    src/ICLUtils/Timer.cpp)

SET(HEADERS src/ICLUtils/Any.h
            src/ICLUtils/Array2D.h
	    src/ICLUtils/BasicTypes.h
	    src/ICLUtils/ClippedCast.h
		src/ICLUtils/CLIncludes.h
	    src/ICLUtils/CLBuffer.h
		src/ICLUtils/CLMemory.h
	    src/ICLUtils/CLImage2D.h
	    src/ICLUtils/CLException.h
	    src/ICLUtils/CLKernel.h
		src/ICLUtils/CLProgram.h
		src/ICLUtils/CLDeviceContext.h
		src/ICLUtils/CLMemoryAssistant.h
	    src/ICLUtils/CompatMacros.h
            src/ICLUtils/ConfigFile.h
            src/ICLUtils/PluginRegister.h
            src/ICLUtils/Configurable.h
            src/ICLUtils/ConfigurableProxy.h
            src/ICLUtils/FixedArray.h
	    src/ICLUtils/ConsoleProgress.h
	    src/ICLUtils/Exception.h
	    src/ICLUtils/FastMedianList.h
	    src/ICLUtils/File.h
	    src/ICLUtils/FPSEstimator.h
	    src/ICLUtils/FPSLimiter.h
	    src/ICLUtils/Function.h
	    src/ICLUtils/Lockable.h
	    src/ICLUtils/Macros.h
	    src/ICLUtils/MultiThreader.h
	    src/ICLUtils/MultiTypeMap.h
	    src/ICLUtils/Mutex.h
	    src/ICLUtils/ParamList.h
	    src/ICLUtils/Point32f.h
	    src/ICLUtils/Point.h
	    src/ICLUtils/ProcessMonitor.h
	    src/ICLUtils/ProgArg.h
	    src/ICLUtils/PThreadFix.h
	    src/ICLUtils/PugiXML.h
	    src/ICLUtils/Random.h
	    src/ICLUtils/Range.h
	    src/ICLUtils/Rect32f.h
	    src/ICLUtils/Rect.h
	    src/ICLUtils/Semaphore.h
	    src/ICLUtils/ShallowCopyable.h
	    src/ICLUtils/SignalHandler.h
	    src/ICLUtils/Size32f.h
	    src/ICLUtils/Size.h
	    src/ICLUtils/SmartPtrBase.h
	    src/ICLUtils/SmartPtr.h
	    src/ICLUtils/SmartArray.h
	    src/ICLUtils/StackTimer.h
	    src/ICLUtils/SteppingRange.h
	    src/ICLUtils/StringUtils.h
	    src/ICLUtils/StrTok.h
	    src/ICLUtils/TestAssertions.h
	    src/ICLUtils/TextTable.h
	    src/ICLUtils/Thread.h
	    src/ICLUtils/Time.h
	    src/ICLUtils/Timer.h
	    src/ICLUtils/UncopiedInstance.h
	    src/ICLUtils/Uncopyable.h
	    src/ICLUtils/Utils.h
	    src/ICLUtils/VisualizationDescription.h
	    src/ICLUtils/XML.h
            src/ICLUtils/SSEUtils.h
            src/ICLUtils/SSETypes.h)

IF(DL_FOUND)
  LIST(APPEND SOURCES src/ICLUtils/IppInterface.cpp)
  LIST(APPEND HEADERS src/ICLUtils/IppInterface.h)
ENDIF()

IF(WIN32)
INCLUDE_DIRECTORIES(BEFORE src
                           "${ICL_SOURCE_DIR}/3rdparty/zlib"
                           "${ICL_SOURCE_DIR}/3rdparty/libpng"
                           "${ICL_SOURCE_DIR}/3rdparty/libjpeg"
                           "${ICL_SOURCE_DIR}/3rdparty/regex"
                           "${CMAKE_BINARY_DIR}/src")
ELSE()
INCLUDE_DIRECTORIES(BEFORE src
						   "${CMAKE_BINARY_DIR}/src")
ENDIF()

ADD_LIBRARY(ICLUtils SHARED ${SOURCES} ${HEADERS})

# ---- 3rd party library handling ----
TARGET_LINK_LIBRARIES(ICLUtils ${ICLUtils_3RDPARTY_LIBRARIES})

SET_TARGET_PROPERTIES(ICLUtils
                      PROPERTIES
		      VERSION ${SO_VERSION})

# ---- Build examples ----
IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF()

IF(BUILD_APPS)
  ADD_SUBDIRECTORY(apps)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ICLUtils
        COMPONENT libraries
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL_FILES_RECURSIVE("include/${INSTALL_PATH_PREFIX}" HEADERS)

# ---- PKG-config ----
IF(IPP_FOUND)
  CREATE_PKGCONFIG(NAME ICLUtils
                   LIBRARY_DEPS ${ICLUtils_3RDPARTY_LIBRARIES}
                   RPATH_DEPS -Wl,-rpath=${IPP_LIB_DIR}:${IOMP_LIB_DIR})
ELSE()
  CREATE_PKGCONFIG(NAME ICLUtils
                   LIBRARY_DEPS ${ICLUtils_3RDPARTY_LIBRARIES})
ENDIF()

IF(BUILD_TESTS)
  CONFIGURE_GTEST(ICLUtils ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BUILD_TESTS)
