#********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : CMakeLists.txt                                         **
#** Module :                                                        **
#** Authors: Christian Groszewski, Christof Elbrechter              **
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
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)
PROJECT(ICL)
#ubuntu version
set(ICL_MAJOR "7")
set(ICL_MINOR "8")
set(ICL_PATCH "0")
set(VERSION "${ICL_MAJOR}.${ICL_MINOR}.${ICL_PATCH}")
cmake_policy(SET CMP0002 OLD)
if(NOT ${CMAKE_PATCH_VERSION} STREQUAL "0")
	cmake_policy(SET CMP0015 OLD)
endif()


set(ICL_VAR_ARCHITECTURE_DEPENDENT_BUILD_ON OFF CACHE BOOL "Compile architecture dependent (adding -march=native to CXX_FLAGS)")
set(ICL_VAR_DEBUG_MODE OFF CACHE BOOL "Compile with debug options")
set(ICL_VAR_USE_OMP OFF CACHE BOOL "add -fopenmp to compiler flags (might not work in combination with Intel IPP or MKL)")


include(cmake_files/build_options.cmake)

if(UNIX)
  include(cmake_files/macros-linux.cmake)
elseif(APPLE)
  include(cmake_files/macros-apple.cmake)
elseif(WIN32)
  # not supported yet
endif()

TODAY(DATE)

set(DOC_VERSION_STRING "(build ${VERSION})")
set(DOC_DATE_STRING "(build date ${DATE})")

#if ON examples apps will be build and install
set(ICL_VAR_BUILD_EXAMPLES ON CACHE BOOL "Build example applications")

set(LIBRARY_OUTPUT_PATH "lib/icl-${VERSION}${DEBUG_POSTFIX}")
set(EXECUTABLE_OUTPUT_PATH "bin/icl-${VERSION}${DEBUG_POSTFIX}")
set(DOC_DIR "doc/icl-${VERSION}${DEBUG_POSTFIX}")
set(ETC_DIR "etc/icl-${VERSION}${DEBUG_POSTFIX}")

#checkboxes to indicate a third party lib will be used
set(ICL_XDEP_IPP_ON OFF CACHE BOOL "Use IPP if available")
set(ICL_XDEP_MKL_ON OFF CACHE BOOL "Use MKL if available")
set(ICL_XDEP_OPENCV_ON OFF CACHE BOOL "Use OPENCV if available")
set(ICL_XDEP_LIBZ_ON OFF CACHE BOOL "Use LIBZ if available")
set(ICL_XDEP_LIBMESASR_ON OFF CACHE BOOL "Use LIBMESASR if available")
set(ICL_XDEP_LIBJPEG_ON OFF CACHE BOOL "Use LIBJPEG if available")
set(ICL_XDEP_IMAGEMAGICK_ON OFF CACHE BOOL "Use IMAGEMAGICK if available")
set(ICL_XDEP_LIBPNG_ON OFF CACHE BOOL "Use LIBPNG if available")
set(ICL_XDEP_LIBDC_ON OFF CACHE BOOL "Use LIBDC if available")
set(ICL_XDEP_LIBFREENECT_ON OFF CACHE BOOL "Use libfreenect if available")
set(ICL_XDEP_UNICAP_ON OFF CACHE BOOL "Use UNICAP if available")
set(ICL_XDEP_XINE_ON OFF CACHE BOOL "Use XINE if available")
set(ICL_XDEP_XCF_ON OFF CACHE BOOL "Use XCF if available")
set(ICL_XDEP_QT_ON OFF CACHE BOOL "Use QT if available")
set(ICL_XDEP_OPENGL_ON OFF CACHE BOOL "Use OPENGL if available")
set(ICL_XDEP_GLX_ON OFF CACHE BOOL "Use GLX if available")
set(ICL_XDEP_OPENSURF_ON OFF CACHE BOOL "Use OPENSURF if available")
set(ICL_XDEP_GTEST_ON OFF CACHE BOOL "Use GTest if available")
set(ICL_XDEP_VIDEODEV_ON OFF CACHE BOOL "Use videodev (v4l) headers if available")
set(ICL_XDEP_VIDEODEV2_ON OFF CACHE BOOL "Use videodev2 (v4l2) headers if available")
set(ICL_XDEP_PYLON_ON ON CACHE BOOL "Use Pylon if available")
set(ICL_XDEP_GENICAM_ON ON CACHE BOOL "Use GenICam if available")
set(ICL_XDEP_OPENCL_ON ON CACHE BOOL "Use OpenCL if available")
set(ICL_XDEP_OPENNI_ON ON CACHE BOOL "Use OpenNI if available")
set(ICL_XDEP_EIGEN3_ON ON CACHE BOOL "Use Eigen3 if available")
set(ICL_XDEP_PCL_ON ON CACHE BOOL "Use PCL if available")

