/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLKernel.cpp                     **
** Module : ICLUtils                                               **
** Authors: Viktor Losing                                          **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/
#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLKernel.h>
#include <iostream>
#include <sstream>
#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.h>
#undef CL_VERSION_1_2
#include <CL/cl.hpp>

namespace icl {
  namespace utils {

    struct CLKernel::Impl {
      cl::Kernel kernel;
      cl::CommandQueue cmdQueue;
      Impl(){}
	  Impl(Impl& other):cmdQueue(other.cmdQueue){
        kernel = other.kernel;
      }
      Impl(cl::Program& program, cl::CommandQueue& cmdQueue, const string &id)
        throw (CLKernelException):cmdQueue(cmdQueue) {
        try {
          kernel = cl::Kernel(program, id.c_str());
        } catch (cl::Error& error) {
          throw CLKernelException(CLException::getMessage(error.err(), error.what()));
        }
      }

	  void finish() throw (CLKernelException) {
		  try {
			  cmdQueue.finish();
		  } catch (cl::Error& error) {
			throw CLKernelException(CLException::getMessage(error.err(), error.what()));
		  }
	  }

      void apply(int gloW, int gloH = 0, int gloC = 0,
                 int locW = 0, int locH = 0, int locC = 0) throw (CLKernelException){
        cl::NDRange globRange;
        if (gloH > 0)
          {
            if (gloC > 0)
              {
                globRange = cl::NDRange(gloW, gloH, gloC);
              }
            else
              {
                globRange = cl::NDRange(gloW, gloH);
              }
          }
        else
          {
            globRange = cl::NDRange(gloW);
          }

        cl::NDRange locRange = cl::NullRange;
        if (locW > 0)
          {
            if (locH > 0)
              {
                if (locC > 0){
                  locRange = cl::NDRange(locW, locH, locC);
                }
                else
                  {
                    locRange = cl::NDRange(locW, locH);
                  }
              }
            else
              {
                locRange = cl::NDRange(locW);
              }
          }
        try {
          cmdQueue.enqueueNDRangeKernel(kernel, cl::NullRange, globRange,
                                        locRange);
        } catch (cl::Error& error) {
          throw CLKernelException(CLException::getMessage(error.err(), error.what()));
        }
      }

      template<class T>
      void setArg(const unsigned idx, const T &value)
        throw (CLKernelException) {
        try
          {
            kernel.setArg(idx, value);
          } catch (cl::Error& error) {
          throw CLKernelException(CLException::getMessage(error.err(), error.what()));
        }
      }

      void setArg(const unsigned idx, const CLKernel::LocalMemory &value)
        throw (CLKernelException) {
        try
          {
            kernel.setArg(idx, value.size, NULL);
          } catch (cl::Error& error) {
          throw CLKernelException(CLException::getMessage(error.err(), error.what()));
        }
      }

    };
    CLKernel::CLKernel(const string &id, cl::Program & program,
                       cl::CommandQueue& cmdQueue) throw (CLKernelException) {
      impl = new Impl(program, cmdQueue, id);

    }
    void CLKernel::apply(int gloW, int gloH, int gloC,
                         int locW, int locH, int locC) throw (CLKernelException){
      impl->apply(gloW, gloH, gloC, locW, locH, locC);
    }

	void CLKernel::finish() throw (CLKernelException) {
		impl->finish();
	}

    CLKernel::~CLKernel() {
      delete impl;
    }

    CLKernel::CLKernel(){
      impl = new Impl();
    }
    CLKernel::CLKernel(const CLKernel &other){
      impl = new Impl(*(other.impl));
    }

    CLKernel const& CLKernel::operator=(CLKernel const& other){
      impl->cmdQueue = other.impl->cmdQueue;
      impl->kernel = other.impl->kernel;
      return *this;
    }

    void CLKernel::setArg(const unsigned idx, const unsigned int &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const int &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const short &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const long &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const unsigned long &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const float &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const double &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const char &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
    void CLKernel::setArg(const unsigned idx, const unsigned char &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }

    void CLKernel::setArg(const unsigned idx, const CLBuffer &value) throw (CLKernelException){
      impl->setArg(idx, value.getBuffer());
    }

    void CLKernel::setArg(const unsigned idx, const CLImage2D &value) throw (CLKernelException){
      impl->setArg(idx, value.getImage2D());
    }

    void CLKernel::setArg(const unsigned idx, const LocalMemory &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }

    void CLKernel::setArg(const unsigned idx, const FixedArray<float,4> &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }

    void CLKernel::setArg(const unsigned idx, const FixedArray<float,3> &value) throw (CLKernelException){
      impl->setArg(idx, value);
    }
  }
}
#endif
