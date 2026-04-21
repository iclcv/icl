# ICL IO module

io_headers = files(
  'ColorFormatDecoder.h',
  'CreateGrabber.h',
  'DC.h',
  'DCDevice.h',
  'DCDeviceFeatures.h',
  'DCDeviceOptions.h',
  'DCFrameQueue.h',
  'DCGrabber.h',
  'DCGrabberThread.h',
  'DemoGrabber.h',
  'FileGrabber.h',
  'FileGrabberPlugin.h',
  'FileGrabberPluginBICL.h',
  'FileGrabberPluginCSV.h',
  'FileGrabberPluginImageMagick.h',
  'FileGrabberPluginJPEG.h',
  'FileGrabberPluginPNG.h',
  'FileGrabberPluginPNM.h',
  'FileList.h',
  'FileWriter.h',
  'FileWriterPlugin.h',
  'FileWriterPluginBICL.h',
  'FileWriterPluginCSV.h',
  'FileWriterPluginImageMagick.h',
  'FileWriterPluginJPEG.h',
  'FileWriterPluginPNG.h',
  'FileWriterPluginPNM.h',
  'FilenameGenerator.h',
  'FourCC.h',
  'GenericGrabber.h',
  'GenericImageOutput.h',
  'Grabber.h',
  'GrabberDeviceDescription.h',
  'IO.h',
  'IOFunctions.h',
  'ImageCompressor.h',
  'ImageOutput.h',
  'ImageUndistortion.h',
  'IntrinsicCalibrator.h',
  'JPEGDecoder.h',
  'JPEGEncoder.h',
  'JPEGHandle.h',
  'Kinect11BitCompressor.h',
  'Kinect2Grabber.h',
  'KinectGrabber.h',
  'LibAVVideoWriter.h',
  'MyrmexDecoder.h',
  'OpenCVCamGrabber.h',
  'OpenCVVideoGrabber.h',
  'OpenCVVideoWriter.h',
  'OpenNIGrabber.h',
  'OpenNIIncludes.h',
  'OpenNIUtils.h',
  'OptrisGrabber.h',
  'PixelSenseGrabber.h',
  'PylonCameraOptions.h',
  'PylonColorConverter.h',
  'PylonGrabber.h',
  'PylonGrabberThread.h',
  'PylonIncludes.h',
  'PylonUtils.h',
  'SharedMemoryGrabber.h',
  'SharedMemoryPublisher.h',
  'SharedMemorySegment.h',
  'SwissRangerGrabber.h',
  'TestImages.h',
  'V4L2Grabber.h',
  'V4L2LoopBackOutput.h',
  'VideoGrabber.h',
  'XiGrabber.h',
  'ZmqGrabber.h',
  'ZmqImageOutput.h',
)

# Core (always-compiled) sources
io_sources = files(
  'ColorFormatDecoder.cpp',
  'CreateGrabber.cpp',
  'DemoGrabber.cpp',
  'DemoImageCameraMan.cpp',
  'DemoImageFlowers.cpp',
  'DemoImageLena.cpp',
  'DemoImageMacaw.cpp',
  'DemoImageMandril.cpp',
  'DemoImageWindows.cpp',
  'FileGrabber.cpp',
  'FileGrabberPluginBICL.cpp',
  'FileGrabberPluginCSV.cpp',
  'FileGrabberPluginJPEG.cpp',
  'FileGrabberPluginPNG.cpp',
  'FileGrabberPluginPNM.cpp',
  'FileList.cpp',
  'FileWriter.cpp',
  'FileWriterPluginBICL.cpp',
  'FileWriterPluginCSV.cpp',
  'FileWriterPluginJPEG.cpp',
  'FileWriterPluginPNG.cpp',
  'FileWriterPluginPNM.cpp',
  'FilenameGenerator.cpp',
  'GenericGrabber.cpp',
  'GenericImageOutput.cpp',
  'Grabber.cpp',
  'IOFunctions.cpp',
  'ImageCompressor.cpp',
  'ImageUndistortion.cpp',
  'IntrinsicCalibrator.cpp',
  'JPEGDecoder.cpp',
  'JPEGEncoder.cpp',
  'JPEGHandle.cpp',
  'Kinect11BitCompressor.cpp',
  'MyrmexDecoder.cpp',
  'TestImages.cpp',
)
# VideoGrabber.cpp requires xine — disabled (dead dependency)

io_extra_deps = []

# libdc1394
if libdc_dep.found()
  io_sources += files(
    'DC.cpp',
    'DCDevice.cpp',
    'DCDeviceFeatures.cpp',
    'DCFrameQueue.cpp',
    'DCGrabber.cpp',
    'DCGrabberThread.cpp',
  )
  io_extra_deps += libdc_dep
endif

# V4L2
if v4l_dep.found()
  io_sources += files(
    'V4L2Grabber.cpp',
    'V4L2LoopBackOutput.cpp',
  )
  io_extra_deps += v4l_dep
endif

# libfreenect
if freenect_dep.found()
  io_sources += files('KinectGrabber.cpp')
  io_extra_deps += freenect_dep
endif

# libfreenect2
if freenect2_dep.found()
  io_sources += files('Kinect2Grabber.cpp')
  io_extra_deps += freenect2_dep
endif

# Pylon
if pylon_dep.found()
  io_sources += files(
    'PylonCameraOptions.cpp',
    'PylonColorConverter.cpp',
    'PylonGrabber.cpp',
    'PylonGrabberThread.cpp',
    'PylonUtils.cpp',
  )
  io_extra_deps += pylon_dep
endif

# OpenCV
if opencv_dep.found()
  io_sources += files(
    'OpenCVCamGrabber.cpp',
    'OpenCVVideoGrabber.cpp',
    'OpenCVVideoWriter.cpp',
  )
endif

# ImageMagick
if imagemagick_dep.found()
  io_sources += files(
    'FileGrabberPluginImageMagick.cpp',
    'FileWriterPluginImageMagick.cpp',
  )
  io_extra_deps += imagemagick_dep
endif

# FFmpeg/libav
if libav_found
  io_sources += files('LibAVVideoWriter.cpp')
  io_extra_deps += libav_deps
endif

# ZeroMQ
if zmq_dep.found()
  io_sources += files(
    'ZmqGrabber.cpp',
    'ZmqImageOutput.cpp',
  )
  io_extra_deps += zmq_dep
endif

# Qt-dependent sources (shared memory grabbers)
if qt_found
  io_sources += files(
    'SharedMemoryGrabber.cpp',
    'SharedMemoryPublisher.cpp',
    'SharedMemorySegment.cpp',
  )
  io_extra_deps += qt_dep
endif

io_deps = [icl_filter_dep, opencv_dep] + io_extra_deps

icl_io_lib = shared_library('icl-io',
  io_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: io_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/io.h',
  install: true,
)

icl_io_dep = declare_dependency(
  link_with: icl_io_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: io_deps,
)

install_headers(io_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'io',
)

pkg.generate(icl_io_lib,
  name: 'icl-io',
  description: 'ICL IO module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/io/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
