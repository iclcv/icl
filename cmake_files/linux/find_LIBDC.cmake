icl_check_external_package(LIBDC dc1394.h dc1394 lib include/dc1394 /usr HAVE_LIBDC_COND TRUE)
if(HAVE_LIBDC_COND)
  set(LIBDC_LIBS_l dc1394)
endif()
