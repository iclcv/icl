icl_check_external_package(MKL "mkl_types.h;mkl_cblas.h" "libmkl_intel_lp64.dylib;libmkl_intel_thread.dylib;libmkl_core.dylib" lib/em64t include _/opt/intel/Compiler/11.1/080/Frameworks/mkl HAVE_MKL_COND)
if(HAVE_MKL_COND)
  set(MKL_LIBS_l mkl_intel_lp64 mkl_intel_thread mkl_core)
endif()