set(ICL_XDEP_RSB_ON ON CACHE BOOL "Use Robotics Service Bus if available")
set(ICL_XDEP_PROTOBUF_ON ON CACHE BOOL "Use Google Protocol Buffers if available (necessary for RSB)")


FIND_PACKAGE(PkgConfig)
include(CheckLibraryExists)

#####VIDEODEV#######
if(UNIX)
  include(cmake_files/linux/find_VIDEODEV.cmake)
  include(cmake_files/linux/find_IPP.cmake)
  include(cmake_files/linux/find_MKL.cmake)
  include(cmake_files/linux/find_OPENCV.cmake)
  include(cmake_files/linux/find_LIBZ.cmake)
  include(cmake_files/linux/find_LIBPNG.cmake)
  include(cmake_files/linux/find_LIBFREENECT.cmake)
  include(cmake_files/linux/find_LIBMESASR.cmake)
  include(cmake_files/linux/find_LIBJPEG.cmake)
  include(cmake_files/linux/find_IMAGEMAGICK.cmake)
  include(cmake_files/linux/find_LIBDC.cmake)
  include(cmake_files/linux/find_UNICAP.cmake)
  include(cmake_files/linux/find_XINE.cmake)
  include(cmake_files/linux/find_XCF.cmake)
  include(cmake_files/linux/find_OPENCL.cmake)
  include(cmake_files/linux/find_OPENGL.cmake)
  include(cmake_files/linux/find_GENICAM.cmake)
  include(cmake_files/linux/find_OPENNI.cmake)
  include(cmake_files/linux/find_GTEST.cmake)
  include(cmake_files/linux/find_RSB_AND_PROTOBUF.cmake)
  include(cmake_files/linux/find_EIGEN3.cmake)

  if(HAVE_EIGEN3_COND)
    include(cmake_files/linux/find_PCL.cmake)
  elseif(ICL_XDEP_PCL_ON)
    message(STATUS "PCL dependency was deactivated due to the missing EIGEN3 dependency")
    message(STATUS "PCL detected: FALSE")
  endif()
  
  if(HAVE_OPENGL_COND)
    include(cmake_files/linux/find_QT.cmake)
  elseif(ICL_XDEP_QT_ON)
    message(STATUS "QT dependency was deactivated due to the missing OPENGL dependency")
    message(STATUS "QT detected: FALSE")
  endif()
  
  if(HAVE_OPENCV_COND)
    include(cmake_files/linux/find_OPENSURF.cmake)
  else(ICL_XDEP_OPENSURF_ON)
    message(STATUS "OPENSURF dependency was deactivated due to the missing OPENCV dependency")
    message(STATUS "OPENSURF detected: FALSE")
  endif()

  if(HAVE_GENICAM_COND)
    include(cmake_files/linux/find_PYLON.cmake)
  else(ICL_XDEP_PYLON_ON)
    message(STATUS "PYLON dependency was deactivated due to the missing GENICAM dependency")
    message(STATUS "PYLON detected: FALSE")
  endif()
    
endif()

if(APPLE)
  include(cmake_files/apple/find_IPP.cmake)
  include(cmake_files/apple/find_MKL.cmake)
  include(cmake_files/apple/find_LIBZ.cmake)
  include(cmake_files/apple/find_LIBJPEG.cmake)
  include(cmake_files/apple/find_IMAGEMAGICK.cmake)
  include(cmake_files/apple/find_LIBDC.cmake)
  include(cmake_files/apple/find_OPENGL.cmake)
  include(cmake_files/apple/find_GTEST.cmake)
  if(HAVE_OPENGL_COND)
    include(cmake_files/apple/find_QT.cmake)
  else()
    message(STATUS "QT dependency was deactivated due to the missing OpenGL dependency")
    message(STATUS "QT detected: FALSE")
  endif()
