/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ViewBasedTemplateMatcher.cpp           **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCV/ViewBasedTemplateMatcher.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{
  
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
    
    
  } // namespace cv
}
