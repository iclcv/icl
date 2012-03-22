if(${ICL_XDEP_VIDEODEV_ON})
  #TODO special check for videodev
  if(EXISTS /usr/include/linux/videodev.h 
      AND EXISTS /usr/include/sys/ioctl.h 
      AND EXISTS /usr/include/sys/mman.h 
      AND EXISTS /usr/include/fcntl.h)
    message(STATUS "VIDEODEV detected: TRUE")
    set(HAVE_VIDEODEV_COND TRUE)
    add_definitions( -DHAVE_VIDEODEV)
  else()
    # check new package: external package !!
    # libname libv4l1.so
    # header: /usr/include/libv4l1-videodev.h
    # /usr/include/libv4l1.h
    # set: an extra definition
    #add_definitions( -DUSE_NEW_VIDEO_DEV_1_HEADERS)
    set(HAVE_VIDEODEV_COND FALSE)
    message(STATUS "VIDEODEV detected: FALSE")
  endif()
else()
  message(STATUS "VIDEODEV detected: FALSE")
endif()


