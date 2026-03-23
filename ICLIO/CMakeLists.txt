file(GLOB SOURCES src/ICLIO/*.cpp)
file(GLOB HEADERS src/ICLIO/*.h)

# Remove optional backend sources that aren't available.
# Files are always present on disk; we exclude them when the dependency is missing.
macro(iclio_require_dep DEP)
  if(NOT ${DEP})
    foreach(_f ${ARGN})
      list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLIO/${_f}.cpp)
      list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLIO/${_f}.h)
    endforeach()
  endif()
endmacro()

iclio_require_dep(LIBUSB_FOUND PixelSenseGrabber)
iclio_require_dep(ImageMagick_FOUND FileWriterPluginImageMagick FileGrabberPluginImageMagick)
iclio_require_dep(LIBDC_FOUND DC DCDevice DCDeviceFeatures DCFrameQueue DCGrabber DCGrabberThread)
# DCDeviceOptions.h is header-only, keep it
iclio_require_dep(V4L_FOUND V4L2Grabber V4L2LoopBackOutput)
iclio_require_dep(OpenCV_FOUND OpenCVCamGrabber OpenCVVideoWriter OpenCVVideoGrabber)
iclio_require_dep(LIBAV_FOUND LibAVVideoWriter)
iclio_require_dep(QT_FOUND SharedMemoryGrabber SharedMemoryPublisher SharedMemorySegment)
iclio_require_dep(ZMQ_FOUND ZmqGrabber ZmqImageOutput)
iclio_require_dep(LIBFREENECT_FOUND KinectGrabber)
iclio_require_dep(LIBFREENECT2_FOUND Kinect2Grabber)
iclio_require_dep(PYLON_FOUND PylonGrabber PylonGrabberThread PylonCameraOptions PylonColorConverter PylonUtils)

# Remove dead backends (dependencies removed in overhaul)
foreach(_dead SwissRangerGrabber VideoGrabber OpenNIGrabber OpenNIUtils XiGrabber OptrisGrabber
             RSBGrabber RSBImageOutput RSBIOUtil ConfigurableRemoteServer)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLIO/${_dead}.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLIO/${_dead}.h)
endforeach()

icl_add_module(ICLIO
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLFilter
  3RDPARTY_LIBS ${ICLIO_3RDPARTY_LIBRARIES}
  PKGCONFIG_DEPS ICLFilter-${SO_VERSION}
  HAS_DEMOS HAS_APPS
  PCH_HEADERS
    <vector> <string> <map> <algorithm> <functional>
    <iostream> <sstream> <memory> <cmath> <type_traits>
    <ICLUtils/CompatMacros.h>
    <ICLUtils/BasicTypes.h>
    <ICLUtils/Exception.h>
    <ICLUtils/StringUtils.h>
    <ICLUtils/Macros.h>
    <ICLUtils/Configurable.h>
    <ICLCore/Img.h>
    <ICLCore/ImgBase.h>)
