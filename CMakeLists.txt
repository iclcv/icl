CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1)
PROJECT(ICL)
#ubuntu version
set(ICL_MAJOR "5")
set(ICL_MINOR "2")
set(ICL_PATCH "1")
set(VERSION "${ICL_MAJOR}.${ICL_MINOR}.${ICL_PATCH}")
cmake_policy(SET CMP0002 OLD)
#cmake_policy(SET CMP0015 OLD)

set(ICL_VAR_DEBUG_MODE OFF CACHE BOOL "Compile with debug options")
if(ICL_VAR_DEBUG_MODE)
set(CMAKE_CXX_FLAGS "-Wall -O0 -g3")
set(CMAKE_BUILD_TYPE "Debug" CACHE INTERNAL "")
else()
set(CMAKE_CXX_FLAGS "-O4 -funroll-loops -march=native")
set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_VERBOSE_MAKEFILE ON)
#set(ICL_VERBOSE_MAKEFILE OFF CACHE BOOL "Compile with more output")
#if(ICL_VERBOSE_MAKEFILE)
#message(STATUS "blub")
#	set(CMAKE_VERBOSE_MAKEFILE ON)
#else()
#	set(CMAKE_VERBOSE_MAKEFILE OFF)
#endif()
message(STATUS "Verbose makefiles: ${CMAKE_VERBOSE_MAKEFILE}")

set(ICL_VAR_INSTALL_PREFIX "/temp/testICL${VERSION}" CACHE PATH "")
set(CMAKE_INSTALL_PREFIX "${ICL_VAR_INSTALL_PREFIX}" CACHE INTERNAL "")
message(STATUS "Install Path: ${CMAKE_INSTALL_PREFIX}")

