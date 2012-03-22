icl_check_external_package(OPENCL "cl.h;cl.hpp" "OpenCL" lib include/CL /usr HAVE_OPENCL_COND TRUE)
if(HAVE_OPENCL_COND)
  set(OPENCL_LIBS_l OpenCL)
endif()
