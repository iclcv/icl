
#pragma once

#ifdef ICL_HAVE_OPENCL
#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
//#include <CL/cl.h>
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#undef CL_VERSION_1_2
#include <CL/cl.hpp>
#endif
