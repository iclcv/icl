
#pragma once

#ifdef ICL_HAVE_OPENCL
#include <CL/cl.h>

#if CL_VERSION_2_0
#define CL_HPP_ENABLE_EXCEPTIONS //enables openCL error catching
#define CL_HPP_TARGET_OPENCL_VERSION 200
// The following is needed to maintain compatibility to 1.2
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_SIZE_T_COMPATIBILITY
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#include <CL/cl2.hpp>
#else
#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
#include <CL/cl.hpp>
#endif
#endif // ICL_HAVE_OPENCL
