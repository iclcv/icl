#http://damien.douxchamps.net/ieee1394/libdc1394/download/
#svn co https://libdc1394.svn.sourceforge.net/svnroot/libdc1394 libdc1394
#autoreconf -i -s, ./configure ,make -j2,sudo make install
icl_check_external_package(LIBDC dc1394.h libdc1394.dylib lib include/dc1394 _/usr/local HAVE_LIBDC_COND TRUE)

if(HAVE_LIBDC_COND)
  set(LIBDC_LIBS_l dc1394)
endif()