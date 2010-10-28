#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : ICLGeom/CMakeLists.txt                                 **
#** Module : ICLGeom                                                **
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
project(ICLGeom)

set(CONDITIONLIST "")
#sources for lib
set(LIB_SOURCES "")
add_libsource (${PROJECT_NAME} "src/Camera.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ConvexHullMonotoneChain.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Scene.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SceneMouseHandler.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SceneObject.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PlaneEquation.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/ViewRay.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/PoseEstimator.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/Posit.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/SoftPosit.cpp" "${CONDITIONLIST}" LIB_SOURCES)
add_libsource (${PROJECT_NAME} "src/CoplanarPointPoseEstimator.cpp" "${CONDITIONLIST}" LIB_SOURCES)
set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLQt)
add_example (${PROJECT_NAME} "softposit-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "show-extrinsic-calibration-grid" "${CONDITIONLIST}" ICLLibsToLinkAgainst)


set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND HAVE_OPENGL_COND)
add_example (${PROJECT_NAME} "art-like-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "cam-calib" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "geom-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "geom-demo-simple" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "geom-property-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)
add_example (${PROJECT_NAME} "swiss-ranger-demo" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST ICL_VAR_BUILD_EXAMPLES HAVE_QT_COND HAVE_IPP_COND)
set(ICLLibsToLinkAgainst ${PROJECT_NAME} ICLAlgorithms ICLQuick  ICLBlob ICLFilter ICLCC ICLCore ICLUtils ${QT_LIBS_l})
add_example_mult(${PROJECT_NAME} "intrinsic-camera-calibration" "examples/intrinsic-camera-calibration.cpp;examples/intrinsic-camera-calibration-tools.cpp;examples/intrinsic-camera-calibration-tools.h" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

set(CONDITIONLIST "HAVE_GTEST_COND")
set(ICLLibsToLinkAgainst ICLGeom ICLUtils ICLCore)
add_gtest (${PROJECT_NAME} "test-camera" "${CONDITIONLIST}" ICLLibsToLinkAgainst)

#create lib
if(WIN32)
add_library(${PROJECT_NAME} ${LIB_SOURCES})
else()
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
endif()
#linker
target_link_libraries(${PROJECT_NAME} ${ICLGeom_dependencies})

link_directories("${LIBRARY_OUTPUT_PATH}")
if(WIN32)
install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}"
	 #MINGW COMPATIBILITY 
	ARCHIVE DESTINATION "${LIBRARY_OUTPUT_PATH}"
	RUNTIME DESTINATION "${LIBRARY_OUTPUT_PATH}")
else()
install (TARGETS ${PROJECT_NAME}
 	 LIBRARY DESTINATION "${LIBRARY_OUTPUT_PATH}")
endif()
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icl-${VERSION}${DEBUG_POSTFIX}
	FILES_MATCHING PATTERN "*.h"
	REGEX .svn* EXCLUDE
)
add_doc_gen(${PROJECT_NAME})
