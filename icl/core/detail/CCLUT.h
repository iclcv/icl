// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Img.h>

namespace icl::core {
  class ICLCore_API CCLUT{
    public:
    CCLUT(format srcFmt, format dstFmt);
    void cc(const ImgBase *src, ImgBase *dst, bool roiOnly=false);

    private:
    Img8u m_oLUT;
  };
} // namespace icl::core