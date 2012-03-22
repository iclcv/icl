icl_check_external_package(IMAGEMAGICK Magick++.h GraphicsMagick++ lib include/GraphicsMagick /usr HAVE_IMAGEMAGICK_COND TRUE)
if(HAVE_IMAGEMAGICK_COND)
  set(IMAGEMAGICK_LIBS_l GraphicsMagick++)
endif()