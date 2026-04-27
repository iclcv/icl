# ICL IO module

# Public headers (installed).  Per-backend / per-plugin headers live under
# detail/<group>/ and are intentionally NOT installed — they are implementation
# details.  Extension authors adding a new codec / grabber / output backend work
# from inside the source tree.
io_public_headers = files(
  'FileGrabber.h',
  'FileList.h',
  'FileWriter.h',
  'FilenameGenerator.h',
  'GenericGrabber.h',
  'GenericImageOutput.h',
  'Grabber.h',
  'GrabberDeviceDescription.h',
  'IO.h',
  'ImageCompressor.h',
  'TestImages.h',
)

# Always-compiled sources (root-level public API impls).
io_sources = files(
  'FileGrabber.cpp',
  'FileList.cpp',
  'FileWriter.cpp',
  'FilenameGenerator.cpp',
  'GenericGrabber.cpp',
  'GenericImageOutput.cpp',
  'Grabber.cpp',
  'ImageCompressor.cpp',
  'TestImages.cpp',
)

# Compression plugins (built-ins always compiled; zstd gated further down).
io_sources += files(
  'detail/compression-plugins/CompressionPlugin.cpp',
  'detail/compression-plugins/CompressionRegistry.cpp',
  'detail/compression-plugins/CompressionPlugin1611.cpp',
  'detail/compression-plugins/CompressionPluginJpeg.cpp',
  'detail/compression-plugins/CompressionPluginRaw.cpp',
  'detail/compression-plugins/CompressionPluginRlen.cpp',
)

# File-format plugins (built-ins always compiled; ImageMagick gated further
# down).  JPEG{Decoder,Encoder,Handle} live here because the JPEG plugins own
# them and the CompressionPluginJpeg codec shares them.
io_sources += files(
  'detail/file-plugins/FileGrabberPluginBICL.cpp',
  'detail/file-plugins/FileGrabberPluginCSV.cpp',
  'detail/file-plugins/FileGrabberPluginJPEG.cpp',
  'detail/file-plugins/FileGrabberPluginPNG.cpp',
  'detail/file-plugins/FileGrabberPluginPNM.cpp',
  'detail/file-plugins/FileWriterPluginBICL.cpp',
  'detail/file-plugins/FileWriterPluginCSV.cpp',
  'detail/file-plugins/FileWriterPluginJPEG.cpp',
  'detail/file-plugins/FileWriterPluginPNG.cpp',
  'detail/file-plugins/FileWriterPluginPNM.cpp',
  'detail/file-plugins/JPEGDecoder.cpp',
  'detail/file-plugins/JPEGEncoder.cpp',
  'detail/file-plugins/JPEGHandle.cpp',
)

# Always-built grabbers + small decoders (no external deps).
io_sources += files(
  'detail/grabbers/ColorFormatDecoder.cpp',
  'detail/grabbers/CreateGrabber.cpp',
  'detail/grabbers/DemoGrabber.cpp',
  'detail/grabbers/MyrmexDecoder.cpp',
)

# Built-in test images: raw JPEG/XPM blobs + factory lambdas that
# self-register into testImageRegistry() at static-init time.
io_sources += files(
  'detail/builtin-images/TestImageCameraman.cpp',
  'detail/builtin-images/TestImageFlowers.cpp',
  'detail/builtin-images/TestImageLena.cpp',
  'detail/builtin-images/TestImageMandril.cpp',
  'detail/builtin-images/TestImageParrot.cpp',
  'detail/builtin-images/TestImageWindows.cpp',
)

# 11-bit compressor — shared between Kinect grabber and CompressionPlugin1611.
io_sources += files('detail/kinect/Kinect11BitCompressor.cpp')

io_extra_deps = []

# libdc1394
if libdc_dep.found()
  io_sources += files(
    'detail/dc/DC.cpp',
    'detail/dc/DCDevice.cpp',
    'detail/dc/DCDeviceFeatures.cpp',
    'detail/dc/DCFrameQueue.cpp',
    'detail/dc/DCGrabber.cpp',
    'detail/dc/DCGrabberThread.cpp',
  )
  io_extra_deps += libdc_dep
endif

# V4L2
if v4l_dep.found()
  io_sources += files(
    'detail/v4l2/V4L2Grabber.cpp',
    'detail/v4l2/V4L2LoopBackOutput.cpp',
  )
  io_extra_deps += v4l_dep
endif

# libfreenect
if freenect_dep.found()
  io_sources += files('detail/kinect/KinectGrabber.cpp')
  io_extra_deps += freenect_dep
endif

# libfreenect2
if freenect2_dep.found()
  io_sources += files('detail/kinect/Kinect2Grabber.cpp')
  io_extra_deps += freenect2_dep
endif

# Pylon
if pylon_dep.found()
  io_sources += files(
    'detail/pylon/PylonCameraOptions.cpp',
    'detail/pylon/PylonColorConverter.cpp',
    'detail/pylon/PylonGrabber.cpp',
    'detail/pylon/PylonGrabberThread.cpp',
    'detail/pylon/PylonUtils.cpp',
  )
  io_extra_deps += pylon_dep
endif

# OpenCV
if opencv_dep.found()
  io_sources += files(
    'detail/opencv/OpenCVCamGrabber.cpp',
    'detail/opencv/OpenCVVideoGrabber.cpp',
    'detail/opencv/OpenCVVideoWriter.cpp',
  )
endif

# ImageMagick
if imagemagick_dep.found()
  io_sources += files(
    'detail/file-plugins/FileGrabberPluginImageMagick.cpp',
    'detail/file-plugins/FileWriterPluginImageMagick.cpp',
  )
  io_extra_deps += imagemagick_dep
endif

# FFmpeg/libav
if libav_found
  io_sources += files('detail/libav/LibAVVideoWriter.cpp')
  io_extra_deps += libav_deps
endif

# zstd compression plugin (optional)
if zstd_dep.found()
  io_sources += files('detail/compression-plugins/CompressionPluginZstd.cpp')
  io_extra_deps += zstd_dep
endif

# Qt6 WebSockets (WSImageOutput + WSGrabber). Self-gating: only built
# when both Qt and the WebSockets module are available. WS replaces the
# legacy SharedMemoryGrabber/Publisher pair retired in Session 47 — the
# loopback path covers the same loose-coupling-between-processes use
# case with auto-reconnect resilience and cross-host as a free bonus.
if qt_found and qt_websockets_dep.found()
  io_sources += files(
    'detail/network/WSImageOutput.cpp',
    'detail/network/WSGrabber.cpp',
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

install_headers(io_public_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'io',
)

pkg.generate(icl_io_lib,
  name: 'icl-io',
  description: 'ICL IO module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/io/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
