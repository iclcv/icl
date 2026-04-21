# ICL Utils module - foundation layer

# ---- Generated config headers (output to builddir/icl/utils/) ----
# No .in templates needed — Meson generates directly from configuration_data.
version_h_conf = configuration_data()
version_h_conf.set('ICL_VERSION_MAJOR', version_major)
version_h_conf.set('ICL_VERSION_MINOR', version_minor)
version_h_conf.set('ICL_VERSION_PATCH', version_patch)
version_h_conf.set_quoted('ICL_SO_VERSION', so_version)
version_h_conf.set_quoted('ICL_VERSION_STRING', meson.project_version())

configure_file(
  output: 'ICLVersion.h',
  configuration: version_h_conf,
  install: true,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'utils',
)

configure_file(
  output: 'ICLConfig.h',
  configuration: conf_config,
  install: true,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'utils',
)

utils_headers = files(
  'Any.h',
  'Array2D.h',
  'BackendDispatching.h',
  'BasicTypes.h',
  'ClippedCast.h',
  'CompatMacros.h',
  'ConfigFile.h',
  'Configurable.h',
  'ConfigurableProxy.h',
  'ConsoleProgress.h',
  'EnumDispatch.h',
  'Exception.h',
  'FPSEstimator.h',
  'FPSLimiter.h',
  'FastMedianList.h',
  'File.h',
  'FixedArray.h',
  'Lockable.h',
  'Macros.h',
  'MultiTypeMap.h',
  'PThreadFix.h',
  'ParamList.h',
  'PluginRegistry.h',
  'Point.h',
  'Point32f.h',
  'ProcessMonitor.h',
  'ProgArg.h',
  'Random.h',
  'Range.h',
  'Rect.h',
  'Rect32f.h',
  'SSETypes.h',
  'SSEUtils.h',
  'SignalHandler.h',
  'Size.h',
  'Size32f.h',
  'StackTimer.h',
  'SteppingRange.h',
  'StrTok.h',
  'StringUtils.h',
  'TextTable.h',
  'Thread.h',
  'Time.h',
  'Timer.h',
  'UncopiedInstance.h',
  'Uncopyable.h',
  'Utils.h',
  'VisualizationDescription.h',
  'XML.h',
)

# OpenCL wrappers — conditionally compiled; always-installed public
# headers so dependent headers (PointCloudCreatorCL, Segmentation3D, …)
# keep working when OpenCL is on.
utils_cl_headers = files(
  'cl/CLBuffer.h',
  'cl/CLDeviceContext.h',
  'cl/CLException.h',
  'cl/CLImage2D.h',
  'cl/CLIncludes.h',
  'cl/CLKernel.h',
  'cl/CLMemory.h',
  'cl/CLMemoryAssistant.h',
  'cl/CLProgram.h',
)

utils_sources = files(
  'cl/CLBuffer.cpp',
  'cl/CLDeviceContext.cpp',
  'cl/CLImage2D.cpp',
  'cl/CLKernel.cpp',
  'cl/CLMemoryAssistant.cpp',
  'cl/CLProgram.cpp',
  'ConfigFile.cpp',
  'Configurable.cpp',
  'ConsoleProgress.cpp',
  'Exception.cpp',
  'FPSEstimator.cpp',
  'FPSLimiter.cpp',
  'File.cpp',
  'MultiTypeMap.cpp',
  'Point.cpp',
  'Point32f.cpp',
  'ProcessMonitor.cpp',
  'ProgArg.cpp',
  'detail/pugi/PugiXML.cpp',
  'Range.cpp',
  'Rect.cpp',
  'Rect32f.cpp',
  'SignalHandler.cpp',
  'Size.cpp',
  'Size32f.cpp',
  'SteppingRange.cpp',
  'StrTok.cpp',
  'StringUtils.cpp',
  'TextTable.cpp',
  'Thread.cpp',
  'Time.cpp',
  'Timer.cpp',
)

utils_deps = [thread_dep, zlib_dep, jpeg_dep, png_dep, dl_dep, m_dep, opencl_dep, glew_dep, openmp_dep]

icl_utils_lib = shared_library('icl-utils',
  utils_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: utils_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/utils.h',
  install: true,
)

icl_utils_dep = declare_dependency(
  link_with: icl_utils_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: utils_deps,
)

install_headers(utils_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'utils',
)

install_headers(utils_cl_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'utils' / 'cl',
)

# Vendored pugixml lives under detail/pugi/ — it's only reached via
# icl/utils/XML.h (which aliases pugi:: types as icl::utils::XML*).
# The header must still be installed so XML.h is self-contained.
install_headers(
  files('detail/pugi/PugiXML.h', 'detail/pugi/pugiconfig.hpp'),
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'utils' / 'detail' / 'pugi',
)

pkg.generate(icl_utils_lib,
  name: 'icl-utils',
  description: 'ICL Utils module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/utils/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
