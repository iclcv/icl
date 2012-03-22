icl_check_external_package(OPENSURF "opensurf/fasthessian.h;opensurf/integral.h;opensurf/surf.h;opensurf/surflib.h;opensurf/utils.h" surf lib include /vol/nivision HAVE_OPENSURF_COND TRUE)
if(HAVE_OPENSURF_COND)
  set(OPENSURF_LIBS_l surf)
endif()