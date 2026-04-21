// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Range.h>
#include <icl/utils/Time.h>
#include <icl/core/Types.h>
#include <icl/core/ImgParams.h>
#include <vector>

namespace icl::qt {
  struct ImageStatistics{
    core::ImgParams params;
    core::depth d;
    std::vector<utils::Range64f> ranges;
    utils::Range64f globalRange;
    std::vector<std::vector<int> > histos;
    bool isNull;
    utils::Time time;
  };
  } // namespace icl::qt