# ICL Math module

math_headers = files(
  'BlasOps.h',
  'DynMatrix.h',
  'DynMatrixBase.h',
  'DynMatrixUtils.h',
  'DynVector.h',
  'FFTException.h',
  'FFTOps.h',
  'FFTUtils.h',
  'FixedMatrix.h',
  'FixedVector.h',
  'GraphCutter.h',
  'HomogeneousMath.h',
  'Homography2D.h',
  'KDTree.h',
  'KMeans.h',
  'LLM.h',
  'LapackOps.h',
  'LeastSquareModelFitting.h',
  'LeastSquareModelFitting2D.h',
  'LevenbergMarquardtFitter.h',
  'LinearTransform1D.h',
  'Math.h',
  'MathFunctions.h',
  'MathOps.h',
  'MatrixSubRectIterator.h',
  'Octree.h',
  'PCLKdtree.h',
  'PCLOctree.h',
  'PolynomialRegression.h',
  'PolynomialSolver.h',
  'Projective4PointTransform.h',
  'QuadTree.h',
  'RansacFitter.h',
  'SOM.h',
  'SOM2D.h',
  'SimdCompat.h',
  'SimplexOptimizer.h',
  'StochasticOptimizer.h',
  'StraightLine2D.h',
)

math_sources = files(
  'BlasOps.cpp',
  'BlasOps_Cpp.cpp',
  'DynMatrix.cpp',
  'DynMatrixUtils.cpp',
  'DynVector.cpp',
  'FFTOps.cpp',
  'FFTOps_Cpp.cpp',
  'FFTUtils.cpp',
  'FixedMatrix.cpp',
  'GraphCutter.cpp',
  'HomogeneousMath.cpp',
  'Homography2D.cpp',
  'KDTree.cpp',
  'LLM.cpp',
  'LapackOps.cpp',
  'LapackOps_Cpp.cpp',
  'LevenbergMarquardtFitter.cpp',
  'MathOps.cpp',
  'MathOps_Cpp.cpp',
  'PolynomialRegression.cpp',
  'Projective4PointTransform.cpp',
  'SOM.cpp',
  'SimplexOptimizer.cpp',
  'StochasticOptimizer.cpp',
  'StraightLine2D.cpp',
)

# Conditional backend sources
if accelerate_dep.found()
  math_sources += files(
    'BlasOps_Accelerate.cpp',
    'FFTOps_Accelerate.cpp',
    'LapackOps_Accelerate.cpp',
  )
endif

if mkl_dep.found()
  math_sources += files(
    'BlasOps_Mkl.cpp',
    'FFTOps_Mkl.cpp',
    'LapackOps_Mkl.cpp',
  )
endif

if eigen_dep.found()
  math_sources += files(
    'LapackOps_Eigen.cpp',
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

pkg.generate(icl_math_lib,
  name: 'icl-math',
  description: 'ICL Math module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/math/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
