/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLAlgorithms/src/ViewBasedTemplateMatcher.cpp         **
** Module : ICLAlgorithms                                          **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLAlgorithms/ViewBasedTemplateMatcher.h>

namespace icl{

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
    
    m_vecResults =  iclMatchTemplate(image,
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
  
  
}
