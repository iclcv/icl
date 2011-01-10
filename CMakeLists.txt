#*********************************************************************
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
set(ICL_MAJOR "5")
set(ICL_MINOR "12")
set(ICL_PATCH "0")
set(VERSION "${ICL_MAJOR}.${ICL_MINOR}.${ICL_PATCH}")
cmake_policy(SET CMP0002 OLD)
if(NOT ${CMAKE_PATCH_VERSION} STREQUAL "0")
	cmake_policy(SET CMP0015 OLD)
endif()
#get_cmake_property(v VARIABLES)
#foreach(vv ${v})
#message(STATUS "${vv}")
#endforeach()

# use void pointer size to check if we are building on 64bit
if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(ICL_64BIT TRUE)
    add_definitions( -DICL_64BIT)
    message("Building for 64bit")
else()
    add_definitions( -DICL_32BIT)
endif()


#build type (Debug or Release)
set(ICL_VAR_DEBUG_MODE OFF CACHE BOOL "Compile with debug options")
if(ICL_VAR_DEBUG_MODE)
set(CMAKE_CXX_FLAGS "-Wall -O0 -g3")
set(CMAKE_BUILD_TYPE "Debug" CACHE INTERNAL "")
set(DEBUG_POSTFIX "-debug")
set(PC_FILE_NAME "icl-${VERSION}-debug.pc")
else()
set(CMAKE_CXX_FLAGS "-O4 -funroll-loops -march=native")
set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "")
set(DEBUG_POSTFIX "")
set(PC_FILE_NAME "icl-${VERSION}.pc")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
#build in verbos g++ mode
if(WIN32)
set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
message(STATUS "Verbose makefiles: ${CMAKE_VERBOSE_MAKEFILE}")

#where to install ICL
if(WIN32)
set(ICL_VAR_INSTALL_PREFIX "C:/Users/lukas/temp/testICL${VERSION}" CACHE PATH "")
else()
set(ICL_VAR_INSTALL_PREFIX "/vol/nivision" CACHE PATH "")
endif()
set(CMAKE_INSTALL_PREFIX "${ICL_VAR_INSTALL_PREFIX}" CACHE INTERNAL "")
message(STATUS "Install Path: ${CMAKE_INSTALL_PREFIX}")

