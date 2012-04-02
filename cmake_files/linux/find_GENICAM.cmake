if(ICL_64BIT)
  set(GENICAM_LIB bin/Linux64_x64)
else()
  set(GENICAM_LIB bin/Linux32_i86)
endif()

icl_check_external_package(GENICAM "GenICam.h;GenApi/GenApi.h" GenApi_gcc40_v2_1 ${GENICAM_LIB} library/CPP/include TRUE TRUE)

# this is done automatically now!
#if(HAVE_GENICAM_COND)
#  set (GENICAM_LIBS_l ) #TODO:genicam libs)
#endif()
