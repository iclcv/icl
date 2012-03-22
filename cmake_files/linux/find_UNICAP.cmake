icl_check_external_package(UNICAP unicap.h unicap lib include/unicap /usr HAVE_UNICAP_COND TRUE)
if(HAVE_UNICAP_COND)
  set(UNICAP_LIBS_l unicap rt)
  #additional definition
  add_definitions(-DUNICAP_FLAGS_NOT_AS_ENUM)
endif()