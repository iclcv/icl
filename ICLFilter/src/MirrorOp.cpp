/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/MirrorOp.cpp                             **
** Module : ICLFilter                                              **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLFilter/MirrorOp.h>
#include <ICLCore/Img.h>

namespace icl{
   template<typename T>
   void MirrorOp::mirror (const ImgBase *poSrc, ImgBase *poDst) {
      const Img<T> *poS = (const Img<T>*) poSrc;
      Img<T> *poD = (Img<T>*) poDst;
      for(int c=0; c < poSrc->getChannels(); c++) {
         flippedCopyChannelROI (m_eAxis, poS, c, m_oSrcOffset, m_oSize,
                                poD, c, m_oDstOffset, m_oSize);
      }
   }

   MirrorOp::MirrorOp (axis eAxis) :
      m_eAxis (eAxis)
   {
      this->m_aMethods[depth8u] = &MirrorOp::mirror<icl8u>;
      this->m_aMethods[depth16s] = &MirrorOp::mirror<icl16s>;
      this->m_aMethods[depth32s] = &MirrorOp::mirror<icl32s>;
      this->m_aMethods[depth32f] = &MirrorOp::mirror<icl32f>;
      this->m_aMethods[depth64f] = &MirrorOp::mirror<icl64f>;
   }

   void MirrorOp::apply (const ImgBase *poSrc, ImgBase **ppoDst) {
      Point oROIOffset;
      if (getClipToROI()) {
         m_oSrcOffset = poSrc->getROIOffset();
         oROIOffset = m_oDstOffset = Point::null;
         m_oSize = poSrc->getROISize();
      } else {
         m_oDstOffset = m_oSrcOffset = Point::null;
         m_oSize = poSrc->getSize();

         oROIOffset = poSrc->getROIOffset();
         if (m_eAxis == axisHorz || m_eAxis == axisBoth) 
            oROIOffset.y = m_oSize.height - oROIOffset.y - poSrc->getROISize().height;
         if (m_eAxis == axisVert || m_eAxis == axisBoth) 
            oROIOffset.x = m_oSize.width - oROIOffset.x - poSrc->getROISize().width;
      }

      if (UnaryOp::prepare (ppoDst, poSrc->getDepth(), m_oSize, 
                           poSrc->getFormat(), poSrc->getChannels(),
                           Rect (oROIOffset, poSrc->getROISize()), poSrc->getTime()))
         (this->*(m_aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
   }
}
