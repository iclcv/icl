/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/AffineOp.h                     **
** Module : ICLFilter                                              **
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

#pragma once

#include <ICLFilter/BaseAffineOp.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace filter{
    /// Class to apply an arbitrary series of affine transformations \ingroup AFFINE \ingroup UNARY
    /** Affine operations apply transform pixel locations using affine matrix transformation.
        To optimize performance concatenated affine transformation's matrices are pre-multiplied.
        \section ARI Adapt Result Image
         
        In some cases, it might make sense to automatically adapt the destinate image so
        that it contains the whole warping result image. Consider the following example
        
        
        <pre>
        source image:
                                           +
        A---------+                   +--/---\--+
        |         |                   |/       \|
        |         |    -------->     /|         |\
        |    A    |     rot(45)     + |    A'   | +
        |         |                  \|         |/
        |         |                   |\       /|
        +---------+                   +--\---/--+
                                           +
                                     
        </pre>     
        But obviously, the result image's corners are cut. If the "Adapt Result Image" option is
        set to true, the result image would be scaled to contain the whole rotated image and the
        affine matrix's shift is adpated in order to fit the result completely into the result image.
        If otherwise "Adpat Result Image is false, the result image's size will be identical to the
        source images size and the corners of the rotated image are cropped.
        
        <b>Please note:</b> this options is by default set to "true". Only the TranslateOp will set it
        to false by default, because a pure translation is just compensated completely by the result
        image adaption.
        
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
    class ICL_FILTER_API AffineOp : public BaseAffineOp, public utils::Uncopyable {
      public:
      /// Constructor
      AffineOp (core::scalemode eInterpolate=core::interpolateLIN);
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
      virtual void apply (const core::ImgBase *poSrc, core::ImgBase **ppoDst);
  
      /// import from super-class
      using BaseAffineOp::apply;
  
      /// sets whether the result image is is scaled and translated to contain the whole result image   
      /** @see \ref ARI */
      inline void setAdaptResultImage(bool on){
        m_adaptResultImage = on;
      }
  
      /// returns the Adapt Result image option
      /** @see \ref ARI */
      inline bool getAdaptResultImage() const{
        return m_adaptResultImage;
      }
      
      private:
      /// array of class methods used to transform depth8u and depth32f images
      void (AffineOp::*m_aMethods[core::depthLast+1])(const core::ImgBase *poSrc, core::ImgBase *poDst); 
      
      template<typename T>
      void affine (const core::ImgBase *poSrc, core::ImgBase *poDst);
      
      void applyT (const double p[2], double aResult[2]);
      static void useMinMax (const double aCur[2], 
                             double aMin[2], double aMax[2]);
      void getShiftAndSize (const utils::Rect& roi, utils::Size& size, 
                            double& xShift, double& yShift);
      double    m_aadT[2][3];
      core::scalemode m_eInterpolate;
      
      /// internal flag
      bool m_adaptResultImage;
    };
  } // namespace filter
}


