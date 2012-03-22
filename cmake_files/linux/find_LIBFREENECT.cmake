icl_check_external_package(LIBFREENECT libfreenect.h freenect lib include/libfreenect /usr HAVE_LIBFREENECT_COND FALSE)

if(HAVE_LIBFREENECT_COND)
  set(LIBFREENECT_LIBS_l freenect)
else()
  message(STATUS "LIBFREENECT detected: FALSE")
endif()
