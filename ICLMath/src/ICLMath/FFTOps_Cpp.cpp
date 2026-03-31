/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FFTOps_Cpp.cpp                     **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

// C++ fallback backends for FFT operations.
// Delegates to existing fft2D_cpp / ifft2D_cpp via non-owning DynMatrix wrappers.

#include <ICLMath/FFTOps.h>
#include <ICLMath/FFTUtils.h>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

      template<class T>
      void cpp_fft_r2c(const T* src, int rows, int cols, std::complex<T>* dst) {
        DynMatrix<T> srcMat(cols, rows, const_cast<T*>(src), false);
        DynMatrix<std::complex<T>> dstMat(cols, rows, dst, false);
        DynMatrix<std::complex<T>> buf;
        fft::fft2D_cpp(srcMat, dstMat, buf);
      }

      template<class T>
      void cpp_fft_c2c(const std::complex<T>* src, int rows, int cols, std::complex<T>* dst) {
        DynMatrix<std::complex<T>> srcMat(cols, rows, const_cast<std::complex<T>*>(src), false);
        DynMatrix<std::complex<T>> dstMat(cols, rows, dst, false);
        DynMatrix<std::complex<T>> buf;
        fft::fft2D_cpp(srcMat, dstMat, buf);
      }

      template<class T>
      void cpp_ifft_c2c(const std::complex<T>* src, int rows, int cols, std::complex<T>* dst) {
        DynMatrix<std::complex<T>> srcMat(cols, rows, const_cast<std::complex<T>*>(src), false);
        DynMatrix<std::complex<T>> dstMat(cols, rows, dst, false);
        DynMatrix<std::complex<T>> buf;
        fft::ifft2D_cpp(srcMat, dstMat, buf);
      }

    } // anonymous namespace

    static const int _cpp_fft_reg = []() {
      auto cpp_f = FFTOps<float>::instance().backends(Backend::Cpp);
      cpp_f.add<FFTOps<float>::R2CSig>(FFTOp::r2c, cpp_fft_r2c<float>, "C++ row-column FFT");
      cpp_f.add<FFTOps<float>::C2CSig>(FFTOp::c2c, cpp_fft_c2c<float>, "C++ row-column FFT");
      cpp_f.add<FFTOps<float>::InvC2CSig>(FFTOp::inv_c2c, cpp_ifft_c2c<float>, "C++ row-column IFFT");

      auto cpp_d = FFTOps<double>::instance().backends(Backend::Cpp);
      cpp_d.add<FFTOps<double>::R2CSig>(FFTOp::r2c, cpp_fft_r2c<double>, "C++ row-column FFT");
      cpp_d.add<FFTOps<double>::C2CSig>(FFTOp::c2c, cpp_fft_c2c<double>, "C++ row-column FFT");
      cpp_d.add<FFTOps<double>::InvC2CSig>(FFTOp::inv_c2c, cpp_ifft_c2c<double>, "C++ row-column IFFT");

      return 0;
    }();

  } // namespace math
} // namespace icl
