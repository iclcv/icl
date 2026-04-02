// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Macros.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLFilter/OpROIHandler.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
   /// check+adapt destination images parameters against given values
  bool OpROIHandler::prepare (ImgBase **ppoDst, depth eDepth,
                              const Size &imgSize, format eFormat, int nChannels,
                              const Rect& roi, Time timestamp) {
    ICLASSERT_RETURN_VAL (ppoDst, false);
    if (m_bCheckOnly) {
      ImgBase* dst = *ppoDst;
      ICLASSERT_RETURN_VAL( dst , false);
      ICLASSERT_RETURN_VAL( dst->getDepth() == eDepth , false);
      ICLASSERT_RETURN_VAL( dst->getChannels () == nChannels ,false);
      ICLASSERT_RETURN_VAL( dst->getFormat () == eFormat ,false);
      if(dst->getROISize() != roi.getSize()){
        ERROR_LOG("ROI size missmatch: given: "<< roi.getSize()
                  << "  destination: "<< dst->getROISize());
        return false;
      }

      dst->setTime(timestamp);
    } else {
      ensureCompatible (ppoDst, eDepth, imgSize,
                        nChannels, eFormat, roi);
      (*ppoDst)->setTime(timestamp);
    }
    return true;
  }
  } // namespace icl::filter