if(UNIX)
	#current def to indicate we are on linux
    add_definitions( -DSYSTEM_LINUX)
	#in the future the above def will be replaced by this def to achieve compatibility with mingw
    add_definitions( -DICL_SYSTEM_LINUX)
    set(SYSTEM_LINUX_COND TRUE)
	#pass -pthread directly to flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

	#runtimepath options
	# use, i.e. don't skip the full RPATH for the build tree
	SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

	# when building, don't use the install RPATH already
	# (but later on when installing)
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

	# the RPATH to be used when installing
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/icl-${VERSION}${DEBUG_POSTFIX}")

	# add the automatically determined parts of the RPATH
	# which point to directories outside the build tree to the install RPATH
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()
if(APPLE)
    add_definitions( -DSYSTEM_APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
if(WIN32)
	#current def to indicate we are on windows
    #add_definitions( -DSYSTEM_WINDOWS)
	#in the future the above def will be replaced by this def to achieve compatibility with mingw
    add_definitions( -DICL_SYSTEM_WINDOWS)
	
	#if(1)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")# -static-libgcc")
	include_directories("C:/Qt/2010.02.1/mingw/mingw32/include")
	link_directories("C:/Qt/2010.02.1/mingw/mingw32/lib")
	#else()
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthreadGC2 -static-libgcc -lgcc")
	#link_directories("C:/Qt/2010.02.1/mingw/lib/gcc/mingw32/4.4.0")
	#include_directories("C:/Users/lukas/pthread/Pre-built.2/include")
	#link_directories("C:/Users/lukas/pthread/Pre-built.2/lib")
	#endif()
	#link_directories("C:/MinGW/lib")
	
	#include_directories("C:/Qt/2010.04/mingw/mingw32/include")
	#link_directories("C:/Qt/2010.04/mingw/mingw32/lib")
	#link_directories("C:/Qt/2010.04/mingw/lib")
	#link_directories("C:/WINDOWS/system32")
endif()

#include file with macros
include(icl_cmake_macros.cmake)

#if ON all found third party libs will be used
set(ICL_XDEP_ALL_ON OFF CACHE BOOL "Use all available third party libs")
#if ON pkg-config will be used where it is available
set(ICL_VAR_USE_PKG_CONFIG OFF CACHE BOOL "Use all available pkgconfig")
#if ON examples apps will be build and install
set(ICL_VAR_BUILD_EXAMPLES ON CACHE BOOL "Build example applications")

set(LIBRARY_OUTPUT_PATH "lib/icl-${VERSION}${DEBUG_POSTFIX}")
set(EXECUTABLE_OUTPUT_PATH "bin/icl-${VERSION}${DEBUG_POSTFIX}")
set(DOC_DIR "doc/icl-${VERSION}${DEBUG_POSTFIX}")
set(ETC_DIR "etc/icl-${VERSION}${DEBUG_POSTFIX}")

#checkboxes to indicate a third party lib will be used
set(ICL_XDEP_IPP_ON OFF CACHE BOOL "Use IPP when available")
set(ICL_XDEP_MKL_ON OFF CACHE BOOL "Use MKL when available")
set(ICL_XDEP_OPENCV_ON OFF CACHE BOOL "Use OPENCV when available")
set(ICL_XDEP_LIBZ_ON OFF CACHE BOOL "Use LIBZ when available")
set(ICL_XDEP_LIBMESASR_ON OFF CACHE BOOL "Use LIBMESASR when available")
set(ICL_XDEP_LIBJPEG_ON OFF CACHE BOOL "Use LIBJPEG when available")
set(ICL_XDEP_IMAGEMAGICK_ON OFF CACHE BOOL "Use IMAGEMAGICK when available")
set(ICL_XDEP_LIBDC_ON OFF CACHE BOOL "Use LIBDC when available")
set(ICL_XDEP_LIBFREENECT_ON OFF CACHE BOOL "Use libfreenect when available")
set(ICL_XDEP_UNICAP_ON OFF CACHE BOOL "Use UNICAP when available")
set(ICL_XDEP_XINE_ON OFF CACHE BOOL "Use XINE when available")
set(ICL_XDEP_XCF_ON OFF CACHE BOOL "Use XCF when available")
set(ICL_XDEP_QT_ON OFF CACHE BOOL "Use QT when available")
set(ICL_XDEP_OPENGL_ON OFF CACHE BOOL "Use OPENGL when available")
#set(ICL_XDEP_SVS_ON OFF CACHE BOOL "Use SVS when available")
set(ICL_XDEP_OPENSURF_ON OFF CACHE BOOL "Use OPENSURF when available")
set(ICL_XDEP_GTEST_ON OFF CACHE BOOL "Use GTest when available")


FIND_PACKAGE(PkgConfig)

#TODO special check for videodev
if(EXISTS /usr/include/linux/videodev.h 
   AND EXISTS /usr/include/sys/ioctl.h 
   AND EXISTS /usr/include/sys/mman.h 
   AND EXISTS /usr/include/fcntl.h)
message(STATUS "found videodev")
set(HAVE_VIDEODEV_COND TRUE)
add_definitions( -DHAVE_VIDEODEV)
elseif()
set(HAVE_VIDEODEV_COND FALSE)
endif()

#check if and what external third party libs are available
if(ICL_64BIT)
  icl_check_external_package(IPP ipp.h "ippcore;ippi;ipps;ippsr;ippcv;ippm;ippcc;guide;iomp5" em64t/sharedlib em64t/include /vol/nivision/share/IPP/6.1-ia32 HAVE_IPP_COND)
else()
  icl_check_external_package(IPP ipp.h "ippcore;ippi;ipps;ippsr;ippcv;ippm;ippcc;guide;iomp5" sharedlib include /vol/nivision/share/IPP/6.1-ia32 HAVE_IPP_COND)
endif()
if(HAVE_IPP_COND)
    set(IPP_LIBS_l ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()

#icl_check_external_package(MKL "mkl_types.h;mkl_cblas.h" "mkl_intel;mkl_intel_thread;mkl_core;iomp5" lib/32 include /vol/nivision/MKL/10.2 HAVE_MKL_COND)
#if(HAVE_MKL_COND)
#    set(MKL_LIBS_l mkl_intel mkl_intel_thread mkl_core iomp5)
#endif()

if(ICL_64BIT)
  icl_check_external_package(MKL "mkl_types.h;mkl_cblas.h" "mkl_intel_lp64;mkl_intel_thread;mkl_core;iomp5" lib/em64t include /vol/nivision/share/MKL/10.2-ia32 HAVE_MKL_COND)
  if(HAVE_MKL_COND)
    set(MKL_LIBS_l mkl_intel_lp64 mkl_intel_thread mkl_core iomp5)
  endif()
else()  
  icl_check_external_package(MKL "mkl_types.h;mkl_cblas.h" "mkl_intel;mkl_intel_thread;mkl_core;iomp5" lib/32 include /vol/nivision/share/MKL/10.2-ia32 HAVE_MKL_COND)
  if(HAVE_MKL_COND)
     set(MKL_LIBS_l mkl_intel mkl_intel_thread mkl_core iomp5)
  endif()
 endif()


include(CheckLibraryExists)
####OPENCV#### 
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(OPENCV opencv HAVE_OPENCV_COND)
	list(FIND OPENCV_LIBS_l "ml" ML_INDEX)
	if(NOT ML_INDEX STREQUAL "-1")
		set(HAVE_OPENCV2_COND TRUE)
		add_definitions( -DHAVE_OPENCV2)
	endif()
	list(FIND OPENCV_LIBS_l "opencv_imgproc" IMGPROC_INDEX)
	if(NOT IMGPROC_INDEX STREQUAL "-1")
		set(HAVE_OPENCV2_COND TRUE)
		add_definitions( -DHAVE_OPENCV2)
	    add_definitions( -DHAVE_OPENCV211)
	endif()
endif()
if(NOT OPENCV_PKG_FOUND)
    #temporary variable for checking opencv2.1.1 if other checks fail
	set(ICL_XDEP_OPENCV_ON_TMP ${ICL_XDEP_OPENCV_ON})
  #check for opencv <= 2.1.0
  if(WIN32)
  icl_check_external_package(OPENCV "cv.h;cvaux.h;cxcore.h;highgui.h" "cvaux200.dll.a;cxcore200.dll.a;highgui200.dll.a;cv200.dll.a;ml200.dll.a" lib include/opencv "C:/Programme/OpenCV2.0" HAVE_OPENCV_COND)
  else()
  icl_check_external_package(OPENCV "cv.h;cvaux.h;cxcore.h;highgui.h" "cvaux;cxcore;highgui;cv" lib include/opencv /opt/OpenCV2.1.1 HAVE_OPENCV_COND)
  endif()
  if(HAVE_OPENCV_COND)
    if(WIN32)
	set(OPENCV_LIBS_l cxcore200 cv200 highgui200 cvaux200)
	else()
	set(OPENCV_LIBS_l cxcore cv highgui cvaux)
	endif()
	#check for version of opencv	
	set(CMAKE_REQUIRED_LIBRARIES ${OPENCV_LIBS_l})
	set(CMAKE_REQUIRED_INCLUDES "${ICL_XDEP_OPENCV_PATH}/include/opencv")
	#if libml.so is not found to link against, this check will fail => VERSION 1 of OpenCV
	if(WIN32)
	check_library_exists(libml200.dll.a cvConvertMaps ${ICL_XDEP_OPENCV_PATH}/lib HAVE_OPENCV2_COND)
	else()
	check_library_exists(libml.so cvConvertMaps ${ICL_XDEP_OPENCV_PATH}/lib HAVE_OPENCV2_COND)
	endif()
	if(HAVE_OPENCV2_COND)
		if(WIN32)
		set(OPENCV_LIBS_l cxcore200 cv200 highgui200 cvaux200 ml200)
		else()
		set(OPENCV_LIBS_l cxcore cv highgui cvaux ml)
		endif()
		add_definitions( -DHAVE_OPENCV2)
	endif()
  else()
    #check for opencv 2.1.1
	if(ICL_XDEP_OPENCV_ON_TMP)
    set(ICL_XDEP_OPENCV_ON ${ICL_XDEP_OPENCV_ON_TMP})
    endif()
	message(STATUS "checking for v2.1.1 ${ICL_XDEP_OPENCV_ON}")
	#TODO change headernames for check cv.h;cvaux.h;cxcore.h;highgui.h;ml.h
	icl_check_external_package(OPENCV "" "opencv_core;opencv_highgui;opencv_ml;opencv_features2d;opencv_imgproc;opencv_calib3d" lib include/ /opt/OpenCV2.1.1 HAVE_OPENCV_COND)
    if(HAVE_OPENCV_COND)
	  set(HAVE_OPENCV2_COND TRUE)
      set(HAVE_OPENCV211_COND TRUE)
	  include_directories(${ICL_XDEP_OPENCV_PATH}/include)
	  set(OPENCV_INCLUDE_PATH "${OPENCV_INCLUDE_PATH};${ICL_XDEP_OPENCV_PATH}/include")
	  message(STATUS "inc ${OPENCV_INCLUDE_PATH}")
	  add_definitions( -DHAVE_OPENCV2)
	  add_definitions( -DHAVE_OPENCV211)	
	  set(OPENCV_LIBS_l opencv_core opencv_highgui opencv_ml opencv_features2d opencv_imgproc opencv_calib3d)
  	endif()
  endif()
endif()

if(WIN32)
icl_check_external_package(LIBZ zlib.h zdll.lib lib include C:/Programme/zlib125 HAVE_LIBZ_COND)
else()
icl_check_external_package(LIBZ zlib.h z lib include /usr HAVE_LIBZ_COND)
endif()
if(HAVE_LIBZ_COND)
	if(WIN32)
	set(LIBZ_LIBS_l zdll)
	else()
	set(LIBZ_LIBS_l z)
	endif()
endif()


if(NOT WIN32)
  icl_check_external_package(LIBFREENECT libfreenect.h freenect lib include /usr HAVE_LIBFREENECT_COND)
endif()
if(HAVE_LIBFREENECT_COND)
  if(NOT WIN32)
    set(LIBFREENECT_LIBS_l freenect)
  endif()
endif()


if(WIN32)
icl_check_external_package(LIBMESASR libMesaSR.h libMesaSR.lib libMesaSR libMesaSR C:/PProgramme/MesaImaging/Swissranger HAVE_LIBMESASR_COND)
else()
icl_check_external_package(LIBMESASR libMesaSR.h mesasr lib include /usr HAVE_LIBMESASR_COND)
endif()
if(HAVE_LIBMESASR_COND)
	if(WIN32)
	set(LIBMESASR_LIBS_l MesaSR)
	else()
	set(LIBMESASR_LIBS_l mesasr)
	endif()
endif()

if(WIN32)
icl_check_external_package(LIBJPEG jpeglib.h jpeg.lib lib include C:/Programme/libjpeg HAVE_LIBJPEG_COND)
else()
icl_check_external_package(LIBJPEG jpeglib.h jpeg lib include /usr HAVE_LIBJPEG_COND)
endif()
if(HAVE_LIBJPEG_COND)
	if(WIN32)
	set(LIBJPEG_LIBS_l jpeg)
	else()
	set(LIBJPEG_LIBS_l jpeg)
	endif()
endif()

####GRAPHICSMAGICK/IMAGEMAGICK####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(IMAGEMAGICK GraphicsMagick++ HAVE_IMAGEMAGICK_COND)
endif()
if(NOT IMAGEMAGICK_PKG_FOUND)
  if(WIN32)
  icl_check_external_package(IMAGEMAGICK Magick++.h "CORE_RL_wand_.lib;CORE_RL_magick_.lib;CORE_RL_Magick++_.lib" lib include "C:/PProgramme/ImageMagick-6.6.0-Q16" HAVE_IMAGEMAGICK_COND)
  else()
  icl_check_external_package(IMAGEMAGICK Magick++.h GraphicsMagick++ lib include/GraphicsMagick /usr HAVE_IMAGEMAGICK_COND)
  endif()
  if(HAVE_IMAGEMAGICK_COND)
	if(WIN32)
	set(IMAGEMAGICK_LIBS_l CORE_RL_Magick++_ CORE_RL_magick_.lib)
	else()
	set(IMAGEMAGICK_LIBS_l GraphicsMagick++)
	endif()
   endif()
endif()

####IMAGEMAGICK####
#old package, maybe  used in the future again
#icl_check_external_package(IMAGEMAGICK Magick++.h lib include/ImageMagick /usr HAVE_IMAGEMAGICK_COND)
#if(HAVE_IMAGEMAGICK_COND)
#	 	set(IMAGEMAGICK_LIBS_l Magick++)
#endif()

####LIBDC####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf(LIBDC "libdc1394-2" HAVE_LIBDC_COND)
endif()
if(NOT LIBDC_PKG_FOUND)
  icl_check_external_package(LIBDC dc1394.h dc1394 lib include/dc1394 /usr HAVE_LIBDC_COND)
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
  icl_check_external_package(UNICAP unicap.h unicap lib include/unicap /usr HAVE_UNICAP_COND)
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
  icl_check_external_package(XINE "xine.h;xine/xineutils.h" xine lib include /usr HAVE_XINE_COND)
  if(HAVE_XINE_COND)
	set(XINE_LIBS_l xine)
   endif()
endif()

####XCF####
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf_mult(XCF HAVE_XCF_COND xcf xmltio Memory)
endif()
if(NOT XCF_PKG_FOUND)
  icl_check_external_package(XCF "xcf/xcf.hpp;xmltio/xmltio.hpp" "xcf;xmltio" lib include /opt/xcf HAVE_XCF_COND)
  if(HAVE_XCF_COND)
	set(XCF_LIBS_l xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
   endif()
endif()


####Qt####
#first check if pkg-config shall be used
if(ICL_VAR_USE_PKG_CONFIG)
	icl_check_external_package_from_pkg_conf_mult(QT HAVE_QT_COND QtCore QtGui QtOpenGL)
endif()
if(NOT QT_PKG_FOUND)
  if(WIN32)
  icl_check_external_package(QT "QtCore/QtCore;QtGui/QtGui;QtOpenGL/QtOpenGL" "QtCore4.a;QtGui4.a;QtOpenGL4.a" lib include C:/Qt/2010.02.1/qt HAVE_QT_COND)
  else()
    #temporary var
    set(temp ${ICL_XDEP_QT_ON})
    #check for special location of qt
    #icl_check_external_package(QT "QtCore/QtCore;QtGui/QtGui;QtOpenGL/QtOpenGL" "QtCore;QtGui;QtOpenGL" lib include /oopt/qtsdk-2010.04/qt HAVE_QT_COND)
      if((NOT HAVE_QT_COND) OR temp)
	     #check for standard location of qt
         icl_check_external_package(QT "QtCore/QtCore;QtGui/QtGui;QtOpenGL/QtOpenGL" "QtCore;QtGui;QtOpenGL" lib include/qt4 /usr HAVE_QT_COND)
      endif()
  endif()
if(HAVE_QT_COND)
  if(WIN32)
	set(QT_LIBS_l QtCore4 QtGui4 QtOpenGL4)
  else()
	set(QT_LIBS_l QtCore QtGui QtOpenGL)
  endif()
endif()
endif()

if(WIN32)
icl_check_external_package(OPENGL "gl.h;glu.h" "glu32.a" lib include/GL C:/Qt/2010.02.1/mingw HAVE_OPENGL_COND)
else()
icl_check_external_package(OPENGL "gl.h;glu.h" "GL;GLU" lib include/GL /usr HAVE_OPENGL_COND)
endif()
if(${HAVE_OPENGL_COND})
	if(WIN32)
	set(OPENGL_LIBS_l glu32 opengl32)
	else()
	set(OPENGL_LIBS_l GL GLU)
	endif()
endif()
#stopped support
#icl_check_external_package(SVS svs.h "svs;svscalc" bin src /vol/nivision/share/SVS/4.2 HAVE_SVS_COND)
#if(HAVE_SVS_COND)
#	set(SVS_LIBS_l svs svscalc)
#endif()

####OPENSURF####
if(ICL_VAR_USE_PKG_CONFIG)
	#icl_check_external_package_from_pkg_conf(OPENSURF opensurf HAVE_OPENSURF_COND)
	icl_check_external_package_from_pkg_conf(OPENSURF surf HAVE_OPENSURF_COND)
endif()
if(NOT OPENSURF_PKG_FOUND)
  if(NOT HAVE_OPENCV211_COND)
  icl_check_external_package(OPENSURF "opensurf/fasthessian.h;opensurf/integral.h;opensurf/surf.h;opensurf/surflib.h;opensurf/utils.h" surf lib include /vol/nivision HAVE_OPENSURF_COND)
  else()
  message(STATUS "opensurf is not compatible to OpenCV2.1.1")
  endif()
  if(HAVE_OPENSURF_COND)
	set(OPENSURF_LIBS_l surf)
   endif()
endif()
#message(STATUS "surflibs: ${OPENSURF_LIBS_l}")

icl_check_external_package(GTEST gtest/gtest.h gtest lib include /vol/nivision/gtest HAVE_GTEST_COND)
if(HAVE_GTEST_COND)
	set(GTEST_LIBS_l gtest)
endif()

#add include directories of ICL
include_directories(${PROJECT_SOURCE_DIR}/include)

#set(incl)
#GET_DIRECTORY_PROPERTY(incl INCLUDE_DIRECTORIES)
#message(STATUS "i: ${incl}")

#check for doxygen in order to build the documentation of ICL
include(UseDoxygen OPTIONAL)
FIND_PACKAGE(Doxygen)

#currently we just write the version number of the ICL to this header file
configure_file(${PROJECT_SOURCE_DIR}/include/ICLUtils/config.h.cmake ${PROJECT_SOURCE_DIR}/include/ICLUtils/config.h @only)

#set the internal and external deps of the ICL
#if an entry is not available, it will be empty
set(ICLUtils_external_dependencies IPP MKL)
set(ICLCore_internal_dependencies ICLUtils)
set(ICLCC_internal_dependencies ICLCore)
set(ICLFilter_internal_dependencies ICLCC)
set(ICLBlob_internal_dependencies ICLCC)
if(${HAVE_OPENCV_COND})
    set(ICLOpenCV_internal_dependencies ICLCore ICLCC)
    set(ICLOpenCV_external_dependencies OPENCV)
    set(ICLIO_internal_dependencies ICLOpenCV ICLFilter)
    set(ICLAlgorithms_internal_dependencies ICLOpenCV ICLQuick ICLGeom)
else()
    set(ICLIO_internal_dependencies ICLFilter)
    set(ICLAlgorithms_internal_dependencies ICLQuick ICLGeom)
endif()

set(ICLIO_external_dependencies LIBZ LIBJPEG LIBMESASR LIBDC XINE UNICAP IMAGEMAGICK XCF VIDEODEV QT LIBFREENECT)
set(ICLQt_internal_dependencies ICLFilter ICLIO)
set(ICLQt_external_dependencies QT OPENGL)
if(${HAVE_QT_COND})
    set(ICLQuick_internal_dependencies ICLQt ICLFilter ICLIO ICLBlob)
else()
    set(ICLQuick_internal_dependencies ICLFilter ICLIO ICLBlob)
endif()
set(ICLGeom_internal_dependencies ICLQuick)
set(ICLAlgorithms_external_dependencies OPENSURF)# SVS)
#resolve internal dependencies
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
#resolve external dependencies
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

set(RESOURCE_DIR_PLACEHOLDER "${CMAKE_INSTALL_PREFIX}/etc/icl-${VERSION}${DEBUG_POSTFIX}")
set(ICL_PGK_CFG_PLACEHOLDER "icl-${VERSION}${DEBUG_POSTFIX}")
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
# this will overwrite the Makefile.custom.template with the configured one (where ICL_PGK_CFG_PLACEHOLDER is substituted)
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
set(ICL_SUB_PACKAGES ICLUtils ICLCore ICLCC ICLFilter ICLBlob ICLOpenCV ICLIO ICLQt ICLQuick ICLGeom ICLAlgorithms)
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
add_custom_target(alldoc doxygen alldoc/doxyfile)
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
	add_custom_target(doc)
endif()
endif()

#prepare pkg-config files if we are on linux system
if(SYSTEM_LINUX_COND)
  set(ALWAYS_TRUE TRUE)
  #replacement in pkg.in DO NOT EDIT
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
    
  set(ICL_SUB_PACKAGE "icl-${VERSION}${DEBUG_POSTFIX}")
  set(LIBS "")
  set(INCLUDES "")
  set(REQUIRE "ICLUtils-${VERSION}${DEBUG_POSTFIX} ICLCore-${VERSION}${DEBUG_POSTFIX} ICLIO-${VERSION}${DEBUG_POSTFIX} ICLCC-${VERSION}${DEBUG_POSTFIX} ICLBlob-${VERSION}${DEBUG_POSTFIX} ICLFilter-${VERSION}${DEBUG_POSTFIX} ICLQuick-${VERSION}${DEBUG_POSTFIX} ICLGeom-${VERSION}${DEBUG_POSTFIX} ICLAlgorithms-${VERSION}${DEBUG_POSTFIX}")
if(HAVE_OPENCV_COND)
  set(REQUIRE "${REQUIRE} ICLOpenCV-${VERSION}${DEBUG_POSTFIX}")
endif()
if(HAVE_QT_COND)
  set(REQUIRE "${REQUIRE} ICLQt-${VERSION}${DEBUG_POSTFIX}")
endif()
  set(ICL_PACKAGE_DESCRIPTION "Image Component Library (ICL)")
  configure_file(pkg.in ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE_NAME} @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE_NAME}" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/ 
				PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ 
				WORLD_READ WORLD_EXECUTE COMPONENT filess)
endif()

add_custom_target(postinstall chmod g+w ${CMAKE_INSTALL_PREFIX}/${LIBRARY_OUTPUT_PATH}
					COMMAND chmod g+w ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig
					COMMAND chmod g+w ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_OUTPUT_PATH}
					COMMAND chmod g+w ${CMAKE_INSTALL_PREFIX}/${DOC_DIR})

