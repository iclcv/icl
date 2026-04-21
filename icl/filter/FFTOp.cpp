// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/FFTOp.h>

using namespace icl::utils;

// Register FFTOp with the Configurable factory so icl-configurable-info and the
// filter playground can discover it. Properties (result mode, size adaption,
// fft shift, force DFT) are added in BaseFFTOp's constructor.
namespace icl::filter {
  REGISTER_CONFIGURABLE_DEFAULT(FFTOp);
}