endif()


#add include directories of ICL
include_directories(${PROJECT_SOURCE_DIR}/include)

#check for doxygen in order to build the documentation of ICL
include(UseDoxygen OPTIONAL)
FIND_PACKAGE(Doxygen)

#currently we just write the version number of the ICL to this header file
configure_file(${PROJECT_SOURCE_DIR}/include/ICLUtils/config.h.cmake ${PROJECT_SOURCE_DIR}/include/ICLUtils/config.h @only)


#set the internal and external deps of the ICL
#if an entry is not available, it will be empty
set(ICLUtils_external_dependencies IPP MKL OPENCL)
set(ICLMath_external_dependencies EIGEN3)
set(ICLCore_external_dependencies OPENCV)
set(ICLFilter_external_dependencies "")
set(ICLIO_external_dependencies LIBZ LIBJPEG LIBMESASR LIBDC XINE UNICAP IMAGEMAGICK XCF VIDEODEV VIDEODEV2 QT LIBFREENECT LIBPNG PYLON GENICAM OPENNI RSB PROTOBUF)
set(ICLQt_external_dependencies QT OPENGL GLX)
set(ICLGeom_external_dependencies PCL)
set(ICLCV_external_dependencies OPENSURF)
set(ICLMarkers_external_dependencies "")

# internal dependency tree
set(ICLUtils_internal_dependencies "")
set(ICLMath_internal_dependencies ICLUtils)
set(ICLCore_internal_dependencies ICLMath)
set(ICLFilter_internal_dependencies ICLCore)
set(ICLQt_internal_dependencies ICLCore) # todo: do some have_qt switches ...
set(ICLIO_internal_dependencies ICLFilter)
set(ICLGeom_internal_dependencies ICLQt)
set(ICLCV_internal_dependencies ICLGeom ICLIO)
set(ICLMarkers_internal_dependencies ICLCV)


#resolve internal dependencies
add_internal_dependencies(ICLMath_internal_dependencies ICLMath_dependencies)
add_internal_dependencies(ICLCore_internal_dependencies ICLCore_dependencies)
add_internal_dependencies(ICLFilter_internal_dependencies ICLFilter_dependencies)
add_internal_dependencies(ICLCV_internal_dependencies ICLCV_dependencies)
add_internal_dependencies(ICLIO_internal_dependencies ICLIO_dependencies)
add_internal_dependencies(ICLQt_internal_dependencies ICLQt_dependencies)
add_internal_dependencies(ICLGeom_internal_dependencies ICLGeom_dependencies)
add_internal_dependencies(ICLMarkers_internal_dependencies ICLMarkers_dependencies)
#resolve external dependencies
add_external_dependencies(ICLUtils_external_dependencies ICLUtils_dependencies)
add_external_dependencies(ICLMath_external_dependencies ICLMath_dependencies)
add_external_dependencies(ICLCore_external_dependencies ICLCore_dependencies)
add_external_dependencies(ICLIO_external_dependencies ICLIO_dependencies)
add_external_dependencies(ICLQt_external_dependencies ICLQt_dependencies)
add_external_dependencies(ICLGeom_external_dependencies ICLGeom_dependencies)
add_external_dependencies(ICLCV_external_dependencies ICLCV_dependencies)

#add subdirs
add_subdirectory(ICLUtils)
add_subdirectory(ICLMath)
add_subdirectory(ICLCore)
add_subdirectory(ICLIO)
add_subdirectory(ICLFilter)
if(${HAVE_QT_COND})
    add_subdirectory(ICLQt)
endif()
add_subdirectory(ICLCV)
add_subdirectory(ICLGeom)
add_subdirectory(ICLMarkers)

#installs include files of the project on make install
install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icl-${VERSION}${DEBUG_POSTFIX}
	FILE_PERMISSIONS OWNER_WRITE OWNER_READ
		GROUP_WRITE GROUP_READ 
		WORLD_READ
	DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
		GROUP_WRITE GROUP_READ GROUP_EXECUTE
		WORLD_READ WORLD_EXECUTE
	COMPONENT hdrs	
	FILES_MATCHING PATTERN "*.h"
	REGEX .svn* EXCLUDE
)