#include(tp2.cmake)
#############################################
#in shell type: cpack
#to build automatically the deb package
set(CPACK_PACKAGE_DESCRIPTION "Do the example thingie")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A longer description about our example app.")
set(CPACK_PACKAGE_NAME "ICL - Image Component Library")
#xine qt libz libjpg graphicsmagick opengl
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.6), libgcc1 (>= 1:4.1)")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libdc1394-22, libunicap2, libxine1, libgraphicsmagick++3, zlib1g")
set(CPACK_PACKAGE_CONTACT "wer auch immer")
set(CPACK_PACKAGE_VENDOR "uni")
set(CPACK_PACKAGE_VERSION_MAJOR "${ICL_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ICL_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${ICL_PATCH}")
set(VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_EXECUTABLES "")
#set(CPACK_COMPONENTS_ALL filess)
set(CPACK_ALL_INSTALL_TYPES Full)
set(CPACK_GENERATOR "DEB;TGZ")
SET(CPACK_SET_DESTDIR "ON")
#set(CPACK_INSTALL_CMAKE_PROJECTS "headers")
#message(STATUS "ignore ${CPACK_SOURCE_IGNORE_FILES}")
#set(CPACK_SOURCE_IGNORE_FILES "\\\\doxyfile$")
#set(CPACK_INSTALLED_DIRECTORIES "")
#foreach(ICL_SUB_PACKAGE_PLACEHOLDER ${ICL_SUB_PACKAGES})
#TODO set ignore file for doxyfile and install doc dir
#set(CPACK_INSTALLED_DIRECTORIES "${CPACK_INSTALLED_DIRECTORIES};${CMAKE_BINARY_DIR}/${ICL_SUB_PACKAGE_PLACEHOLDER}/doc/html;${CMAKE_INSTALL_PREFIX}/doc/${ICL_SUB_PACKAGE_PLACEHOLDER}")

#endforeach()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_PROCESSOR}")
#set(CPACK_PACKAGE_INSTALL_PREFIX "/")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
set(CPACK_STRIP_FILES "ICLUtils/lib/ICLUtils.so" "ICLCore/lib/ICLCore.so" "ICLCC/lib/ICLCC.so" "ICLFilter/lib/ICLFilter.so" "ICLBlob/lib/ICLBlob.so" "ICLOpenCV/lib/ICLOpenCV.so" "ICLIO/lib/ICLIO.so" "ICLQt/lib/ICLQt.so" "ICLQuick/lib/ICLQuick.so" "ICLGeom/lib/ICLGeom.so" "ICLAlgorithms/lib/ICLAlgorithms.so")

include(CPack)
