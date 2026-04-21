project('icl', 'cpp',
  version: '26.0.0',
  default_options: [
    'cpp_std=c++20',
    'buildtype=release',
    'default_library=shared',
  ],
  meson_version: '>= 1.0',
)

so_version = '26.0'
install_prefix = 'icl-' + so_version
pkg = import('pkgconfig')
cc = meson.get_compiler('cpp')
fs = import('fs')

# ---- Version ----
version_major = meson.project_version().split('.')[0]
version_minor = meson.project_version().split('.')[1]
version_patch = meson.project_version().split('.')[2]

# ---- Compiler flags ----
if cc.get_id() != 'msvc'
  add_project_arguments('-Wall', language: 'cpp')
endif

if get_option('native')
  add_project_arguments('-march=native', language: 'cpp')
endif

# ---- SIMD detection ----
host_cpu = host_machine.cpu_family()
have_sse2 = host_cpu == 'x86_64'
have_neon = host_cpu == 'aarch64'

sse2neon_inc = []
simd_args = []
if have_sse2
  simd_args += ['-DICL_USE_SSE2', '-DICL_USE_SSE3', '-DICL_USE_SSSE3']
elif have_neon
  sse2neon_inc = [include_directories('3rdparty/sse2neon')]
  simd_args += ['-DICL_USE_SSE2', '-DICL_USE_SSE3', '-DICL_USE_SSSE3']
endif
add_project_arguments(simd_args, language: 'cpp')

# ---- OpenMP ----
openmp_dep = dependency('openmp', required: get_option('openmp'))
if openmp_dep.found()
  add_project_arguments('-DICL_HAVE_OPENMP', language: 'cpp')
endif

# ---- Platform definitions ----
if host_machine.system() == 'darwin'
  add_project_arguments('-DICL_SYSTEM_APPLE', language: 'cpp')
elif host_machine.system() == 'linux'
  add_project_arguments('-DICL_SYSTEM_LINUX', language: 'cpp')
elif host_machine.system() == 'windows'
  add_project_arguments('-DICL_SYSTEM_WINDOWS', '-DHAVE_STRUCT_TIMESPEC', language: 'cpp')
endif

# ---- Include path: project root for #include <icl/foo/Bar.h> ----
root_inc = include_directories('.')

# ---- Required dependencies ----
thread_dep = dependency('threads')
zlib_dep = dependency('zlib')
jpeg_dep = dependency('libjpeg')
png_dep = dependency('libpng')
dl_dep = cc.find_library('dl', required: false)
m_dep = cc.find_library('m', required: false)

add_project_arguments(
  '-DICL_HAVE_LIBZ', '-DICL_HAVE_LIBJPEG', '-DICL_HAVE_LIBPNG',
  language: 'cpp',
)

required_deps = [thread_dep, zlib_dep, jpeg_dep, png_dep, dl_dep, m_dep, openmp_dep]

# ---- Optional dependencies ----

# Qt6
qt_opt = get_option('qt')
qt_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets', 'OpenGL', 'OpenGLWidgets'],
                    required: qt_opt)
qt_found = qt_dep.found()
qt_multimedia_dep = dependency('qt6', modules: ['Multimedia'], required: false)
if qt_found
  add_project_arguments('-DICL_HAVE_QT', '-DICL_HAVE_OPENGL', language: 'cpp')
  opengl_dep = dependency('gl')
  glew_dep = dependency('glew', required: true)
else
  opengl_dep = dependency('', required: false)
  glew_dep = dependency('', required: false)
endif

# OpenCV
opencv_dep = dependency('opencv4', required: get_option('opencv'))
have_opencv_features2d = false
if opencv_dep.found()
  add_project_arguments('-DICL_HAVE_OPENCV', language: 'cpp')
  have_opencv_features2d = cc.has_header('opencv2/features2d.hpp', dependencies: opencv_dep)
  if have_opencv_features2d
    add_project_arguments('-DICL_HAVE_OPENCV_FEATURES_2D', language: 'cpp')
  endif
endif

# Eigen3
eigen_dep = dependency('eigen3', required: get_option('eigen'))
if eigen_dep.found()
  add_project_arguments('-DICL_HAVE_EIGEN3', language: 'cpp')
endif

# OpenCL — on macOS, dependency('OpenCL') adds -F flags that break std headers.
# Use appleframeworks instead.
if host_machine.system() == 'darwin'
  opencl_dep = dependency('appleframeworks', modules: ['OpenCL'],
                          required: get_option('opencl'))
