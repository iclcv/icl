#ftp://ftp.graphicsmagick.org/pub/GraphicsMagick/GraphicsMagick-LATEST.tar.gz
#./configure --enable-shared, male -j2, sudo make install
icl_check_external_package(IMAGEMAGICK Magick++.h libGraphicsMagick++.la lib include/GraphicsMagick /usr/local HAVE_IMAGEMAGICK_COND TRUE)

if(HAVE_IMAGEMAGICK_COND)
  set(IMAGEMAGICK_LIBS_l GraphicsMagick++)
endif()
