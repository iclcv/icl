icl_check_external_package(XINE "xine.h;xine/xineutils.h" xine lib include /usr HAVE_XINE_COND TRUE)
if(HAVE_XINE_COND)
  set(XINE_LIBS_l xine)
endif()
