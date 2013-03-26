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
#ifdef HAVE_OPENCL
#include <ICLUtils/CLKernel.h>
#include <iostream>
#include <sstream>
//#include <ICLMath/FixedVector.h>
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
			void apply(int w, int h, int c) throw (CLKernelException) {
				cl::NDRange range;
				if (h > 0)
				if (c > 0)
				range = cl::NDRange(w, h, c);
				else
				range = cl::NDRange(w, h);
				else
				range = cl::NDRange(w);
				try {
					cmdQueue.enqueueNDRangeKernel(kernel, cl::NullRange, range,
							cl::NullRange);
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

		};
		CLKernel::CLKernel(const string &id, cl::Program & program,
				cl::CommandQueue& cmdQueue) throw (CLKernelException) {
			impl = new Impl(program, cmdQueue, id);

		}
		void CLKernel::apply(int w, int h, int c) throw (CLKernelException) {
			impl->apply(w, h, c);
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


		template<class T>
		void CLKernel::setArg(const unsigned idx, const T &value)
		throw (CLKernelException) {
			impl->setArg(idx, value);
		}
		template<>
		void CLKernel::setArg<CLBuffer>(const unsigned idx, const CLBuffer &value)
		throw (CLKernelException) {
			impl->setArg(idx, value.getBuffer());
		}

#define INST_TEMPL(T) \
	template void CLKernel::setArg<T>(const unsigned int idx, const T &value) throw (CLKernelException);

		INST_TEMPL(int);
		INST_TEMPL(unsigned int);
		INST_TEMPL(long);
		INST_TEMPL(unsigned long);
		INST_TEMPL(CLBuffer);
		INST_TEMPL(float);
		INST_TEMPL(double);
		INST_TEMPL(char);
		INST_TEMPL(unsigned char);
		INST_TEMPL(cl_float4);
//		INST_TEMPL(FixedColVector);



	}
}
#endif