else
  opencl_dep = dependency('OpenCL', required: get_option('opencl'))
endif
if opencl_dep.found()
  add_project_arguments('-DICL_HAVE_OPENCL', language: 'cpp')
endif

# ImageMagick
imagemagick_dep = dependency('Magick++', required: get_option('imagemagick'))
if imagemagick_dep.found()
  add_project_arguments('-DICL_HAVE_IMAGEMAGICK', language: 'cpp')
endif

# FFmpeg/libav
libavcodec_dep = dependency('libavcodec', required: get_option('libav'))
libavformat_dep = dependency('libavformat', required: get_option('libav'))
libswscale_dep = dependency('libswscale', required: get_option('libav'))
libavutil_dep = dependency('libavutil', required: get_option('libav'))
libswresample_dep = dependency('libswresample', required: get_option('libav'))
libavdevice_dep = dependency('libavdevice', required: get_option('libav'))
libav_found = libavcodec_dep.found() and libavformat_dep.found() and libswscale_dep.found()
libav_deps = []
if libav_found
  libav_deps = [libavcodec_dep, libavformat_dep, libswscale_dep,
                libavutil_dep, libswresample_dep, libavdevice_dep]
  add_project_arguments('-DICL_HAVE_LIBAV', language: 'cpp')
endif

# ZeroMQ
zmq_dep = dependency('libzmq', required: get_option('zmq'))
if zmq_dep.found()
  add_project_arguments('-DICL_HAVE_ZMQ', language: 'cpp')
endif

# V4L2
v4l_dep = dependency('libv4l2', required: get_option('v4l'))
if v4l_dep.found()
  add_project_arguments('-DICL_HAVE_VIDEODEV', language: 'cpp')
endif

# Bullet
bullet_dep = dependency('bullet', required: get_option('bullet'))
bullet_found = bullet_dep.found()
if bullet_found
  add_project_arguments('-DICL_HAVE_BULLET', language: 'cpp')
endif

# PCL
pcl_dep = dependency('pcl_common', required: get_option('pcl'))
if pcl_dep.found()
  add_project_arguments('-DICL_HAVE_PCL', language: 'cpp')
endif

# Apple Accelerate framework
accelerate_dep = dependency('', required: false)
if host_machine.system() == 'darwin'
  accelerate_dep = dependency('Accelerate', required: false)
  if accelerate_dep.found()
    add_project_arguments('-DICL_HAVE_ACCELERATE', '-DACCELERATE_NEW_LAPACK', language: 'cpp')
  endif
endif

# IPP (disabled by default on macOS, manual paths needed)
ipp_dep = dependency('', required: false)
# TODO: custom IPP detection if needed

# MKL
mkl_dep = dependency('', required: false)
# TODO: custom MKL detection if needed

# libdc1394
libdc_dep = dependency('libdc1394-2', required: get_option('libdc'))
if libdc_dep.found()
  add_project_arguments('-DICL_HAVE_LIBDC', language: 'cpp')
endif

# libfreenect
freenect_dep = dependency('libfreenect', required: get_option('libfreenect'))
if freenect_dep.found()
  add_project_arguments('-DICL_HAVE_LIBFREENECT', language: 'cpp')
endif

# libfreenect2
freenect2_dep = dependency('libfreenect2', required: get_option('libfreenect2'))
if freenect2_dep.found()
  add_project_arguments('-DICL_HAVE_LIBFREENECT2', language: 'cpp')
endif

# Pylon
pylon_dep = dependency('', required: false)
# TODO: custom Pylon detection if needed

# Cycles
cycles_dep = dependency('', required: false)
cycles_found = false
_cycles_lib = meson.project_source_root() / '3rdparty/cycles/build/lib/libcycles_session.a'
if get_option('cycles').allowed() and fs.exists(_cycles_lib)
  cycles_found = true
  add_project_arguments('-DICL_HAVE_CYCLES', language: 'cpp')
endif

# ---- Config headers ----
# ICL code uses #ifdef, so we must only #define when true (not #define FOO 0).
conf_config = configuration_data()
conf_config.set('ICL_64BIT', 1)

