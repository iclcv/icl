/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FFTDispatching.cpp                 **
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

#include <ICLMath/FFTDispatching.h>
#include <ICLMath/FFTUtils.h>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

      // C++ backend: delegates to the existing thread-safe fft2D_cpp / ifft2D_cpp
      DynMatrix<icl32c>& cpp_fft_fwd32f(const DynMatrix<icl32f>& src,
                                          DynMatrix<icl32c>& dst,
                                          DynMatrix<icl32c>& buf) {
        return fft::fft2D_cpp(src, dst, buf);
      }

      DynMatrix<icl32c>& cpp_fft_inv32f(const DynMatrix<icl32c>& src,
                                          DynMatrix<icl32c>& dst,
                                          DynMatrix<icl32c>& buf) {
        return fft::ifft2D_cpp(src, dst, buf);
      }

      DynMatrix<icl32c>& cpp_fft_fwd32fc(const DynMatrix<icl32c>& src,
                                           DynMatrix<icl32c>& dst,
                                           DynMatrix<icl32c>& buf) {
        return fft::fft2D_cpp(src, dst, buf);
      }

    } // anonymous namespace

    FFTDispatching& FFTDispatching::instance() {
      static FFTDispatching d;
      return d;
    }

    // Static-init registration of C++ fallback backends.
    // IPP/MKL/OpenCL backends self-register from their own .cpp files
    // using utils::registerBackend<FFTContext, Sig>(...).
    static const int _r1 = []() {
      auto& d = FFTDispatching::instance();
      d.initDispatching("FFT");
      auto& fwd = d.addSelector<FFTFwd32fSig>("fwd32f");
      fwd.add(Backend::Cpp, cpp_fft_fwd32f);
      auto& inv = d.addSelector<FFTInv32fSig>("inv32f");
      inv.add(Backend::Cpp, cpp_fft_inv32f);
      auto& fwdc = d.addSelector<FFTFwd32fcSig>("fwd32fc");
      fwdc.add(Backend::Cpp, cpp_fft_fwd32fc);
      return 0;
    }();

  } // namespace math
} // namespace icl
