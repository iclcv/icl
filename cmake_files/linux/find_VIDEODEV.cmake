set(on ${ICL_XDEP_VIDEODEV_ON})
icl_check_external_package(VIDEODEV "linux/videodev.h;linux/videodev2.h;sys/ioctl.h;sys/mman.h;fcntl.h" "" lib include FALSE)
if(NOT HAVE_VIDEODEV_COND AND ${on})
  set(ICL_XDEP_VIDEODEV_ON TRUE)
  icl_simple_check_external_package(VIDEODEV "libv4l1-videodev.h;libv4l1.h;libv4l2.h;libv4lconvert.h" "v4l1;v4l2")
  if(HAVE_VIDEODEV_COND)
    add_definitions( -DICL_USE_VIDEODEV_LIB)
  endif()
endif()



