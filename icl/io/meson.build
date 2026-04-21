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
  'FileGrabberPluginBICL.h',
  'FileGrabberPluginCSV.h',
  'FileGrabberPluginImageMagick.h',
  'FileGrabberPluginJPEG.h',
  'FileGrabberPluginPNG.h',
  'FileGrabberPluginPNM.h',
  'FileList.h',
  'FileWriter.h',
  'FileWriterPluginBICL.h',
  'FileWriterPluginCSV.h',
  'FileWriterPluginImageMagick.h',
  'FileWriterPluginJPEG.h',
  'FileWriterPluginPNG.h',
  'FileWriterPluginPNM.h',
  'FilenameGenerator.h',
  'FourCC.h',
  'CompressionPlugin.h',
  'CompressionRegistry.h',
  'GenericGrabber.h',
  'GenericImageOutput.h',
  'Grabber.h',
  'GrabberDeviceDescription.h',
  'IO.h',
  'IOFunctions.h',
  'ImageCompressor.h',
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
  'SwissRangerGrabber.h',
  'TestImages.h',
  'V4L2Grabber.h',
  'V4L2LoopBackOutput.h',
  'XiGrabber.h',
  'WSGrabber.h',
  'WSImageOutput.h',
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
  'CompressionPlugin1611.cpp',
  'CompressionPluginJpeg.cpp',
  'CompressionPluginRaw.cpp',
  'CompressionPluginRlen.cpp',
  'CompressionRegistry.cpp',
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

# zstd compression plugin (optional)
if zstd_dep.found()
  io_sources += files('CompressionPluginZstd.cpp')
  io_extra_deps += zstd_dep
endif

# Qt6 WebSockets (WSImageOutput + WSGrabber). Self-gating: only built
# when both Qt and the WebSockets module are available. WS replaces the
# legacy SharedMemoryGrabber/Publisher pair retired in Session 47 — the
# loopback path covers the same loose-coupling-between-processes use
# case with auto-reconnect resilience and cross-host as a free bonus.
if qt_found and qt_websockets_dep.found()
  io_sources += files(
    'WSImageOutput.cpp',
    'WSGrabber.cpp',
  )
  io_extra_deps += qt_websockets_dep
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
