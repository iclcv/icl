
#pragma once

#ifdef ICL_HAVE_OPENCL

#ifdef ICL_SYSTEM_APPLE
  // macOS uses OpenCL framework headers
  #include <OpenCL/opencl.h>
#else

#ifdef ICL_HAVE_CL2HPP

#define CL_HPP_ENABLE_EXCEPTIONS //enables openCL error catching
#define CL_HPP_TARGET_OPENCL_VERSION 120
// The following is needed to maintain compatibility to 1.2
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_SIZE_T_COMPATIBILITY

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY

#include <CL/cl2.hpp>
#include <CL/cl.h>

#else

#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
#include <CL/cl.h>
#include <CL/cl.hpp>
#endif
#endif // ICL_SYSTEM_APPLE
#endif // ICL_HAVE_OPENCL
