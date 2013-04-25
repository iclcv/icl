/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLKernel.h                       **
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
#pragma once
#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
#include <CL/cl.hpp>
#include <string.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLException.h>
#include <ICLUtils/FixedArray.h>
using namespace std;
namespace icl {
	namespace utils {
		/// Wrapper for an OpenCL Kernels
		/**
		 Can only be created via CLProgram.
		 Kernel arguments can be set on two different ways.

		 example:

		 kernel.setArgs(inBuffer, outBuffer, inSize, outSize);

		 or:
		 kernel[0] = inBuffer;
		 kernel[1] = outBuffer;
		 kernel[2] = inSize;
		 kernel[3] = outSize;
		 **/
		class CLKernel {
		public:
			struct Impl;
		private:
			Impl *impl;

			CLKernel(const string &id, cl::Program & program, cl::CommandQueue& cmdQueue) throw (CLKernelException);
		public:
			void setArg(const unsigned idx, const unsigned int &value) throw (CLKernelException);
			void setArg(const unsigned idx, const int &value) throw (CLKernelException);
			void setArg(const unsigned idx, const short &value) throw (CLKernelException);
			void setArg(const unsigned idx, const long &value) throw (CLKernelException);
			void setArg(const unsigned idx, const unsigned long &value) throw (CLKernelException);
			void setArg(const unsigned idx, const float &value) throw (CLKernelException);
			void setArg(const unsigned idx, const double &value) throw (CLKernelException);
			void setArg(const unsigned idx, const char &value) throw (CLKernelException);
			void setArg(const unsigned idx, const unsigned char &value) throw (CLKernelException);
			void setArg(const unsigned idx, const FixedArray<float,4> &value) throw (CLKernelException);
			void setArg(const unsigned idx, const FixedArray<float,3> &value) throw (CLKernelException);
			void setArg(const unsigned idx, const CLBuffer &value) throw (CLKernelException);

			/// sets arguments of the kernel
			template<typename A>
			void setArgs(const A &value) throw (CLKernelException) {
				setArg(0, value);
			}
			template<typename A, typename B>
			void setArgs(const A &valueA, const B &valueB) throw (CLKernelException) {
				setArgs(valueA);
				setArg(1, valueB);
			}
			template<typename A, typename B, typename C>
			void setArgs(const A &valueA, const B &valueB, const C &valueC) throw (CLKernelException) {
				setArgs(valueA, valueB);
				setArg(2, valueC);
			}
			template<typename A, typename B, typename C, typename D>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC);
				setArg(3, valueD);
			}
			template<typename A, typename B, typename C, typename D, typename E>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD);
				setArg(4, valueE);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE);
				setArg(5, valueF);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF);
				setArg(6, valueG);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG);
				setArg(7, valueH);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH, const I &valueI) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH);
				setArg(8, valueI);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH, const I &valueI, const J &valueJ) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI);
				setArg(9, valueJ);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH, const I &valueI, const J &valueJ, const K &valueK) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI, valueJ);
				setArg(10, valueK);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH, const I &valueI, const J &valueJ, const K &valueK, const L &valueL) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI, valueJ, valueK);
				setArg(11, valueL);
			}

			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH, const I &valueI, const J &valueJ, const K &valueK, const L &valueL, const M &valueM) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI, valueJ, valueK, valueL);
				setArg(12, valueM);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH, const I &valueI, const J &valueJ, const K &valueK, const L &valueL, const M &valueM, const N &valueN) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI, valueJ, valueK, valueL, valueM);
				setArg(13, valueN);
			}
			template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
			void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD, const E &valueE, const F &valueF, const G &valueG, const H &valueH, const I &valueI, const J &valueJ, const K &valueK, const L &valueL, const M &valueM, const N &valueN, const O &valueO) throw (CLKernelException) {
				setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI, valueJ, valueK, valueL, valueM, valueN);
				setArg(14, valueO);
			}



			~CLKernel();
			CLKernel();
			CLKernel(const CLKernel &other);
			CLKernel const& operator=(CLKernel const& other);
			friend class CLProgram;

			struct Arg {
				CLKernel &k;
				int idx;
				Arg(CLKernel &k, int idx):k(k),idx(idx) {}
				template<class T>
				void operator=(const T &t) {
					k.setArg(idx,t);
				}
			};

			Arg operator[](int idx) {return Arg(*this,idx);}

			/// executes the kernel with given dimensions
			void apply(int gloW, int gloH = 0, int gloC = 0,
					int locW = 0, int locH = 0, int locC = 0) throw (CLKernelException);
		};
	}
}

#endif
