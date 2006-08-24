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
      if (bOnlyROI) {
         oSrcOffset = poSrc->getROIOffset();
         oDstOffset = Point::zero;
         oSize = poSrc->getROISize();
      } else {
         oDstOffset = oSrcOffset = Point::zero;
         oSize = poSrc->getSize();
      }

      ensureCompatible (ppoDst, poSrc->getDepth(), oSize, 
                        poSrc->getFormat(), poSrc->getChannels(),
                        Rect (bOnlyROI ? Point::zero : poSrc->getROIOffset(), oSize));

      (this->*(aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
   }

// }}}

// {{{ Affine

#ifdef WITH_IPP_OPTIMIZATION 
   template<>
   void Affine::affine<icl8u> (ImgI *poSrc, ImgI *poDst) {
      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiWarpAffine_8u_C1R (poSrc->asImg<icl8u>()->getROIData (c),
                                poSrc->getSize(), poSrc->getLineStep(), 
//                                Rect (Point::zero, poSrc->getSize()),
                                poSrc->getROI(),
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
      double aRect[4][2] = {roi.x, roi.y,
                            roi.x + roi.width, roi.y, 
                            roi.x + roi.width, roi.y + roi.height,
                            roi.x, roi.y + roi.height};

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

      double quad[4][2];
      ippiGetAffineQuad(poSrc->getROI(), quad, T);
      printf ("\n\n%f %f     %f %f\n", quad[0][0],quad[0][1], quad[1][0],quad[1][1]);
      printf ("%f %f     %f %f\n", quad[2][0],quad[2][1], quad[3][0],quad[3][1]);
      ippiGetAffineBound(poSrc->getROI(), quad, T);
      printf ("bounds:\n %f %f  -  %f %f\n", quad[0][0],quad[0][1],quad[1][0],quad[1][1]);
      printf ("%d %d  -  %d %d\n", (*ppoDst)->getROIOffset().x, (*ppoDst)->getROIOffset().y,
              (*ppoDst)->getROISize().width, (*ppoDst)->getROISize().height);

      getShiftAndSize (poSrc->getROI(), oSize, xShift, yShift);
      translate (-xShift, -yShift);
      printf ("shift: %f %f\n", xShift, yShift);
      ensureCompatible (ppoDst, poSrc->getDepth(), oSize, 
                        poSrc->getFormat(), poSrc->getChannels());

      ippiGetAffineQuad(poSrc->getROI(), quad, T);
      printf ("\n%f %f     %f %f\n", quad[0][0],quad[0][1], quad[1][0],quad[1][1]);
      printf ("%f %f     %f %f\n", quad[2][0],quad[2][1], quad[3][0],quad[3][1]);
      ippiGetAffineBound(poSrc->getROI(), quad, T);
      printf ("bounds:\n%f %f  -  %f %f\n", quad[0][0],quad[0][1],quad[1][0],quad[1][1]);
      printf ("%d %d  -  %d %d\n", (*ppoDst)->getROIOffset().x, (*ppoDst)->getROIOffset().y,
              (*ppoDst)->getROISize().width, (*ppoDst)->getROISize().height);

      (this->*(aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
      translate (xShift, yShift);
   }

// }}}
#if 0
// {{{ Rotate

#ifdef WITH_IPP_OPTIMIZATION 
   template<>
   void Rotate::rotate<icl8u> (ImgI *poSrc, ImgI *poDst) {
      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiRotate_8u_C1R (poSrc->asImg<icl8u>()->getROIData (c),
                            poSrc->getSize(), poSrc->getLineStep(), poSrc->getROI(),
                            poDst->asImg<icl8u>()->getData (c), 
                            poDst->getLineStep(), poDst->getROI(), dAngle, xShift, yShift, 
                            eInterpolate);
      }
   }
   template<>
   void Rotate::rotate<icl32f> (ImgI *poSrc, ImgI *poDst) {
      for(int c=0; c < poSrc->getChannels(); c++) {
         ippiRotate_32f_C1R (poSrc->asImg<icl32f>()->getROIData (c),
                             poSrc->getSize(), poSrc->getLineStep(), poSrc->getROI(),
                             poDst->asImg<icl32f>()->getData (c), 
                             poDst->getLineStep(), poDst->getROI(), dAngle, xShift, yShift, 
                             eInterpolate);
      }
   }
#else
#warning "fallback for Rotate::rotate not yet implemented"
   template<typename T>
   void Rotate::rotate (ImgI *poSrc, ImgI *poDst) {
      ERROR_LOG ("not yet implemented");
   }
#endif

   Rotate::Rotate (double dAngle, scalemode eInterpolate) :
      dAngle (dAngle), eInterpolate (eInterpolate)
   {
      this->aMethods[depth8u] = &Rotate::rotate<icl8u>;
      this->aMethods[depth32f] = &Rotate::rotate<icl32f>;
   }

   void Rotate::apply (ImgI *poSrc, ImgI **ppoDst) {
      double bounds[2][2];
#ifdef WITH_IPP_OPTIMIZATION
      // center of rotation is center of ROI
      xShift = 0.5 * (double) poSrc->getROISize().width;
      xShift += poSrc->getROIOffset().x;
      yShift = 0.5 * (double) poSrc->getROISize().height;
      yShift += poSrc->getROIOffset().y;

      // compute necessary shift
      ippiGetRotateShift (xShift, yShift, dAngle, &xShift, &yShift);
      // compute bounding box for destination image
      ippiGetRotateBound (poSrc->getROI(), bounds, dAngle, xShift, yShift);
#else
#warning "fallback for Rotate:apply not yet implemented"
#endif
      Size oSize ((int) ceil(bounds[1][0] - bounds[0][0]), 
                  (int) ceil(bounds[1][1] - bounds[0][1]));
      ensureCompatible (ppoDst, poSrc->getDepth(), oSize, 
                        poSrc->getFormat(), poSrc->getChannels());

      (this->*(aMethods[poSrc->getDepth()]))(poSrc, *ppoDst);
   }

// }}}
#endif
} // namespace icl