set(ICL_RESOURCE_DIR_PLACEHOLDER "${CMAKE_INSTALL_PREFIX}/etc/icl-${VERSION}${DEBUG_POSTFIX}")
set(ICL_PKG_CFG_PLACEHOLDER "icl-${VERSION}${DEBUG_POSTFIX}")
#prepare files from scripts dir for installation
configure_file(${CMAKE_SOURCE_DIR}/scripts/icl-create-project ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/icl-create-project-data/Makefile.custom.template ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom.template @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/icl-make ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-make @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/icl-version ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-version @ONLY)


#installs files from scripts dir on make install with given permissions
install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-make ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-version
	PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	DESTINATION ${EXECUTABLE_OUTPUT_PATH}
	COMPONENT filess
)
#install scripts/icl-create-project-data/ to etc
install(DIRECTORY ${CMAKE_SOURCE_DIR}/scripts/icl-create-project-data/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/${ETC_DIR}
	FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_WRITE GROUP_READ 
		WORLD_READ WORLD_EXECUTE
	DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_WRITE GROUP_READ 
		WORLD_READ WORLD_EXECUTE
	COMPONENT filess
	FILES_MATCHING PATTERN "*.template"
	REGEX .svn* EXCLUDE
)
# this will overwrite the Makefile.custom.template with the configured one (where ICL_PKG_CFG_PLACEHOLDER is substituted)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/icl-create-project-data/Makefile.custom.template 
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${ETC_DIR}/
        COMPONENT filess
)
if(DOXYGEN_FOUND)
#
install(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/
		DESTINATION ${CMAKE_INSTALL_PREFIX}/${DOC_DIR}/ICL
		FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		COMPONENT docu
		PATTERN "*template" EXCLUDE
        REGEX .svn* EXCLUDE
		REGEX .css EXCLUDE)
#rename menuStyle.css to style.css for menu with border
install(FILES ${CMAKE_SOURCE_DIR}/doc/menuStyle.css
		DESTINATION ${CMAKE_INSTALL_PREFIX}/${DOC_DIR}/ICL
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_WRITE GROUP_READ 
		WORLD_READ WORLD_EXECUTE
		COMPONENT docu
		RENAME style.css)

#create dir for alldocs
exec_program(mkdir ARGS -p "\"${CMAKE_CURRENT_BINARY_DIR}/alldoc/html\"")

#the replace entry for menu.html
set(ENTRY "<tr><td> <a href=\"../ALL/index.html\" target=\"MainFrame\">All Packages</a></td>\n")
set(INPUT_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/alldoc")
# prepare doxygen configuration file
set(ICL_SUB_PACKAGES ICLUtils ICLCore ICLFilter ICLCV ICLIO ICLQt ICLQuick ICLGeom ICLMarkers ICLMath)
foreach(ICL_SUB_PACKAGE_PLACEHOLDER ${ICL_SUB_PACKAGES})
	#replacement in doxyfile, DO NOT EDIT
	set(INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/${ICL_SUB_PACKAGE_PLACEHOLDER}")
	#replacement in doxyfile for alldoc
	set(INPUT_PATHS "${INPUT_PATHS} ${INPUT_PATH}")    	
	#replacement in doxyfile, DO NOT EDIT	
	set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/html")
	#prepares doxyfile for doxygen
	configure_file(doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/doxyfile @ONLY)
	#replacement for menu.html
	set(ENTRY "${ENTRY}<tr><td> <a href=\"../${ICL_SUB_PACKAGE_PLACEHOLDER}/index.html\" target=\"MainFrame\">${ICL_SUB_PACKAGE_PLACEHOLDER}</a></td>\n")
    #copy pics
	file(COPY ${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		FILES_MATCHING 
		PATTERN "*.png" 
		PATTERN "*.jpg"
		PATTERN ".svn*" EXCLUDE)
	#create directories in each doc dir of each subproject
    exec_program(mkdir ARGS -p "\"${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/html/\"")
endforeach()
#configure file for the menu
configure_file(doc/menu.htmltemplate ${CMAKE_SOURCE_DIR}/doc/menu.html @ONLY)
configure_file(doc/index.html.in ${CMAKE_SOURCE_DIR}/doc/index.html @ONLY)
configure_file(doc/top.html.in ${CMAKE_SOURCE_DIR}/doc/top.html @ONLY)

#copy necessary files to build dir
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/alldoc/
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/alldoc/html
		FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE		
		FILES_MATCHING 
		PATTERN "*.html"
		PATTERN ".svn*" EXCLUDE)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/alldoc/
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/alldoc/html
		FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		FILES_MATCHING
		PATTERN "*.png"
		PATTERN ".svn*" EXCLUDE)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc/
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/alldoc/html
		FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_WRITE GROUP_READ 
			WORLD_READ WORLD_EXECUTE
		FILES_MATCHING 
		PATTERN "*.gif"
		PATTERN ".svn*" EXCLUDE)

