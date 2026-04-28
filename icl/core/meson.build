# ICL Core module

core_prop_headers = files(
  'prop/Constraints.h',
)

core_convert_headers = files(
  'convert/Converter.h',
  'convert/FixedConverter.h',
)

core_line_headers = files(
  'line/Line.h',
  'line/LineSampler.h',
)

core_headers = files(
  'BayerConverter.h',
  'CCFunctions.h',
  'Channel.h',
  'ChromaAndRGBClassifier.h',
  'ChromaClassifier.h',
  'Color.h',
  'Core.h',
  'CoreFunctions.h',
  'DataSegment.h',
  'DataSegmentBase.h',
  'DoxygenMainPage.h',
  'Image.h',
  'ImageBackendDispatching.h',
  'Img.h',
  'ImgBase.h',
  'ImgBorder.h',
  'ImgIterator.h',
  'ImgOps.h',
  'ImgParams.h',
  'OpenCV.h',
  'Parable.h',
  'PixelOps.h',
  'PixelRef.h',
  'Types.h',
  'Visitors.h',
  'VisitorsN.h',
)

core_sources = files(
  'prop/Adapter.cpp',
  'BayerConverter.cpp',
  'CCFunctions.cpp',
  'CCLUT.cpp',
  'Color.cpp',
  'convert/Converter.cpp',
  'CoreFunctions.cpp',
  'convert/FixedConverter.cpp',
  'Image.cpp',
  'Img.cpp',
  'ImgBase.cpp',
  'ImgBorder.cpp',
  'ImgBuffer.cpp',
  'ImgOps.cpp',
  'ImgParams.cpp',
  'detail/img/Img_Cpp.cpp',
  'line/Line.cpp',
  'line/LineSampler.cpp',
  'PixelOps.cpp',
)

# Conditional backend sources
if ipp_dep.found()
  core_sources += files('detail/img/Img_Ipp.cpp')
endif

if accelerate_dep.found()
  core_sources += files('detail/img/Img_Accelerate.cpp')
endif

if opencv_dep.found()
  core_sources += files('OpenCV.cpp')
endif

core_deps = [icl_math_dep, opencv_dep, accelerate_dep, ipp_dep]

icl_core_lib = shared_library('icl-core',
  core_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: core_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/core.h',
  install: true,
)

icl_core_dep = declare_dependency(
  link_with: icl_core_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: core_deps,
)

install_headers(core_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'core',
)

install_headers(core_prop_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'core' / 'prop',
)

install_headers(core_convert_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'core' / 'convert',
)

install_headers(core_line_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'core' / 'line',
)

pkg.generate(icl_core_lib,
  name: 'icl-core',
  description: 'ICL Core module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/core/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
