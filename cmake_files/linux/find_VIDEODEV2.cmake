if(${ICL_XDEP_VIDEODEV2_ON})
  #TODO special check for videodev 2
  if(EXISTS /usr/include/linux/videodev2.h 
      AND EXISTS /usr/include/sys/ioctl.h 
      AND EXISTS /usr/include/sys/mman.h 
      AND EXISTS /usr/include/fcntl.h)
    message(STATUS "VIDEODEV2 detected: TRUE")
    set(HAVE_VIDEODEV2_COND TRUE)
    add_definitions( -DHAVE_VIDEODEV2)
  else()
    set(HAVE_VIDEODEV2_COND FALSE)
    message(STATUS "VIDEODEV2 detected: FALSE")
  endif()
else()
  message(STATUS "VIDEODEV2 detected: FALSE")
endif()
