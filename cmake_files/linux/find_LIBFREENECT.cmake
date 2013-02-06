set(on ${ICL_XDEP_LIBFREENECT_ON})

icl_check_external_package(LIBFREENECT libfreenect.h freenect lib include/libfreenect FALSE TRUE)

if(NOT HAVE_LIBFREENECT_COND AND ${on})
  set(ICL_XDEP_LIBFREENECT_ON TRUE)
  icl_check_external_package(LIBFREENECT libfreenect.h freenect lib include TRUE TRUE)
endif()