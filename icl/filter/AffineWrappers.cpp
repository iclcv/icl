// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Hosts REGISTER_CONFIGURABLE for the three header-only AffineOp subclasses
// so they show up in icl-configurable-info / filter-playground. Each hides
// the AffineOp knobs it doesn't use via deactivateProperty() in its ctor.

#include <icl/filter/RotateOp.h>
#include <icl/filter/ScaleOp.h>
#include <icl/filter/TranslateOp.h>

using namespace icl::utils;

namespace icl::filter {
  REGISTER_CONFIGURABLE_DEFAULT(RotateOp);
  REGISTER_CONFIGURABLE_DEFAULT(ScaleOp);
  REGISTER_CONFIGURABLE_DEFAULT(TranslateOp);
}
