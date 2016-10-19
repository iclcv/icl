
#pragma once

#ifdef ICL_HAVE_OPENCL

#ifdef ICL_HAVE_CL2HPP

#define CL_HPP_ENABLE_EXCEPTIONS //enables openCL error catching
#define CL_HPP_TARGET_OPENCL_VERSION 120
// The following is needed to maintain compatibility to 1.2
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_SIZE_T_COMPATIBILITY

//#define CL_HPP_NO_STD_UNIQUE_PTR

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY

//#define CL_HPP_CL_1_2_DEFAULT_BUILD

#include <CL/cl2.hpp>
//using namespace  cl::compatibility;
#include <CL/cl.h>

#else

#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
#include <CL/cl.h>
#include <CL/cl.hpp>
#endif
#endif // ICL_HAVE_CL2HPP
