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

#pragma once

#ifdef ICL_HAVE_OPENCL

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLImage2D.h>
#include <ICLUtils/CLException.h>
#include <ICLUtils/FixedArray.h>
#include <string>

/** \cond */
namespace cl{
  class CommandQueue;
  class Program;
}
/** \endcond */

namespace icl {
  namespace utils {
    /// Wrapper for an OpenCL Kernel
    /** A Kernel is a callable OpenCL function. CLKernel instances can
        only be created by CLProgram instances. Please refer to the CLProgram
        reference for further details.

        \section ARGS Kernel Arguments

        Before, a Kernel (aka an OpenCL function) is called, its
        arguments have to be set. This can either be done step by step
        using one of the overloaded CLKernel.setArg-methods or using
        the assignment operator or with the templated CLKernel::setArgs method.

        <code>
        kernel.setArgs(inBuffer, outBuffer, inSize, outSize);

        // or:
        kernel[0] = inBuffer;
        kernel[1] = outBuffer;
        kernel[2] = inSize;
        kernel[3] = outSize;
        **/
    class ICLUtils_API CLKernel {
      struct Impl; //!< internal implementation
      Impl *impl;  //!< internally used data

      /// private constructor (CLKernel instances can only be created by CLPrograms)
      CLKernel(const string &id, cl::Program & program,
               cl::CommandQueue& cmdQueue) throw (CLKernelException);

      public:

      /// struct that represents the dynamic local memory for a kernel
      struct LocalMemory {
        size_t size; //!< size of the dynamic local memory

        /// constructor
        LocalMemory(size_t size) : size(size) { }
      };

      /// Default constructor (creates a dummy instance)
      CLKernel();

      /// copy constructor (creates shallow copy)
      CLKernel(const CLKernel &other);

      /// assignment operator (creates shallow copy)
      CLKernel const& operator=(CLKernel const& other);

      /// Destructor
      ~CLKernel();

      /// executes kernel with given global and local coordinates
      /** gloW can be accessed in the kernel code by get_global_id(0), and so on */
      void apply(int gloW, int gloH = 0, int gloC = 0,
                 int locW = 0, int locH = 0, int locC = 0) throw (CLKernelException);

	  /// calls the finish-fkt. of opencl to wait until the command queue is done
	  void finish() throw (CLKernelException);

      /// for tight integration with the CLProgram class
      friend class CLProgram;
	  friend class CLDeviceContext;

      /// Overloaded Kernel argument setter for unsigned int values
      void setArg(const unsigned idx, const unsigned int &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for int values
      void setArg(const unsigned idx, const int &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for short values
      void setArg(const unsigned idx, const short &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for long values
      void setArg(const unsigned idx, const long &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for unsigned long values
      void setArg(const unsigned idx, const unsigned long &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for float values
      void setArg(const unsigned idx, const float &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for double values
      void setArg(const unsigned idx, const double &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for char values
      void setArg(const unsigned idx, const char &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for unsigned char values
      void setArg(const unsigned idx, const unsigned char &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for 4D vectors
      void setArg(const unsigned idx, const FixedArray<float,4> &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for 3D vectors
      void setArg(const unsigned idx, const FixedArray<float,3> &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for CLBuffer values (aka arrays/pointers)
      void setArg(const unsigned idx, const CLBuffer &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for CLImage2D values (aka arrays/pointers)
      void setArg(const unsigned idx, const CLImage2D &value) throw (CLKernelException);

      /// Overloaded Kernel argument setter for dynamic local memory
      void setArg(const unsigned idx, const LocalMemory &value) throw (CLKernelException);

      /// sets mutiple kernel arguments at once
      template<typename A>
      void setArgs(const A &value) throw (CLKernelException) {
        setArg(0, value);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B>
      void setArgs(const A &valueA, const B &valueB) throw (CLKernelException) {
        setArgs(valueA);
        setArg(1, valueB);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C>
      void setArgs(const A &valueA, const B &valueB, const C &valueC) throw (CLKernelException) {
        setArgs(valueA, valueB);
        setArg(2, valueC);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD) throw (CLKernelException) {
        setArgs(valueA, valueB, valueC);
        setArg(3, valueD);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE) throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD);
        setArg(4, valueE);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF) throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE);
        setArg(5, valueF);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG)
                   throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF);
        setArg(6, valueG);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH)
                   throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG);
        setArg(7, valueH);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI) throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH);
        setArg(8, valueI);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ) throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI);
        setArg(9, valueJ);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J, typename K>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ, const K &valueK)
                   throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI,
                valueJ);
        setArg(10, valueK);
      }
      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J, typename K, typename L>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ, const K &valueK, const L &valueL)
                   throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI,
                valueJ, valueK);
        setArg(11, valueL);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J, typename K, typename L,
               typename M>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ, const K &valueK, const L &valueL,
                   const M &valueM) throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI,
                valueJ, valueK, valueL);
        setArg(12, valueM);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J, typename K, typename L,
               typename M, typename N>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ, const K &valueK, const L &valueL,
                   const M &valueM, const N &valueN) throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI,
                valueJ, valueK, valueL, valueM);
        setArg(13, valueN);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J, typename K, typename L,
               typename M, typename N, typename O>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ, const K &valueK, const L &valueL,
                   const M &valueM, const N &valueN, const O &valueO)
                   throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI,
                valueJ, valueK, valueL, valueM, valueN);
        setArg(14, valueO);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J, typename K, typename L,
               typename M, typename N, typename O, typename P>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ, const K &valueK, const L &valueL,
                   const M &valueM, const N &valueN, const O &valueO, const P &valueP)
                   throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI,
                valueJ, valueK, valueL, valueM, valueN, valueP);
        setArg(15, valueP);
      }

      /// sets mutiple kernel arguments at once
      template<typename A, typename B, typename C, typename D, typename E, typename F,
               typename G, typename H, typename I, typename J, typename K, typename L,
               typename M, typename N, typename O, typename P, typename Q>
      void setArgs(const A &valueA, const B &valueB, const C &valueC, const D &valueD,
                   const E &valueE, const F &valueF, const G &valueG, const H &valueH,
                   const I &valueI, const J &valueJ, const K &valueK, const L &valueL,
                   const M &valueM, const N &valueN, const O &valueO, const P &valueP,
                   const Q &valueQ)
                   throw (CLKernelException) {
        setArgs(valueA, valueB, valueC, valueD, valueE, valueF, valueG, valueH, valueI,
                valueJ, valueK, valueL, valueM, valueN, valueP, valueQ);
        setArg(16, valueQ);
      }

      /// Utility structure for the CLKernel's index operator
      struct Arg {
        CLKernel &k; //!< parent CLKernel
        int idx;     //!< Argument index

        /// constructor
        Arg(CLKernel &k, int idx):k(k),idx(idx) {}

        /// template-based assigment (calls the parent kernel's setArg method)
        template<class T>
        inline void operator=(const T &t) {
          k.setArg(idx,t);
        }
      };

      /// for index operator-based setting of kernel arguments
      inline Arg operator[](int idx) { return Arg(*this,idx); }

    };
  }
}

#endif
