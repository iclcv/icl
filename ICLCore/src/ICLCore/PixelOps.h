// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>

namespace icl{
  namespace core{

    /// copies data from source to destination array (memcpy) \ingroup GENERAL
    /** Explicitly instantiated for all 5 ICL depth types.
        For performance-critical inner loops, consider using memcpy directly. */
    template<class T>
    ICLCore_API void copy(const T *src, const T *srcEnd, T *dst);

    /// converts data from source to destination array with type casting \ingroup GENERAL
    /** Explicitly instantiated for all 25 source/destination type pairs.
        IPP-optimized or SSE2-optimized specializations are used where available. */
    template<class srcT, class dstT>
    ICLCore_API void convert(const srcT *poSrcStart, const srcT *poSrcEnd, dstT *poDst);

  } // namespace core
}
