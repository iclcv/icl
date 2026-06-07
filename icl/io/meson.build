# ICL IO module

# Public headers (installed).  Clustered into topical subdirs (source/,
# sink/, file/, compress/); only the umbrella IO.h + the small free-function
# helpers (ExternalViewer, SaveLoad) stay at module-top.  Per-backend /
# per-plugin headers live under detail/<group>/ and are intentionally NOT
# installed — they are implementation details.  Extension authors adding a new
# codec / source / sink backend work from inside the source tree.
io_public_headers = files(
  'IO.h',
  'ExternalViewer.h',
  'SaveLoad.h',
)

io_source_headers = files(
  'source/ImageSource.h',
  'source/SourceBackend.h',
  'source/DeviceDescription.h',
  'source/SourceBackendRegistry.h',
  'source/TestImages.h',
)

io_sink_headers = files(
  'sink/ImageSink.h',
)

io_file_headers = files(
  'file/FileList.h',
)

io_compress_headers = files(
  'compress/ImageCompressor.h',
)

# Always-compiled sources (public API impls, mirroring the header clustering).
io_sources = files(
  'source/ImageSource.cpp',
  'source/SourceBackend.cpp',
  'source/SourceBackendRegistry.cpp',
  'source/TestImages.cpp',
  'sink/ImageSink.cpp',
  'file/FileList.cpp',
  'detail/FileSource.cpp',
  'detail/FileWriter.cpp',
  'detail/FilenameGenerator.cpp',
  'compress/ImageCompressor.cpp',
  'ExternalViewer.cpp',
  'SaveLoad.cpp',
)

# Compression plugins (built-ins always compiled; zstd gated further down).
#
# These TUs self-register via __attribute__((constructor)) at dyld time —
# nothing in libicl-io references their symbols.  On macOS that means the
# linker dead-strips the entire .o (no __init_offsets entry → dyld never
# calls the ctor).  Building them as a static_library and linking it with
# link_whole into libicl-io forces every .o to be retained regardless of
# external references, so the ctors fire reliably.
io_compression_plugin_sources = files(
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
  'detail/file-plugins/FileSourcePluginBICL.cpp',
  'detail/file-plugins/FileSourcePluginCSV.cpp',
  'detail/file-plugins/FileSourcePluginJPEG.cpp',
  'detail/file-plugins/FileSourcePluginPNG.cpp',
  'detail/file-plugins/FileSourcePluginPNM.cpp',
  'detail/file-plugins/FileSinkPluginBICL.cpp',
  'detail/file-plugins/FileSinkPluginCSV.cpp',
  'detail/file-plugins/FileSinkPluginJPEG.cpp',
  'detail/file-plugins/FileSinkPluginPNG.cpp',
  'detail/file-plugins/FileSinkPluginPNM.cpp',
  'detail/file-plugins/JPEGDecoder.cpp',
  'detail/file-plugins/JPEGEncoder.cpp',
  'detail/file-plugins/JPEGHandle.cpp',
)

# Always-built grabbers + small decoders (no external deps).
io_sources += files(
  'detail/grabbers/ColorFormatDecoder.cpp',
  'detail/grabbers/CreateSource.cpp',
  'detail/grabbers/DemoSource.cpp',
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
    'detail/dc/DCSource.cpp',
    'detail/dc/DCGrabberThread.cpp',
  )
  io_extra_deps += libdc_dep
endif

# V4L2
if v4l_dep.found()
  io_sources += files(
    'detail/v4l2/V4L2Source.cpp',
    'detail/v4l2/V4L2LoopBackOutput.cpp',
  )
  io_extra_deps += v4l_dep
endif

# libfreenect
if freenect_dep.found()
  io_sources += files('detail/kinect/KinectSource.cpp')
  io_extra_deps += freenect_dep
endif

# libfreenect2
if freenect2_dep.found()
  io_sources += files('detail/kinect/Kinect2Source.cpp')
  io_extra_deps += freenect2_dep
endif

# Pylon
if pylon_dep.found()
  io_sources += files(
    'detail/pylon/PylonCameraOptions.cpp',
    'detail/pylon/PylonColorConverter.cpp',
    'detail/pylon/PylonSource.cpp',
    'detail/pylon/PylonGrabberThread.cpp',
    'detail/pylon/PylonUtils.cpp',
  )
  io_extra_deps += pylon_dep
endif

# OpenCV
if opencv_dep.found()
  io_sources += files(
    'detail/opencv/OpenCVCamSource.cpp',
    'detail/opencv/OpenCVVideoSource.cpp',
    'detail/opencv/OpenCVVideoWriter.cpp',
  )
endif

# ImageMagick
if imagemagick_dep.found()
  io_sources += files(
    'detail/file-plugins/FileSourcePluginImageMagick.cpp',
    'detail/file-plugins/FileSinkPluginImageMagick.cpp',
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
  io_compression_plugin_sources += files('detail/compression-plugins/CompressionPluginZstd.cpp')
  io_extra_deps += zstd_dep
endif

# Qt6 WebSockets (WSSink + WSSource). Self-gating: only built
# when both Qt and the WebSockets module are available. WS replaces the
# legacy SharedMemoryGrabber/Publisher pair retired in Session 47 — the
# loopback path covers the same loose-coupling-between-processes use
# case with auto-reconnect resilience and cross-host as a free bonus.
if qt_found and qt_websockets_dep.found()
  io_sources += files(
    'detail/network/WSSink.cpp',
    'detail/network/WSSource.cpp',
  )
  io_extra_deps += qt_websockets_dep
  io_extra_deps += qt_dep
endif

io_deps = [icl_filter_dep, opencv_dep] + io_extra_deps

# Compression plugins are bundled as a static archive linked with
# link_whole so the linker can't dead-strip the self-registering TUs
# (see comment on io_compression_plugin_sources).
io_compression_plugins_lib = static_library('icl-io-compression-plugins',
  io_compression_plugin_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: io_deps,
  implicit_include_directories: false,
  cpp_pch: '../_pch/io.h',
)

icl_io_lib = shared_library('icl-io',
  io_sources,
  link_whole: io_compression_plugins_lib,
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

install_headers(io_source_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'io' / 'source',
)

install_headers(io_sink_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'io' / 'sink',
)

install_headers(io_file_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'io' / 'file',
)

install_headers(io_compress_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'io' / 'compress',
)

pkg.generate(icl_io_lib,
  name: 'icl-io',
  description: 'ICL IO module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/io/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
