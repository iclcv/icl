// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include<ICLUtils/Exception.h>

namespace icl{
  namespace math{
    /// Special exception implementation for the FFT package
    class FFTException : public utils::ICLException{
      public:
      FFTException(const std::string &msg):utils::ICLException(msg){}
    };
  }// namespace math
}