#output path for generated files on make alldoc
set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/alldoc/html")
#all pathes to all subdir of project
set(INPUT_PATH "${INPUT_PATHS}")
#prepares doxyfile for doxygen
configure_file(doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/alldoc/doxyfile @ONLY)

#adds custom target alldoc to  makefile
#if(APPLE)
#add_custom_target(alldoc /Applications/Doxygen.app/Contents/Resources/doxygen alldoc/doxyfile)
#else()
add_custom_target(alldoc doxygen alldoc/doxyfile)
#endif()
#installs content of alldoc/html on make install
#excludes .svn dir and doxyfile from installation

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/alldoc/html/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${DOC_DIR}/ALL
		FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	  		GROUP_EXECUTE GROUP_WRITE GROUP_READ 
	  		WORLD_READ WORLD_EXECUTE	  
		DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	  		GROUP_EXECUTE GROUP_WRITE GROUP_READ 
	  		WORLD_READ WORLD_EXECUTE
	  COMPONENT docu
      PATTERN "doxyfile" EXCLUDE
      REGEX .svn* EXCLUDE)

#remove build docs from alldoc on make clean
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES alldoc/html/.)

get_target_property(DOC_TARGET doc TYPE)
if(NOT DOC_TARGET)
	#if(APPLE)
	#add_custom_target(doc /Applications/Doxygen.app/Contents/Resources/doxygen doxyfile)
	#else()
	add_custom_target(doc)
	#endif()
endif()
endif()

#prepare pkg-config files if we are on linux system
if(SYSTEM_LINUX_COND OR APPLE)
  set(ALWAYS_TRUE TRUE)
  #replacement in pkg.in DO NOT EDIT
  set(lprefix      ${CMAKE_INSTALL_PREFIX})

  icl_create_pkg_config_file_if(ICLUtils ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLCore ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLFilter ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLCV ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLIO ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLQt HAVE_QT_COND)
  icl_create_pkg_config_file_if(ICLQuick ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLGeom ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLMath ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLMarkers ALWAYS_TRUE)
    
  set(ICL_SUB_PACKAGE "icl-${VERSION}${DEBUG_POSTFIX}")
  set(LIBS "")
  set(INCLUDES "")
  set(REQUIRE "ICLUtils-${VERSION}${DEBUG_POSTFIX} ICLCore-${VERSION}${DEBUG_POSTFIX} ICLIO-${VERSION}${DEBUG_POSTFIX} ICLCV-${VERSION}${DEBUG_POSTFIX} ICLFilter-${VERSION}${DEBUG_POSTFIX} ICLQuick-${VERSION}${DEBUG_POSTFIX} ICLGeom-${VERSION}${DEBUG_POSTFIX} ICLMath-${VERSION}${DEBUG_POSTFIX} ICLMarkers-${VERSION}${DEBUG_POSTFIX}")
if(HAVE_QT_COND)
  set(REQUIRE "${REQUIRE} ICLQt-${VERSION}${DEBUG_POSTFIX}")
endif()
  set(ICL_PACKAGE_DESCRIPTION "Image Component Library (ICL)")
  configure_file(pkg.in ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE_NAME} @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE_NAME}" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/ 
				PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ 
				WORLD_READ WORLD_EXECUTE COMPONENT filess)
endif()


# if this is the last sub-dir, it is exectued at the end
# the scripts/CMakeLists does only contain the
# install(SCRIPT ./post_install.cmake)
# line ...


add_subdirectory(scripts)