# Helper: only set if condition is true (so #ifdef works)
_cfg = {
  'ICL_SYSTEM_APPLE': host_machine.system() == 'darwin',
  'ICL_SYSTEM_LINUX': host_machine.system() == 'linux',
  'ICL_SYSTEM_WINDOWS': host_machine.system() == 'windows',
  'ICL_HAVE_LIBZ': true,
  'ICL_HAVE_LIBJPEG': true,
  'ICL_HAVE_LIBPNG': true,
  'ICL_HAVE_IPP': ipp_dep.found(),
  'ICL_HAVE_MKL': mkl_dep.found(),
  'ICL_HAVE_OPENCL': opencl_dep.found(),
  'ICL_HAVE_EIGEN3': eigen_dep.found(),
  'ICL_HAVE_OPENCV': opencv_dep.found(),
  'ICL_HAVE_IMAGEMAGICK': imagemagick_dep.found(),
  'ICL_HAVE_LIBDC': libdc_dep.found(),
  'ICL_HAVE_VIDEODEV': v4l_dep.found(),
  'ICL_HAVE_ZMQ': zmq_dep.found(),
  'ICL_HAVE_QT': qt_found,
  'ICL_HAVE_OPENGL': qt_found,
  'ICL_HAVE_LIBFREENECT': freenect_dep.found(),
  'ICL_HAVE_PYLON': pylon_dep.found(),
  'ICL_HAVE_PCL': pcl_dep.found(),
  'ICL_HAVE_BULLET': bullet_found,
  'ICL_USE_SSE2': have_sse2 or have_neon,
  'ICL_USE_SSE3': have_sse2 or have_neon,
  'ICL_USE_SSSE3': have_sse2 or have_neon,
}

foreach key, val : _cfg
  if val
    conf_config.set(key, 1)
  endif
endforeach

# Generated config header include directory.
# include_directories('.') from root gives both source_root and build_root.
# Config headers are generated in icl/utils/ subdir, so they appear at
# build_root/icl/utils/ICLConfig.h, matching #include <icl/utils/ICLConfig.h>.
gen_inc = include_directories('.')

# ---- Modules: libraries (strict dependency order) ----
# Demo/app/example builds happen below, after all libraries are defined.
_build_demos = get_option('demos')
_build_apps = get_option('apps')
_build_examples = get_option('examples')

subdir('icl/utils')
subdir('icl/math')
subdir('icl/core')
subdir('icl/filter')
subdir('icl/io')
subdir('icl/cv')
if qt_found
  subdir('icl/qt')
endif
subdir('icl/geom')
subdir('icl/markers')
if bullet_found and qt_found
  subdir('icl/physics')
endif
if qt_found
  subdir('icl/geom2')
endif

# ---- Demos, apps, examples (after all libraries are defined) ----
if _build_demos or _build_apps or _build_examples
  subdir('icl/utils/targets')
  subdir('icl/math/targets')
  subdir('icl/core/targets')
  subdir('icl/filter/targets')
  subdir('icl/io/targets')
  subdir('icl/cv/targets')
  if qt_found
    subdir('icl/qt/targets')
  endif
  subdir('icl/geom/targets')
  if qt_found
    subdir('icl/geom2/targets')
  endif
  subdir('icl/markers/targets')
  if bullet_found and qt_found
    subdir('icl/physics/targets')
  endif
endif

# ---- Tests ----
if get_option('tests')
  subdir('tests')
endif

# ---- Benchmarks ----
if get_option('benchmarks')
  subdir('benchmarks')
endif

# ---- Symlink all executables into build/bin/ ----
custom_target('symlink-bin',
  command: [find_program('scripts/symlink-bin.sh'), meson.project_build_root()],
  output: 'symlink-bin.stamp',
  build_always_stale: true,
  build_by_default: true,
)

# ---- Build summary ----
_simd_str = 'none'
if have_sse2
  _simd_str = 'SSE2/SSE3/SSSE3'
elif have_neon
  _simd_str = 'NEON via sse2neon'
endif

summary({
  'Build type': get_option('buildtype'),
  'C++ standard': 'C++20',
  'SIMD': _simd_str,
  'Modules': 'utils math core filter io cv' +
             (qt_found ? ' qt' : '') +
             ' geom markers' +
             (bullet_found and qt_found ? ' physics' : ''),
}, section: 'Build')

summary({
  'Qt6': qt_found,
  'OpenCV': opencv_dep.found(),
  'Eigen3': eigen_dep.found(),
  'OpenCL': opencl_dep.found(),
  'ImageMagick': imagemagick_dep.found(),
  'FFmpeg': libav_found,
  'Bullet': bullet_found,
  'Accelerate': accelerate_dep.found(),
}, section: 'Dependencies')
