#include "iclAffineOp.h"
#include <iclImg.h>
#include <cstring>
using std::memcpy;

namespace icl{

  
#ifdef WITH_IPP_OPTIMIZATION 
   template<>
   void AffineOp::affine<icl8u> (const ImgBase *poSrc, ImgBase *poDst) {
     // {{{ open

      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiWarpAffine_8u_C1R (poSrc->asImg<icl8u>()->getData (c),
                                poSrc->getSize(), poSrc->getLineStep(), 
                                Rect (Point::null, poSrc->getSize()),
                                poDst->asImg<icl8u>()->getData (c), 
                                poDst->getLineStep(), poDst->getROI(), 
                                m_aadT, m_eInterpolate);
      }
   }
   // }}}
  
   template<>
   void AffineOp::affine<icl32f> (const ImgBase *poSrc, ImgBase *poDst) {
     // {{{ opem

      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiWarpAffine_32f_C1R (poSrc->asImg<icl32f>()->getData (c),
                                 poSrc->getSize(), poSrc->getLineStep(), 
                                 Rect (Point::null, poSrc->getSize()),
                                 poDst->asImg<icl32f>()->getData (c), 
                                 poDst->getLineStep(), poDst->getROI(), 
                                 m_aadT, m_eInterpolate);
      }
   }

   // }}}


   AffineOp::AffineOp (scalemode eInterpolate) : m_eInterpolate (eInterpolate)  {
     // {{{ open
     reset ();
      this->m_aMethods[depth8u] = &AffineOp::affine<icl8u>;
      this->m_aMethods[depth32f] = &AffineOp::affine<icl32f>;
   }

  // }}}
#endif

   void AffineOp::reset () {
     // {{{ open

      m_aadT[0][0] = m_aadT[1][1] = 1.0;
      m_aadT[0][1] = m_aadT[1][0] = m_aadT[0][2] = m_aadT[1][2] = 0.0;
   }

  // }}}
  
   void AffineOp::rotate (double dAngle) {
     // {{{ open

      double c=cos(dAngle * M_PI / 180.);
      double s=sin(dAngle * M_PI / 180.);
      double O[2][3]; memcpy (O, m_aadT, 6*sizeof(double));

      m_aadT[0][0] = O[0][0]*c + O[0][1]*s;
      m_aadT[0][1] = O[0][1]*c - O[0][0]*s;
      m_aadT[1][0] = O[1][0]*c + O[1][1]*s;
      m_aadT[1][1] = O[1][1]*c - O[1][0]*s;
   }

  // }}}

  

   inline void AffineOp::applyT (const double p[2], double adResult[2]) {
     // {{{ open

      adResult[0] = m_aadT[0][0]*p[0] + m_aadT[0][1]*p[1] + m_aadT[0][2];
      adResult[1] = m_aadT[1][0]*p[0] + m_aadT[1][1]*p[1] + m_aadT[1][2];
   }

  // }}}
  
   inline void AffineOp::useMinMax (const double adCur[2], 
                                  double adMin[2], double adMax[2]) {
     // {{{ open

      adMin[0] = std::min (adCur[0], adMin[0]);
      adMin[1] = std::min (adCur[1], adMin[1]);
      adMax[0] = std::max (adCur[0], adMax[0]);
      adMax[1] = std::max (adCur[1], adMax[1]);
   }

  // }}}

   void AffineOp::getShiftAndSize (const Rect& roi, Size& size, 
                                 double& xShift, double& yShift) {
     // {{{ open

      double adMin[2], adMax[2], adCur[2];
      // compute corners of the ROI rectangle
      double aadRect[4][2] = {{roi.x, roi.y},
                              {roi.x + roi.width, roi.y},
                              {roi.x + roi.width, roi.y + roi.height},
                              {roi.x, roi.y + roi.height}};

      // apply transform to each corner off the ROI rectangle
      // shift is smallest x and y coordinate of this transform
      // size is difference between largest and smalles values
      
      applyT (aadRect[0], adMax); adMin[0] = adMax[0]; adMin[1] = adMax[1];
      applyT (aadRect[1], adCur); useMinMax (adCur, adMin, adMax);
      applyT (aadRect[2], adCur); useMinMax (adCur, adMin, adMax);
      applyT (aadRect[3], adCur); useMinMax (adCur, adMin, adMax);

      size.width  = (int) ceil(adMax[0] - adMin[0]); xShift = adMin[0];
      size.height = (int) ceil(adMax[1] - adMin[1]); yShift = adMin[1];
   }

  // }}}

   void AffineOp::apply (const ImgBase *poSrc, ImgBase **ppoDst) {
     // {{{ open

     ICLASSERT_RETURN(poSrc);
     ICLASSERT_RETURN(ppoDst);
     ICLASSERT_RETURN(poSrc != *ppoDst);
     ICLASSERT_RETURN( poSrc->getDepth() == depth32f || poSrc->getDepth() == depth8u);
      double xShift, yShift;
      Size   oSize;
      getShiftAndSize (poSrc->getROI(), oSize, xShift, yShift);
      translate (-xShift, -yShift);
      ensureCompatible (ppoDst, poSrc->getDepth(), oSize, 
                        poSrc->getChannels(), poSrc->getFormat());

      (this->*(m_aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
      translate (xShift, yShift);
   }

  // }}}


}
