
#pragma once

#ifdef ICL_HAVE_OPENCL

#ifdef ICL_SYSTEM_APPLE
  #define GL_SILENCE_DEPRECATION
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

#endif // ICL_HAVE_OPENCL
