# ICL Math module

# Public headers (installed).  The Blas/Lapack/FFT/MathOps backend-dispatch
# facades live under detail/<family>/ and are intentionally NOT installed —
# they are implementation details.  Public consumers reach backend ops through
# DynMatrix / DynMatrixUtils / FFTUtils / FixedMatrix etc., which dispatch
# internally.
math_headers = files(
  'DynMatrix.h',
  'DynMatrixBase.h',
  'DynMatrixUtils.h',
  'DynVector.h',
  'FixedMatrix.h',
  'FixedVector.h',
  'GraphCutter.h',
  'KMeans.h',
  'LLM.h',
  'LeastSquareModelFitting.h',
  'LeastSquareModelFitting2D.h',
  'LevenbergMarquardtFitter.h',
  'Math.h',
  'MathFunctions.h',
  'MatrixSubRectIterator.h',
  'PolynomialRegression.h',
  'PolynomialSolver.h',
  'RansacFitter.h',
  'SOM.h',
  'SOM2D.h',
  'SimdCompat.h',
  'SimplexOptimizer.h',
  'StochasticOptimizer.h',
  'StraightLine2D.h',
)

math_fft_headers = files(
  'fft/FFTException.h',
  'fft/FFTUtils.h',
)

math_tree_headers = files(
  'tree/KDTree.h',
  'tree/Octree.h',
  'tree/PCLKdtree.h',
  'tree/PCLOctree.h',
  'tree/QuadTree.h',
)

math_transform_headers = files(
  'transform/HomogeneousMath.h',
  'transform/Homography2D.h',
  'transform/LinearTransform1D.h',
  'transform/Projective4PointTransform.h',
  'transform/StraightLine2D.h',
)

# Always-compiled sources (root-level public API impls).
math_sources = files(
  'DynMatrix.cpp',
  'DynMatrixUtils.cpp',
  'DynVector.cpp',
  'fft/FFTUtils.cpp',
  'FixedMatrix.cpp',
  'GraphCutter.cpp',
  'transform/HomogeneousMath.cpp',
  'transform/Homography2D.cpp',
  'tree/KDTree.cpp',
  'LLM.cpp',
  'LevenbergMarquardtFitter.cpp',
  'PolynomialRegression.cpp',
  'transform/Projective4PointTransform.cpp',
  'SOM.cpp',
  'SimplexOptimizer.cpp',
  'StochasticOptimizer.cpp',
  'transform/StraightLine2D.cpp',
)

# Backend-dispatch facades (private to libicl-math).  Each family has a
# facade header + main .cpp + per-backend dispatch cpps gated by the
# corresponding backend dep.
math_sources += files(
  'detail/blas/BlasOps.cpp',
  'detail/blas/BlasOps_Cpp.cpp',
  'detail/fft/FFTOps.cpp',
  'detail/fft/FFTOps_Cpp.cpp',
  'detail/lapack/LapackOps.cpp',
  'detail/lapack/LapackOps_Cpp.cpp',
  'detail/mathops/MathOps.cpp',
  'detail/mathops/MathOps_Cpp.cpp',
)

# Conditional backend sources
if accelerate_dep.found()
  math_sources += files(
    'detail/blas/BlasOps_Accelerate.cpp',
    'detail/fft/FFTOps_Accelerate.cpp',
    'detail/lapack/LapackOps_Accelerate.cpp',
  )
endif

if mkl_dep.found()
  math_sources += files(
    'detail/blas/BlasOps_Mkl.cpp',
    'detail/fft/FFTOps_Mkl.cpp',
    'detail/lapack/LapackOps_Mkl.cpp',
  )
endif

if eigen_dep.found()
  math_sources += files(
    'detail/lapack/LapackOps_Eigen.cpp',
  )
endif

math_deps = [icl_utils_dep, accelerate_dep, mkl_dep, eigen_dep]

icl_math_lib = shared_library('icl-math',
  math_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: math_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/math.h',
  install: true,
)

icl_math_dep = declare_dependency(
  link_with: icl_math_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: math_deps,
)

install_headers(math_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'math',
)

install_headers(math_fft_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'math' / 'fft',
)

install_headers(math_tree_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'math' / 'tree',
)

install_headers(math_transform_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'math' / 'transform',
)

pkg.generate(icl_math_lib,
  name: 'icl-math',
  description: 'ICL Math module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/math/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
