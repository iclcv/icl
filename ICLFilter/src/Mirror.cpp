#include "Mirror.h"
#include <Img.h>

namespace icl{
   template<typename T>
   void Mirror::mirror (const ImgBase *poSrc, ImgBase *poDst) {
      const Img<T> *poS = (const Img<T>*) poSrc;
      Img<T> *poD = (Img<T>*) poDst;
      for(int c=0; c < poSrc->getChannels(); c++) {
         flippedCopyChannelROI (m_eAxis, poS, c, m_oSrcOffset, m_oSize,
                                poD, c, m_oDstOffset, m_oSize);
      }
   }

   Mirror::Mirror (axis eAxis) :
      m_eAxis (eAxis)
   {
      this->m_aMethods[depth8u] = &Mirror::mirror<icl8u>;
      this->m_aMethods[depth16s] = &Mirror::mirror<icl16s>;
      this->m_aMethods[depth32s] = &Mirror::mirror<icl32s>;
      this->m_aMethods[depth32f] = &Mirror::mirror<icl32f>;
      this->m_aMethods[depth64f] = &Mirror::mirror<icl64f>;
   }

   void Mirror::apply (const ImgBase *poSrc, ImgBase **ppoDst) {
      Point oROIOffset;
      if (bClipToROI) {
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

      if (Filter::prepare (ppoDst, poSrc->getDepth(), m_oSize, 
                           poSrc->getFormat(), poSrc->getChannels(),
                           Rect (oROIOffset, poSrc->getROISize()), poSrc->getTime()))
         (this->*(m_aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
   }
}