if(UNIX)
    add_definitions( -DSYSTEM_LINUX)
    add_definitions( -DICL_SYSTEM_LINUX)
    set(SYSTEM_LINUX_COND TRUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	# use, i.e. don't skip the full RPATH for the build tree
	SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

	# when building, don't use the install RPATH already
	# (but later on when installing)
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 


	# the RPATH to be used when installing
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

	# add the automatically determined parts of the RPATH
	# which point to directories outside the build tree to the install RPATH
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()
if(APPLE)
    add_definitions( -DSYSTEM_APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
if(WIN32)
    add_definitions( -DSYSTEM_WINDOWS)
    add_definitions( -DICL_SYSTEM_WINDOWS)
endif()

#include file with macros
include(icl_cmake_macros.cmake)

set(ICL_XDEP_ALL_ON OFF CACHE BOOL "Use all available third party libs")
set(ICL_VAR_USE_PKG_CONFIG OFF CACHE BOOL "Use all available pkgconfig")
set(ICL_VAR_BUILD_EXAMPLES ON CACHE BOOL "Build example applications")
set(LIBRARY_OUTPUT_PATH "lib")# CACHE PATH "Output directory for libraries" )
set(EXECUTABLE_OUTPUT_PATH "bin")# CACHE PATH "Output directory for applications" )

set(ICL_XDEP_IPP_ON OFF CACHE BOOL "Use IPP when available")
set(ICL_XDEP_MKL_ON OFF CACHE BOOL "Use MKL when available")
set(ICL_XDEP_OPENCV_ON OFF CACHE BOOL "Use OPENCV when available")
set(ICL_XDEP_LIBZ_ON OFF CACHE BOOL "Use LIBZ when available")
set(ICL_XDEP_LIBMESASR_ON OFF CACHE BOOL "Use LIBMESASR when available")
set(ICL_XDEP_LIBJPEG_ON OFF CACHE BOOL "Use LIBJPEG when available")
set(ICL_XDEP_GRAPHICSMAGICK_ON OFF CACHE BOOL "Use GRAPHICSMAGICK when available")
set(ICL_XDEP_LIBDC_ON OFF CACHE BOOL "Use LIBDC when available")
set(ICL_XDEP_UNICAP_ON OFF CACHE BOOL "Use UNICAP when available")
set(ICL_XDEP_XINE_ON OFF CACHE BOOL "Use XINE when available")
set(ICL_XDEP_XCF_ON OFF CACHE BOOL "Use XCF when available")
set(ICL_XDEP_QT_ON OFF CACHE BOOL "Use QT when available")
set(ICL_XDEP_OPENGL_ON OFF CACHE BOOL "Use OPENGL when available")
set(ICL_XDEP_SVS_ON OFF CACHE BOOL "Use SVS when available")
set(ICL_XDEP_OPENSURF_ON OFF CACHE BOOL "Use OPENSURF when available")
set(ICL_XDEP_GTEST_ON OFF CACHE BOOL "Use GTest when available")

FIND_PACKAGE(PkgConfig)

icl_check_external_package(IPP ipp.h sharedlib include /vol/nivision/IPP/6.1-ia32 HAVE_IPP_COND)
if(HAVE_IPP_COND)
    set(IPP_LIBS_l ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()

icl_check_external_package(MKL mkl.h lib/32 include /vol/nivision/MKL/10.2 HAVE_MKL_COND)
if(HAVE_MKL_COND)
    set(MKL_LIBS_l mkl_intel mkl_intel_thread mkl_core iomp5)
endif()

####OPENCV####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(OPENCV opencv HAVE_OPENCV_COND)
endif()
if(NOT OPENCV_PKG_FOUND)
  icl_check_external_package(OPENCV cxtypes.h lib include/opencv /opt/OpenCV2 HAVE_OPENCV_COND)
  if(HAVE_OPENCV_COND)
	set(OPENCV_LIBS_l cxcore cv highgui cvaux ml)
   endif()
endif()

icl_check_external_package(LIBZ zlib.h lib include /usr HAVE_LIBZ_COND)
if(HAVE_LIBZ_COND)
	set(LIBZ_LIBS_l z)
endif()

icl_check_external_package(LIBMESASR libMesaSR.h lib include /usr HAVE_LIBMESASR_COND)
if(HAVE_LIBMESASR_COND)
	set(LIBMESASR_LIBS_l mesasr)
endif()

icl_check_external_package(LIBJPEG jpeglib.h lib include /usr HAVE_LIBJPEG_COND)
if(HAVE_LIBJPEG_COND)
	set(LIBJPEG_LIBS_l jpeg)
endif()

####GRAPHICSMAGICK####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(GRAPHICSMAGICK GraphicsMagick++ HAVE_GRAPHICSMAGICK_COND)
endif()
if(NOT GRAPHICSMAGICK_PKG_FOUND)
  icl_check_external_package(GRAPHICSMAGICK Magick++.h lib include/GraphicsMagick /usr HAVE_GRAPHICSMAGICK_COND)
  if(HAVE_GRAPHICSMAGICK_COND)
	set(GRAPHICSMAGICK_LIBS_l GraphicsMagick++)
   endif()
endif()

####IMAGEMAGICK####
#icl_check_external_package(IMAGEMAGICK Magick++.h lib include/ImageMagick /usr HAVE_IMAGEMAGICK_COND)
#if(HAVE_IMAGEMAGICK_COND)
#	 	set(IMAGEMAGICK_LIBS_l Magick++)
#endif()


####LIBDC####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(LIBDC "libdc1394-2" HAVE_LIBDC_COND)
endif()
if(NOT LIBDC_PKG_FOUND)
  icl_check_external_package(LIBDC dc1394.h lib include/dc1394 /usr HAVE_LIBDC_COND)
  if(HAVE_LIBDC_COND)
	set(LIBDC_LIBS_l dc1394)
   endif()
endif()

####UNICAP####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(UNICAP libunicap HAVE_UNICAP_COND)
	#additional definition
	add_definitions(-DUNICAP_FLAGS_NOT_AS_ENUM)
endif()
if(NOT UNICAP_PKG_FOUND)
  icl_check_external_package(UNICAP unicap.h lib include/unicap /usr HAVE_UNICAP_COND)
  if(HAVE_UNICAP_COND)
	set(UNICAP_LIBS_l unicap rt)
	#additional definition
	add_definitions(-DUNICAP_FLAGS_NOT_AS_ENUM)
   endif()
endif()

####XINE####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(XINE libxine HAVE_XINE_COND)
endif()
if(NOT XINE_PKG_FOUND)
  icl_check_external_package(XINE xineutils.h lib include/xine /usr HAVE_XINE_COND)
  if(HAVE_XINE_COND)
	set(XINE_LIBS_l xine)
   endif()
endif()

####XCF####
if(ICL_VAR_USE_PKG_CONFIG)
	#icl_check_external_package_from_pkg_conf(XCF "xcf;xmltio;Memory" HAVE_XCF_COND)
endif()
if(NOT XCF_PKG_FOUND)
  icl_check_external_package(XCF xcf/XCFBase.hpp lib include /opt/xcf HAVE_XCF_COND)
  if(HAVE_XCF_COND)
	set(XCF_LIBS_l xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
   endif()
endif()

#icl_check_external_package(XCF xcf/XCFBase.hpp lib include /opt/xcf HAVE_XCF_COND)
#if(HAVE_XCF_COND)
#	if(${ICL_VAR_USE_PKGCONF}_)
#		icl_define_variables_from_pkg_config(XCF xcf;xmltio;Memory)
#		pkg_check_modules(XCF_PKG xcf)		
#		pkg_check_modules(XCF_XMLTIO_PKG xmltio)
#		pkg_check_modules(XCF_MEMORY_PKG Memory)		
#		if(${XCF_PKG_FOUND} AND ${XCF_MEMORY_PKG_FOUND} AND ${XCF_XMLTIO_PKG_FOUND})
#			set(XCF_LIBS_l "${XCF_PKG_LIBRARIES};${XCF_MEMORY_PKG_LIBRARIES};${XCF_XMLTIO_PKG_LIBRARIES}")
#			list(LENGTH XCF_LIBS_l listsize)
#			if(${listsize} GREATER 1)
#				list(REMOVE_DUPLICATES XCF_LIBS_l)
#				list(REMOVE_ITEM XCF_LIBS_l pthread)
#			endif()
#		else()
#			set(XCF_LIBS_l xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
#		endif()		
#    else()
#		set(XCF_LIBS_l xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
#              endif()
	#additional include path
	#include_directories("${XCF_PATH}/include")
#endif()

icl_check_external_package(QT QtGui/QApplication lib include /opt/qtsdk-2009.04/qt HAVE_QT_COND)
if(NOT HAVE_QT_COND)
  icl_check_external_package(QT QtGui/QApplication lib include/qt4 /usr HAVE_QT_COND)
endif()
if(HAVE_QT_COND)
	if(${ICL_VAR_USE_PKGCONF}_)
		pkg_check_modules(QtCore_PKG QtCore)
		pkg_check_modules(QtGui_PKG QtGui)
		pkg_check_modules(QtOpenGL_PKG QtOpenGL)
		if(${QtCore_PKG_FOUND} AND ${QtGui_PKG_FOUND} AND ${QtOpenGL_PKG_FOUND})
			set(QT_LIBS_l "${QtCore_PKG_LIBRARIES};${QtGui_PKG_LIBRARIES};${QtOpenGL_PKG_LIBRARIES}")
		else()
			set(QT_LIBS_l QtCore QtGui QtOpenGL)
		endif()
    else()
		set(QT_LIBS_l QtCore QtGui QtOpenGL)
    endif()
endif()

icl_check_external_package(OPENGL gl.h lib include/GL /usr HAVE_OPENGL_COND)
if(${HAVE_OPENGL_COND})
	set(OPENGL_LIBS_l GL GLU)
endif()

icl_check_external_package(SVS dcam.h bin src /vol/nivision/SVS/4.2 HAVE_SVS_COND)
if(HAVE_SVS_COND)
	set(SVS_LIBS_l svs svscalc)
endif()

####OPENSURF####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(OPENSURF opensurf HAVE_OPENSURF_COND)
endif()
if(NOT OPENSURF_PKG_FOUND)
  icl_check_external_package(OPENSURF opensurf/surf.h lib include /vol/nivision HAVE_OPENSURF_COND)
  if(HAVE_OPENSURF_COND)
	set(OPENSURF_LIBS_l opensurf)
   endif()
endif()

icl_check_external_package(GTEST gtest/gtest.h lib include /vol/nivision/gtest HAVE_GTEST_COND)
if(HAVE_GTEST_COND)
	set(GTEST_LIBS_l gtest)
endif()


include_directories(${PROJECT_SOURCE_DIR}/include)
#set(incl)
#GET_DIRECTORY_PROPERTY(incl INCLUDE_DIRECTORIES)
#message(STATUS "iiiii: ${incl}")
include(UseDoxygen OPTIONAL)
FIND_PACKAGE(Doxygen)

configure_file(${PROJECT_SOURCE_DIR}/include/ICLUtils/config.h.cmake ${PROJECT_SOURCE_DIR}/include/ICLUtils/config.h @only)

set(ICLUtils_external_dependencies IPP MKL)
set(ICLCore_internal_dependencies ICLUtils)
set(ICLCC_internal_dependencies ICLCore)
set(ICLFilter_internal_dependencies ICLCC)
set(ICLBlob_internal_dependencies ICLCC)
if(${HAVE_OPENCV_COND})
    set(ICLOpenCV_internal_dependencies ICLCore)
    set(ICLOpenCV_external_dependencies OPENCV)
    set(ICLIO_internal_dependencies ICLOpenCV ICLFilter)
    set(ICLAlgorithms_internal_dependencies ICLOpenCV ICLQuick ICLGeom)
else()
    set(ICLIO_internal_dependencies ICLFilter)
    set(ICLAlgorithms_internal_dependencies ICLQuick ICLGeom)
endif()
set(ICLIO_external_dependencies LIBZ LIBJPEG LIBMESASR LIBDC XINE UNICAP GRAPHICSMAGICK XCF)
set(ICLQt_internal_dependencies ICLFilter ICLIO)
set(ICLQt_external_dependencies QT OPENGL)
if(${HAVE_QT_COND})
    set(ICLQuick_internal_dependencies ICLQt ICLFilter ICLIO ICLBlob)
else()
    set(ICLQuick_internal_dependencies ICLFilter ICLIO ICLBlob)
endif()
set(ICLGeom_internal_dependencies ICLQuick)

set(ICLAlgorithms_external_dependencies OPENSURF SVS)

add_internal_dependencies(ICLCore_internal_dependencies ICLCore_dependencies)
add_internal_dependencies(ICLCC_internal_dependencies ICLCC_dependencies)
add_internal_dependencies(ICLFilter_internal_dependencies ICLFilter_dependencies)
add_internal_dependencies(ICLBlob_internal_dependencies ICLBlob_dependencies)
add_internal_dependencies(ICLOpenCV_internal_dependencies ICLOpenCV_dependencies)
add_internal_dependencies(ICLIO_internal_dependencies ICLIO_dependencies)
add_internal_dependencies(ICLQt_internal_dependencies ICLQt_dependencies)
add_internal_dependencies(ICLQuick_internal_dependencies ICLQuick_dependencies)
add_internal_dependencies(ICLGeom_internal_dependencies ICLGeom_dependencies)
add_internal_dependencies(ICLAlgorithms_internal_dependencies ICLAlgorithms_dependencies)

add_external_dependencies(ICLUtils_external_dependencies ICLUtils_dependencies)
add_external_dependencies(ICLOpenCV_external_dependencies ICLOpenCV_dependencies)
add_external_dependencies(ICLIO_external_dependencies ICLIO_dependencies)
add_external_dependencies(ICLQt_external_dependencies ICLQt_dependencies)
add_external_dependencies(ICLAlgorithms_external_dependencies ICLAlgorithms_dependencies)

#add subdirs
add_subdirectory(ICLUtils)
add_subdirectory(ICLCore)
#what about the dir located in ../examples
add_subdirectory(ICLCC)
add_subdirectory(ICLFilter)
add_subdirectory(ICLBlob)
if(${HAVE_OPENCV_COND})
    add_subdirectory(ICLOpenCV)
endif()
add_subdirectory(ICLIO)
if(${HAVE_QT_COND})
    add_subdirectory(ICLQt)
endif()
add_subdirectory(ICLQuick)
add_subdirectory(ICLGeom)
add_subdirectory(ICLAlgorithms)

install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ICL
	FILES_MATCHING PATTERN "*.h"
	REGEX .svn* EXCLUDE
)

configure_file(${CMAKE_SOURCE_DIR}/scripts/icl-create-project-with-tests ${CMAKE_CURRENT_BINARY_DIR}/icl-create-project-with-tests @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/icl-create-project ${CMAKE_CURRENT_BINARY_DIR}/icl-create-project @ONLY)

install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/icl-create-project-with-tests ${CMAKE_CURRENT_BINARY_DIR}/icl-create-project ${CMAKE_SOURCE_DIR}/scripts/icl-create-project-with-tests ${CMAKE_SOURCE_DIR}/scripts/icl-make
	PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/scripts/icl-create-project-data/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/etc
	FILES_MATCHING PATTERN "*.template"
	REGEX .svn* EXCLUDE
)

if(DOXYGEN_FOUND)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/
         DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/ICL
)

# prepare doxygen configuration file
set(ICL_SUB_PACKAGE_PLACEHOLDER2 ICLUtils ICLCore ICLCC ICLFilter ICLBlob ICLOpenCV ICLIO ICLQt ICLQuick ICLGeom ICLAlgorithms)
foreach(ICL_SUB_PACKAGE_PLACEHOLDER ${ICL_SUB_PACKAGE_PLACEHOLDER2})
    	configure_file(doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/doxyfile @ONLY)
    	#copy pics
	file(COPY ${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc
		FILES_MATCHING 
		PATTERN "*.png" 
		PATTERN "*.jpg")
	exec_program(mkdir ARGS -p "\"${CMAKE_CURRENT_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/html/\"")
endforeach()

# add doxygen as dependency to doc-target
get_target_property(DOC_TARGET doc TYPE)
if(NOT DOC_TARGET)
	add_custom_target(doc)
endif()
endif()


if(SYSTEM_LINUX_COND)
  set(ALWAYS_TRUE TRUE)
  set(lprefix      ${CMAKE_INSTALL_PREFIX})

  icl_create_pkg_config_file_if(ICLUtils ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLCore ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLCC ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLFilter ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLBlob ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLOpenCV HAVE_OPENCV_COND)
  icl_create_pkg_config_file_if(ICLIO ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLQt HAVE_QT_COND)
  icl_create_pkg_config_file_if(ICLQuick ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLGeom ALWAYS_TRUE)
  icl_create_pkg_config_file_if(ICLAlgorithms ALWAYS_TRUE)
    
  set(ICL_SUB_PACKAGE "ICL")
  set(REQUIRE "ICLUtils ICLCore ICLIO ICLCC ICLBlob ICLFilter ICLOpenCV ICLQuick ICLGeom ICLAlgorithms ICLQt ICLOpenCV")
  set(ICL_PACKAGE_DESCRIPTION "Image Component Library (ICL)")
  configure_file(pkg.in ${CMAKE_CURRENT_BINARY_DIR}/icl.pc @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/icl.pc" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/)
endif()


#############################################
set(CPACK_PACKAGE_DESCRIPTION "Do the example thingie")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A longer description about our example app.")
set(CPACK_PACKAGE_NAME "ICL - Image Component Library")
#xine qt libz libjpg graphicsmagick opengl
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.6), libgcc1 (>= 1:4.1)")

set(CPACK_PACKAGE_CONTACT "wer auch immer")
set(CPACK_PACKAGE_VENDOR "uni")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)




