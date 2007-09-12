#ifndef AFFINE_OP_H
#define AFFINE_OP_H

#include "iclBaseAffineOp.h"

namespace icl{
  /// Class to apply an arbitrary series of affine transformations \ingroup AFFINE \ingroup UNARY
  /**
    every affine operation modifies an internal matrix,
    with the apply function, the matrix will be multiplicated to the image, 
    so that the desired affine operations are executed on the image.
  */
  class AffineOp : public BaseAffineOp {
    public:
    /// Constructor
    AffineOp (scalemode eInterpolate=interpolateLIN);
    /// resets the internal Matrix
    /**
      to
      1 0 0
      0 1 0
      0 0 
    */
    void reset  ();
    /// adds a rotation
    /**
      @param dAngle angle in degrees (clockwise) 
    */
    void rotate (double dAngle);
    
    ///adds a traslation
    /**
      @param x pixels to translate in x-direction
      @param y pixels to translate in y-direction
    */
    void translate (double x, double y) {
      m_aadT[0][2] += x; m_aadT[1][2] += y;
    }
    /// adds a scale
    /**
      @param x scale-factor in x-direction
      @param y scale-factor in y-direction
      different values for x and y will lead to a dilation / upsetting deformation
    */
    void scale (double x, double y) {
      m_aadT[0][0] *= x; m_aadT[1][0] *= x;
      m_aadT[0][1] *= y; m_aadT[1][1] *= y;
    }
    
    /// Applies the affine transform to the image
    void apply (const ImgBase *poSrc, ImgBase **ppoDst);
    
    private:
    /// array of class methods used to transform depth8u and depth32f images
    void (AffineOp::*m_aMethods[depthLast+1])(const ImgBase *poSrc, ImgBase *poDst); 
    
    template<typename T>
    void affine (const ImgBase *poSrc, ImgBase *poDst);
    
    void applyT (const double p[2], double aResult[2]);
    static void useMinMax (const double aCur[2], 
                           double aMin[2], double aMax[2]);
    void getShiftAndSize (const Rect& roi, Size& size, 
                          double& xShift, double& yShift);
    private:
    double    m_aadT[2][3];
    // double    m_dxShift, m_dyShift;
    scalemode m_eInterpolate;
  };
}


#endif
