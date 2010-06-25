/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/AffineOp.h                           **
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

#ifndef AFFINE_OP_H
#define AFFINE_OP_H

#include <ICLFilter/BaseAffineOp.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  /// Class to apply an arbitrary series of affine transformations \ingroup AFFINE \ingroup UNARY
  /** Affine operations apply transform pixel locations using affine matrix transformation.
      To optimize performance concatenated affine transformation's matrices are pre-multiplied.
      
      \section BENCH Benchmarks
      
      example: a 300x400 rgb 8u-image was scaled by 1.001 and rotated by 1-360 degree in 3.6 degree steps
               We used a 2Ghz Core2Duo machine and g++ 4.3 with -O4 and -march=native flags set

      With IPP:
      * neares neighbour interpolation: 1ms
      * linear interpolation 5ms
      
      C++-Fallback:
      * neares neighbour interpolation: 22ms
      * linear interpolation 52ms

  */
  class AffineOp : public BaseAffineOp, public Uncopyable {
    public:
    /// Constructor
    AffineOp (scalemode eInterpolate=interpolateLIN);
    /// resets the internal Matrix
    /** to
        <pre>
        1 0 0
        0 1 0
        0 0 
        </pre>
    */
    void reset  ();
    /// adds a rotation
    /** @param dAngle angle in degrees (clockwise) 
    */
    void rotate (double dAngle);
    
    ///adds a traslation
    /** @param x pixels to translate in x-direction
        @param y pixels to translate in y-direction
        */
    void translate (double x, double y) {
      m_aadT[0][2] += x; m_aadT[1][2] += y;
    }
    /// adds a scale
    /** @param x scale-factor in x-direction
        @param y scale-factor in y-direction
        different values for x and y will lead to a dilation / upsetting deformation
        */
    void scale (double x, double y) {
      m_aadT[0][0] *= x; m_aadT[1][0] *= x;
      m_aadT[0][1] *= y; m_aadT[1][1] *= y;
    }
    
    /// Applies the affine transform to the image
    virtual void apply (const ImgBase *poSrc, ImgBase **ppoDst);

    /// import from super-class
    BaseAffineOp::apply;

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
    scalemode m_eInterpolate;
  };
}


#endif
