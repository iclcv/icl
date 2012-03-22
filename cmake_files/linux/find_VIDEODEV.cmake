if(${ICL_XDEP_VIDEODEV_ON})
  #TODO special check for videodev
  if(EXISTS /usr/include/linux/videodev.h 
      AND EXISTS /usr/include/linux/videodev2.h 
      AND EXISTS /usr/include/sys/ioctl.h 
      AND EXISTS /usr/include/sys/mman.h 
      AND EXISTS /usr/include/fcntl.h)
    message(STATUS "VIDEODEV detected: TRUE")
    set(HAVE_VIDEODEV_COND TRUE)
    add_definitions( -DHAVE_VIDEODEV)
  else()
    icl_check_external_package(VIDEODEV "libv4l1-videodev.h;libv4l1.h;libv4l2.h;libv4lconvert.h" "v4l1;v4l2" lib include /usr HAVE_VIDEODEV_COND TRUE)
    if(HAVE_VIDEODEV_COND)
      add_definitions( -DICL_USE_VIDEODEV_LIB)
      set(VIDEODEV_LIBS_l v4l1 v4l2)
    else()
    endif()
  endif()
endif()



