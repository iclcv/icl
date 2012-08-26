#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLCV/CMakeLists.txt                                   **
#** Module : ICLCV                                                  **
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
project(ICLCV)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/ColorBlobSearcher.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/DefaultColorBlobSearcher.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Extrapolator.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/HungarianAlgorithm.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PositionTracker.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/VectorTracker.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImageRegion.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ImageRegionData.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/RegionDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/RunLengthEncoder.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/RegionPCAInfo.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/VQ2D.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SimpleBlobSearcher.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/MeanShiftTracker.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/FloodFiller.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST HAVE_IPP_COND)
add_libsource (${PROJECT_NAME} "src/UsefulFunctions.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ViewBasedTemplateMatcher.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/HoughLineDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/HoughLine.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/TemplateTracker.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST HAVE_OPENCV_COND)
add_libsource (${PROJECT_NAME} "src/OpenCVSurfDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/GenericSurfDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)

set(CONDITIONLIST HAVE_OPENSURF_COND HAVE_OPENCV_COND)
add_libsource (${PROJECT_NAME} "src/OpenSurfDetector.cpp" "${CONDITIONLIST}" LIB_SOURCES)


set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES)
set(ICLLibsToLinkAgainst ${PROJECT_NAME})
add_example (${PROJECT_NAME} "quick-example" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND)
add_example (${PROJECT_NAME} "region-inspector" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "vector-tracker-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "simple-blob-searcher-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "mean-shift-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "corner-detection-css-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "region-detection-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "flood-filler-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "quick-diff-image-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_IPP_COND HAVE_QT_COND)
add_example (${PROJECT_NAME} "template-matching-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "hough-line-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

# use or instead of and ...
set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND HAVE_OPENCV_COND)
add_example (${PROJECT_NAME} "surf-detector-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)


set(CONDITIONLIST HAVE_GTEST_COND)
set(ICLLibsToLinkAgainst "${GTEST_LIBS_l}")
if(NOT WIN32)
add_gtest (${PROJECT_NAME} "test-region-detector" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
endif()
#create lib
if(WIN32)
add_library(${PROJECT_NAME} ${LIB_SOURCES})
else()
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
endif()
#linker
target_link_libraries(${PROJECT_NAME} ${ICLCV_dependencies})

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