#http://www.ijg.org/files/jpegsrc.v8b.tar.gz
#./configure --disable-static, make -j2, sudo make install
icl_check_external_package(LIBJPEG jpeglib.h libjpeg.dylib lib include /usr/local HAVE_LIBJPEG_COND TRUE)

if(HAVE_LIBJPEG_COND)
  set(LIBJPEG_LIBS_l jpeg)
endif()