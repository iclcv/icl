#include "GeoTransforms.h"
#include "Img.h"

namespace icl {

// {{{ Mirror

#ifdef WITH_IPP_OPTIMIZATION 
   template<>
   void Mirror::mirror<icl8u> (ImgI *poSrc, ImgI *poDst) {
      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiMirror_8u_C1R (poSrc->asImg<icl8u>()->getROIData (c, oSrcOffset), 
                            poSrc->getLineStep(),
                            poDst->asImg<icl8u>()->getROIData (c, oDstOffset), 
                            poDst->getLineStep(), oSize, (IppiAxis) eAxis);
      }
   }
   template<>
   void Mirror::mirror<icl32f> (ImgI *poSrc, ImgI *poDst) {
      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiMirror_32s_C1R ((Ipp32s*) poSrc->asImg<icl32f>()->getROIData (c, oSrcOffset), 
                             poSrc->getLineStep(),
                             (Ipp32s*) poDst->asImg<icl32f>()->getROIData (c, oDstOffset), 
                             poDst->getLineStep(), oSize, (IppiAxis) eAxis);
      }
   }
#else
#warning "fallback for Mirror::mirror not yet implemented"
   template<typename T>
   void Mirror::mirror (ImgI *poSrc, ImgI *poDst) {
      ERROR_LOG ("not yet implemented");
   }
#endif

   Mirror::Mirror (axis eAxis, bool bOnlyROI) :
      eAxis (eAxis), bOnlyROI (bOnlyROI) 
   {
      this->aMethods[depth8u] = &Mirror::mirror<icl8u>;
      this->aMethods[depth32f] = &Mirror::mirror<icl32f>;
   }

   void Mirror::apply (ImgI *poSrc, ImgI **ppoDst) {
      Point oROIOffset;
      if (bOnlyROI) {
         oSrcOffset = poSrc->getROIOffset();
         oROIOffset = oDstOffset = Point::zero;
         oSize = poSrc->getROISize();
      } else {
         oDstOffset = oSrcOffset = Point::zero;
         oSize = poSrc->getSize();

         oROIOffset = poSrc->getROIOffset();
         if (eAxis == axisHorizontal || eAxis == axisBoth) 
            oROIOffset.y = oSize.height - oROIOffset.y - poSrc->getROISize().height;
         if (eAxis == axisVertical || eAxis == axisBoth) 
            oROIOffset.x = oSize.width - oROIOffset.x - poSrc->getROISize().width;
      }

      ensureCompatible (ppoDst, poSrc->getDepth(), oSize, 
                        poSrc->getFormat(), poSrc->getChannels(),
                        Rect (oROIOffset, poSrc->getROISize()));
      
      (this->*(aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
   }

// }}}

// {{{ Affine

#ifdef WITH_IPP_OPTIMIZATION 
   template<>
   void Affine::affine<icl8u> (ImgI *poSrc, ImgI *poDst) {
      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiWarpAffine_8u_C1R (poSrc->asImg<icl8u>()->getData (c),
                                poSrc->getSize(), poSrc->getLineStep(), 
//                                poSrc->getROI(),
                                Rect (Point::zero, poSrc->getSize()),
                                poDst->asImg<icl8u>()->getData (c), 
                                poDst->getLineStep(), poDst->getROI(), 
                                T, eInterpolate);
      }
   }
   template<>
   void Affine::affine<icl32f> (ImgI *poSrc, ImgI *poDst) {
      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiWarpAffine_32f_C1R (poSrc->asImg<icl32f>()->getData (c),
                                 poSrc->getSize(), poSrc->getLineStep(), 
//                                 poSrc->getROI(),
                                 Rect (Point::zero, poSrc->getSize()),
                                 poDst->asImg<icl32f>()->getData (c), 
                                 poDst->getLineStep(), poDst->getROI(), 
                                 T, eInterpolate);
      }
   }
#else
#warning "fallback for Affine::affine not yet implemented"
   template<typename T>
   void Affine::affine (ImgI *poSrc, ImgI *poDst) {
      ERROR_LOG ("not yet implemented");
   }
#endif

   Affine::Affine (scalemode eInterpolate) : eInterpolate (eInterpolate)
   {
      reset ();
      this->aMethods[depth8u] = &Affine::affine<icl8u>;
      this->aMethods[depth32f] = &Affine::affine<icl32f>;
   }

   void Affine::reset () {
      T[0][0] = T[1][1] = 1.0;
      T[0][1] = T[1][0] = T[0][2] = T[1][2] = 0.0;
   }
   void Affine::rotate (double dAngle) {
      double c=cos(dAngle * M_PI / 180.);
      double s=sin(dAngle * M_PI / 180.);
      double O[2][3]; memcpy (O, T, 6*sizeof(double));

      T[0][0] = O[0][0]*c + O[0][1]*s;
      T[0][1] = O[0][1]*c - O[0][0]*s;
      T[1][0] = O[1][0]*c + O[1][1]*s;
      T[1][1] = O[1][1]*c - O[1][0]*s;
   }

   // {{{ compute geometry of transformed ROI

   inline void Affine::applyT (const double p[2], double aResult[2]) {
      aResult[0] = T[0][0]*p[0] + T[0][1]*p[1] + T[0][2];
      aResult[1] = T[1][0]*p[0] + T[1][1]*p[1] + T[1][2];
   }
   inline void Affine::useMinMax (const double aCur[2], 
                                  double aMin[2], double aMax[2]) {
      aMin[0] = std::min (aCur[0], aMin[0]);
      aMin[1] = std::min (aCur[1], aMin[1]);
      aMax[0] = std::max (aCur[0], aMax[0]);
      aMax[1] = std::max (aCur[1], aMax[1]);
   }

   void Affine::getShiftAndSize (const Rect& roi, Size& size, 
                                 double& xShift, double& yShift) {
      double aMin[2], aMax[2], aCur[2];
      // compute corners of the ROI rectangle
      double aRect[4][2] = {{roi.x, roi.y},
                            {roi.x + roi.width, roi.y},
                            {roi.x + roi.width, roi.y + roi.height},
                            {roi.x, roi.y + roi.height}};

      // apply transform to each corner off the ROI rectangle
      // shift is smallest x and y coordinate of this transform
      // size is difference between largest and smalles values
      
      applyT (aRect[0], aMax); aMin[0] = aMax[0]; aMin[1] = aMax[1];
      applyT (aRect[1], aCur); useMinMax (aCur, aMin, aMax);
      applyT (aRect[2], aCur); useMinMax (aCur, aMin, aMax);
      applyT (aRect[3], aCur); useMinMax (aCur, aMin, aMax);

      size.width  = (int) ceil(aMax[0] - aMin[0]); xShift = aMin[0];
      size.height = (int) ceil(aMax[1] - aMin[1]); yShift = aMin[1];
   }

   // }}}
   
   void Affine::apply (ImgI *poSrc, ImgI **ppoDst) {
      double xShift, yShift;
      Size   oSize;

      getShiftAndSize (poSrc->getROI(), oSize, xShift, yShift);
      translate (-xShift, -yShift);
      ensureCompatible (ppoDst, poSrc->getDepth(), oSize, 
                        poSrc->getFormat(), poSrc->getChannels());

      (this->*(aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
      translate (xShift, yShift);
   }

// }}}

} // namespace icl
