// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Time.h>
#include <ICLCore/Types.h>
#include <ICLCore/ImgParams.h>
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