## uninstall 
set(P ${CMAKE_INSTALL_PREFIX})
set(D icl-${VERSION}${DEBUG_POSTFIX})
add_custom_target(uninstall
  COMMAND echo "calling rm -rf ${P}/bin/${D} ..."
  COMMAND rm -rf ${P}/bin/${D}
  COMMAND echo "calling rm -rf ${P}/lib/${D} ..."
  COMMAND rm -rf ${P}/lib/${D}
  COMMAND echo "calling rm -rf ${P}/doc/${D} ..."
  COMMAND rm -rf ${P}/doc/${D}
  COMMAND echo "calling rm -rf ${P}/include/${D} ..."
  COMMAND rm -rf ${P}/include/${D}
  COMMAND echo "calling rm -rf ${P}/etc/${D} ..."
  COMMAND rm -rf ${P}/etc/${D})



macro(add_deb_dependency WHAT WHEN)
  if(${WHEN})
    message(STATUS "adding CPack dependency ${WHAT}")
    if(CPACK_DEBIAN_PACKAGE_DEPENDS)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${WHAT}")
    else()
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${WHAT}")
    endif()
  endif()
endmacro()
#############################################



#in shell type: cpack
#to build automatically the deb package
set(CPACK_PACKAGE_DESCRIPTION "The Image Component Library (ICL) is a C++ computer vision library. ICL does not only focus on providing computer vision related tools, but it also provides a well chosen set of support functionalities.  ")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Image Component Library (ICL)")
set(CPACK_PACKAGE_NAME "icl")

message(STATUS "CPack configuring dependencies")
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.6), libgcc1 (>= 1:4.1)")
add_deb_dependency(libdc1394-22-dev HAVE_LIBDC_COND)
add_deb_dependency(libmagick++-dev HAVE_IMAGEMAGICK_COND)
add_deb_dependency(libunicap2-dev HAVE_UNICAP_COND)
add_deb_dependency(libqt4-dev HAVE_QT_COND)
add_deb_dependency(libjpeg-dev HAVE_LIBJPEG_COND)
add_deb_dependency(zlib1g-dev HAVE_LIBZ_COND)
add_deb_dependency(libpng12-dev HAVE_LIBPNG_COND)
add_deb_dependency(libv4l-dev HAVE_VIDEODEV_COND)
add_deb_dependency(libeigen3-dev HAVE_EIGEN3_COND)

set(CPACK_PACKAGE_CONTACT "nivision@techfak.uni-bielefeld.de")
set(CPACK_PACKAGE_VENDOR "CITEC, Bielefeld University, Neuroinformatics Group")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Christof Elbrechter")
set(CPACK_PACKAGE_VERSION_MAJOR "${ICL_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ICL_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${ICL_PATCH}")

set(CPACK_PACKAGE_EXECUTABLES "")
set(CPACK_COMPONENTS_ALL filess)
set(CPACK_ALL_INSTALL_TYPES Full)
set(CPACK_GENERATOR "DEB")
set(CPACK_SET_DESTDIR "ON")
#OLD ?? set(CPACK_INSTALL_CMAKE_PROJECTS "headers")
#set(CPACK_INSTALL_CMAKE_PROJECTS "/homes/celbrech/projects/ICL;ICL;ALL;/")
message(STATUS "ignore ${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_SOURCE_IGNORE_FILES "\\\\doxyfile$")
set(CPACK_INSTALLED_DIRECTORIES "")
#foreach(ICL_SUB_PACKAGE_PLACEHOLDER ${ICL_SUB_PACKAGES})
#TODO set ignore file for doxyfile and install doc dir
#  set(CPACK_INSTALLED_DIRECTORIES "${CPACK_INSTALLED_DIRECTORIES};${CMAKE_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/html;${CMAKE_INSTALL_PREFIX}/doc/${ICL_SUB_PACKAGE_PLACEHOLDER}")
#endforeach()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_INSTALL_PREFIX "/tmp/")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
set(CPACK_STRIP_FILES "ICLUtils/lib/ICLUtils.so" "ICLCore/lib/ICLCore.so" "ICLFilter/lib/ICLFilter.so" "ICLCV/lib/ICLCV.so" "ICLIO/lib/ICLIO.so" "ICLQt/lib/ICLQt.so" "ICLQuick/lib/ICLQuick.so" "ICLGeom/lib/ICLGeom.so" "ICLMath/lib/ICLMath.so" "ICLMarkers/lib/ICLMarkers.so")
#SET(CPACK_SYSTEM_NAME "Linux-i686")
#SET(CPACK_TOPLEVEL_TAG "Linux-i686")

include(CPack)

