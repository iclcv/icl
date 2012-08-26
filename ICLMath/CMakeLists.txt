#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLMath/CMakeLists.txt                                 **
#** Module : ICLMath                                                **
#** Authors: Christian Groszewski                                   **
#**                                                                 **
#**                                                                 **
#** Commercial License                                              **
#** ICL can be used commercially, please refer to our website       **
#** www.iclcv.org for more details.                                 **
#**                                                                 **
#** GNU General Public License Usage                                **
#** Alternatively, this file may be used under the terms of the     **
#** GNU General Public License version 3.0 as published by the      **
#** Free Software Foundation and appearing in the file LICENSE.GPL  **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the GNU General Public License  **
#** version 3.0 requirements will be met:                           **
#** http://www.gnu.org/copyleft/gpl.html.                           **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************
project(ICLMath)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/SOM.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/LLM.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DynMatrix.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FFTUtils.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FixedMatrix.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DynMatrixUtils.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/StochasticOptimizer.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/LevenbergMarquardtFitter.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SimplexOptimizer.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Homography2D.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/StraightLine2D.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES)
set(ICLLibsToLinkAgainst ${PROJECT_NAME})
add_example (${PROJECT_NAME} "levenberg-marquardt-example" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND)
add_example (${PROJECT_NAME} "llm-1D-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "llm-1D-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST HAVE_GTEST_COND)
set(ICLLibsToLinkAgainst "${GTEST_LIBS_l}")
#add_gtest (${PROJECT_NAME} "test-image" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

#create lib
if(WIN32)
add_library(${PROJECT_NAME} ${LIB_SOURCES})
else()
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
endif()
#linker
target_link_libraries(${PROJECT_NAME} ${ICLMath_dependencies})

link_directories("${LIBRARY_OUTPUT_PATH}")
if(WIN32)
install (TARGETS ${PROJECT_NAME}  
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
	 #MINGW COMPATIBILITY 
     ARCHIVE DESTINATION "${LIBRARY_OUTPUT_PATH}"
	 RUNTIME DESTINATION "${LIBRARY_OUTPUT_PATH}")
else()
install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
	 PERMISSIONS OWNER_WRITE OWNER_READ
	 GROUP_WRITE GROUP_READ 
	 WORLD_READ
)
endif()

install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icl-${VERSION}${DEBUG_POSTFIX}
		 FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
		 DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
		 	GROUP_EXECUTE GROUP_WRITE GROUP_READ 
		 	WORLD_READ WORLD_EXECUTE
		 FILES_MATCHING PATTERN "*.h"
		 REGEX .svn* EXCLUDE
)
add_doc_gen(${PROJECT_NAME})