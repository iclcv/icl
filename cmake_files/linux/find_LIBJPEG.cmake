icl_check_external_package(LIBJPEG jpeglib.h jpeg lib include /usr HAVE_LIBJPEG_COND TRUE)

if(HAVE_LIBJPEG_COND)
  set(LIBJPEG_LIBS_l jpeg)
endif()