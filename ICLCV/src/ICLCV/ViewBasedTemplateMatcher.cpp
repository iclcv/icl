// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCV/ViewBasedTemplateMatcher.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::cv {
  ViewBasedTemplateMatcher::ViewBasedTemplateMatcher(float significance, mode m, bool clipBuffersToROI):
    m_fSignificance(significance),m_eMode(m),m_bClipBuffersToROI(clipBuffersToROI){}

  void ViewBasedTemplateMatcher::setSignificance(float significance){
    m_fSignificance = significance;
  }
  void ViewBasedTemplateMatcher::setMode(mode m){
    m_eMode = m;

  }

  void ViewBasedTemplateMatcher::setClipBuffersToROI(bool flag){
    m_bClipBuffersToROI = flag;
  }

  const std::vector<Rect> &ViewBasedTemplateMatcher::match(const Img8u &image,
                                                           const Img8u &templ,
                                                           const Img8u &imageMask,
                                                           const Img8u &templMask){

    m_vecResults =  matchTemplate(image,
                                  imageMask.isNull() ? 0 : &imageMask,
                                  templ,
                                  templMask.isNull() ? 0 : &templMask,
                                  m_fSignificance,
                                  m_aoBuffers,
                                  m_aoBuffers+1,
                                  m_aoBuffers+2,
                                  m_bClipBuffersToROI,
                                  &m_oRD,
                                  m_eMode == sqrtDistance ? false : true);

    return m_vecResults;
  }


  } // namespace icl::cv