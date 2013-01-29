icl_simple_check_external_package(OPENCL "CL/cl.h;CL/cl.hpp" OpenCL)

if(HAVE_OPENCL_COND)
  add_definitions( -DCL_USE_DEPRECATED_OPENCL_1_1_APIS)
endif()
