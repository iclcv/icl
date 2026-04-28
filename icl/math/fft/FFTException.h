// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>

namespace icl::math {
  /// Special exception implementation for the FFT package
  class FFTException : public utils::ICLException{
    public:
    FFTException(const std::string &msg):utils::ICLException(msg){}
  };
  } // namespace icl::math