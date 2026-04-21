# ICL Core module

core_headers = files(
  'AbstractCanvas.h',
  'BayerConverter.h',
  'CCFunctions.h',
  'CCLUT.h',
  'Channel.h',
  'ChromaAndRGBClassifier.h',
  'ChromaClassifier.h',
  'Color.h',
  'Converter.h',
  'ConvexHull.h',
  'Core.h',
  'CoreFunctions.h',
  'DataSegment.h',
  'DataSegmentBase.h',
  'DoxygenMainPage.h',
  'FixedConverter.h',
  'Image.h',
  'ImageBackendDispatching.h',
  'ImageRenderer.h',
  'ImageSerializer.h',
  'Img.h',
  'ImgBase.h',
  'ImgBorder.h',
  'ImgBuffer.h',
  'ImgIterator.h',
  'ImgOps.h',
  'ImgParams.h',
  'Line.h',
  'Line32f.h',
  'LineSampler.h',
  'OpenCV.h',
  'Parable.h',
  'PixelOps.h',
  'PixelRef.h',
  'PseudoColorConverter.h',
  'Types.h',
  'Visitors.h',
  'VisitorsN.h',
)

core_sources = files(
  'AbstractCanvas.cpp',
  'BayerConverter.cpp',
  'CCFunctions.cpp',
  'CCLUT.cpp',
  'Color.cpp',
  'Converter.cpp',
  'ConvexHull.cpp',
  'CoreFunctions.cpp',
  'FixedConverter.cpp',
  'Image.cpp',
  'ImageRenderer.cpp',
  'ImageSerializer.cpp',
  'Img.cpp',
  'ImgBase.cpp',
  'ImgBorder.cpp',
  'ImgBuffer.cpp',
  'ImgOps.cpp',
  'ImgParams.cpp',
  'Img_Cpp.cpp',
  'Line.cpp',
  'Line32f.cpp',
  'LineSampler.cpp',
  'PixelOps.cpp',
  'PseudoColorConverter.cpp',
)

# Conditional backend sources
if ipp_dep.found()
  core_sources += files('Img_Ipp.cpp')
endif

if accelerate_dep.found()
  core_sources += files('Img_Accelerate.cpp')
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

pkg.generate(icl_core_lib,
  name: 'icl-core',
  description: 'ICL Core module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/core/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
