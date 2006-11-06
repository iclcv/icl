#ifndef ICL_GEOTRANSFORMS_H
#define ICL_GEOTRANSFORMS_H

#include <Filter.h>

namespace icl {

/* {{{ Mirror */

   /// Class to mirror images vertically or horizontally
   class Mirror : public Filter {
   public:
      /// Constructor
      Mirror (axis eAxis);

      /// Applies the mirror transform to the images
      void apply (const ImgBase *poSrc, ImgBase **ppoDst);

   protected:
      /// array of class methods used to transform depth8u and depth32f images
      void (Mirror::*aMethods[2])(const ImgBase *poSrc, ImgBase *poDst); 

      template<typename T>
      void mirror (const ImgBase *poSrc, ImgBase *poDst);

   private:
      axis  eAxis;
      Size  oSize;
      Point oSrcOffset, oDstOffset;
   };

/* }}} */

/* {{{ Affine */

   /// Class to apply an arbitrary series of affine transformations
   class Affine : public Filter {
   public:
      /// Constructor
      Affine (scalemode eInterpolate=interpolateLIN);

      void reset  ();
      void rotate (double dAngle);
      void translate (double x, double y) {
         T[0][2] += x; T[1][2] += y;
      }
      void scale (double x, double y) {
         T[0][0] *= x; T[1][0] *= x;
         T[0][1] *= y; T[1][1] *= y;
      }

      /// Applies the affine transform to the image
      void apply (const ImgBase *poSrc, ImgBase **ppoDst);

   protected:
      /// array of class methods used to transform depth8u and depth32f images
      void (Affine::*aMethods[2])(const ImgBase *poSrc, ImgBase *poDst); 

      template<typename T>
      void affine (const ImgBase *poSrc, ImgBase *poDst);

   private:
      void applyT (const double p[2], double aResult[2]);
      static void useMinMax (const double aCur[2], 
                             double aMin[2], double aMax[2]);
      void getShiftAndSize (const Rect& roi, Size& size, 
                            double& xShift, double& yShift);
   private:
      double    T[2][3];
      double    xShift, yShift;
      scalemode eInterpolate;
   };

/* }}} */

/* {{{ Rotate */

   /// Class to rotate images
   class Rotate : private Affine {
   public:
      /// Constructor
      Rotate (double dAngle, scalemode eInterpolate=interpolateLIN) :
         Affine (eInterpolate) {}

      /// change rotation angle
      void setAngle (double dAngle) {
         Affine::reset (); Affine::rotate (dAngle);
      }

      Affine::apply;
   };

/* }}} */

} // namespace icl

#endif // ICL_GEOTRANSFORMS_H
