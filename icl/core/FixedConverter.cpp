// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/FixedConverter.h>
#include <icl/core/CoreFunctions.h>


namespace icl::core {
  FixedConverter::FixedConverter(const ImgParams &p, depth d, bool applyToROIOnly):
    m_oParams(p),m_oConverter(applyToROIOnly),m_eDepth(d) { }

  void FixedConverter::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( ppoDst );
    ensureCompatible(ppoDst,m_eDepth,m_oParams);
    m_oConverter.apply(poSrc,*ppoDst);
  }
  } // namespace icl::core