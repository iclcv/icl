// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Viktor Losing, Christof Elbrechter

#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLKernel.h>
#include <ICLUtils/CLIncludes.h>

#include <cstring>

namespace icl {
  namespace utils {

    struct CLKernel::Impl {
      cl_kernel kernel;
      cl_command_queue cmdQueue;

      Impl() : kernel(nullptr), cmdQueue(nullptr) {}

      Impl(Impl &other) : kernel(other.kernel), cmdQueue(other.cmdQueue) {
        if (kernel) clRetainKernel(kernel);
        if (cmdQueue) clRetainCommandQueue(cmdQueue);
      }

      Impl(cl_program program, cl_command_queue cmdQueue, const std::string &id)
        : kernel(nullptr), cmdQueue(cmdQueue)
      {
        if (this->cmdQueue) clRetainCommandQueue(this->cmdQueue);
        cl_int err = CL_SUCCESS;
        kernel = clCreateKernel(program, id.c_str(), &err);
        if (err != CL_SUCCESS) {
          throw CLKernelException(CLException::getMessage(err, "clCreateKernel"));
        }
      }

      ~Impl() {
        if (kernel) clReleaseKernel(kernel);
        if (cmdQueue) clReleaseCommandQueue(cmdQueue);
      }

      void finish() {
        cl_int err = clFinish(cmdQueue);
        if (err != CL_SUCCESS) {
          throw CLKernelException(CLException::getMessage(err, "clFinish"));
        }
      }

      void apply(int gloW, int gloH = 0, int gloC = 0,
                 int locW = 0, int locH = 0, int locC = 0) {
        cl_uint work_dim = 1;
        size_t globalWorkSize[3] = {};
        size_t localWorkSize[3] = {};
        bool hasLocal = false;

        if (gloH > 0) {
          if (gloC > 0) {
            work_dim = 3;
            globalWorkSize[0] = static_cast<size_t>(gloW);
            globalWorkSize[1] = static_cast<size_t>(gloH);
            globalWorkSize[2] = static_cast<size_t>(gloC);
          } else {
            work_dim = 2;
            globalWorkSize[0] = static_cast<size_t>(gloW);
            globalWorkSize[1] = static_cast<size_t>(gloH);
          }
        } else {
          work_dim = 1;
          globalWorkSize[0] = static_cast<size_t>(gloW);
        }

        if (locW > 0) {
          hasLocal = true;
          localWorkSize[0] = static_cast<size_t>(locW);
          if (locH > 0) {
            localWorkSize[1] = static_cast<size_t>(locH);
            if (locC > 0) {
              localWorkSize[2] = static_cast<size_t>(locC);
            }
          }
        }

        cl_int err = clEnqueueNDRangeKernel(
          cmdQueue, kernel, work_dim,
          nullptr, // global_work_offset
          globalWorkSize,
          hasLocal ? localWorkSize : nullptr,
          0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
          throw CLKernelException(
            CLException::getMessage(err, "clEnqueueNDRangeKernel"));
        }
      }

      template<class T>
      void setArg(const unsigned idx, const T &value) {
        cl_int err = clSetKernelArg(kernel, idx, sizeof(T), &value);
        if (err != CL_SUCCESS) {
          throw CLKernelException(
            CLException::getMessage(err, "clSetKernelArg"));
        }
      }

      void setArgBuffer(const unsigned idx, cl_mem buf) {
        cl_int err = clSetKernelArg(kernel, idx, sizeof(cl_mem), &buf);
        if (err != CL_SUCCESS) {
          throw CLKernelException(
            CLException::getMessage(err, "clSetKernelArg(cl_mem)"));
        }
      }

      void setArg(const unsigned idx, const CLKernel::LocalMemory &value) {
        cl_int err = clSetKernelArg(kernel, idx, value.size, nullptr);
        if (err != CL_SUCCESS) {
          throw CLKernelException(
            CLException::getMessage(err, "clSetKernelArg(LocalMemory)"));
        }
      }

      void setArgFloat3(const unsigned idx, const FixedArray<float,3> &value) {
        // OpenCL float3 is stored as float4 (16 bytes)
        cl_float4 v;
        v.s[0] = value[0];
        v.s[1] = value[1];
        v.s[2] = value[2];
        v.s[3] = 0.0f;
        cl_int err = clSetKernelArg(kernel, idx, sizeof(cl_float4), &v);
        if (err != CL_SUCCESS) {
          throw CLKernelException(
            CLException::getMessage(err, "clSetKernelArg(float3)"));
        }
      }

      void setArgFloat4(const unsigned idx, const FixedArray<float,4> &value) {
        cl_float4 v;
        v.s[0] = value[0];
        v.s[1] = value[1];
        v.s[2] = value[2];
        v.s[3] = value[3];
        cl_int err = clSetKernelArg(kernel, idx, sizeof(cl_float4), &v);
        if (err != CL_SUCCESS) {
          throw CLKernelException(
            CLException::getMessage(err, "clSetKernelArg(float4)"));
        }
      }
    };

    CLKernel::CLKernel(const std::string &id, cl_program program,
                       cl_command_queue cmdQueue) {
      impl = new Impl(program, cmdQueue, id);
    }

    void CLKernel::apply(int gloW, int gloH, int gloC,
                         int locW, int locH, int locC) {
      impl->apply(gloW, gloH, gloC, locW, locH, locC);
    }

    void CLKernel::finish() {
      impl->finish();
    }

    CLKernel::~CLKernel() {
      delete impl;
    }

    CLKernel::CLKernel() {
      impl = new Impl();
    }

    CLKernel::CLKernel(const CLKernel &other) {
      impl = new Impl(*(other.impl));
    }

    CLKernel const& CLKernel::operator=(CLKernel const& other) {
      if (this != &other) {
        // Release old handles
        if (impl->kernel) clReleaseKernel(impl->kernel);
        if (impl->cmdQueue) clReleaseCommandQueue(impl->cmdQueue);
        // Copy and retain new handles
        impl->kernel = other.impl->kernel;
        impl->cmdQueue = other.impl->cmdQueue;
        if (impl->kernel) clRetainKernel(impl->kernel);
        if (impl->cmdQueue) clRetainCommandQueue(impl->cmdQueue);
      }
      return *this;
    }

    void CLKernel::setArg(const unsigned idx, const unsigned int &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const int &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const short &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const long &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const unsigned long &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const float &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const double &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const char &value) {
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const unsigned char &value) {
      impl->setArg(idx, value);
    }

    void CLKernel::setArg(const unsigned idx, const CLBuffer &value) {
      cl_mem buf = value.getBuffer();
      impl->setArgBuffer(idx, buf);
    }

    void CLKernel::setArg(const unsigned idx, const CLImage2D &value) {
      cl_mem img = value.getImage2D();
      impl->setArgBuffer(idx, img);
    }

    void CLKernel::setArg(const unsigned idx, const LocalMemory &value) {
      impl->setArg(idx, value);
    }

    void CLKernel::setArg(const unsigned idx, const FixedArray<float,4> &value) {
      impl->setArgFloat4(idx, value);
    }

    void CLKernel::setArg(const unsigned idx, const FixedArray<float,3> &value) {
      impl->setArgFloat3(idx, value);
    }
  }
}
#endif
