/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/GradientImage.h                **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>

namespace icl{
  namespace filter{

    /// Class for calculation of gradient image information
    /** The GradientImage class provides functionalities for
        the calculation of the following gradient image information:
        - X-Gradient (using 3x3 Sobel-X Convolution Kenrnel)
        - Y-Gradient (using 3x3 Sobel-Y Convolution Kenrnel)
        - Gradient Intensity (internally accelerated using a LUT)
          \f$ I=\sqrt(gx^2+gy^2) \f$
        - Gradient Angle (also accelerated using a LUT)
          \f$ I=atan2(gy,gx) \f$

        For best performance, the input image is restricted to be
        of type Img8u. The update function will firstly calculate
        X- and Y-Gradient information using a ConvolutionOp object.
        X- and Y-Gradient image have range [0,255] now (result of
        Sobel-Filter is internally scaled to the range of the
        used datatype icl8u. Further calculation of gradient intensity
        and gradient angle depends on the given calculation mode
        flag.
    */
    class ICLFilter_API GradientImage{
      public:

      /// flags to indicate which gradient information should be calculated
      enum calculationMode{
        calculateXYOnly=0,    ///< only calculate X- and Y-Gradient
        calculateIntensity=1, ///< X/Y-Gradient and intensity map
        calculateAngle=2,     ///< X/Y-Gradient and angle map
        calculateAll=3        ///< X/Y-Gradient, intensity- and angle map
      };

      /// Empty constructor
      GradientImage(){}

      /// Main function calculates new gradient image information
      void update(const core::ImgBase *src, calculationMode mode=calculateAll);

      /// return current X-Gradient image
      const core::Img16s &getGradXImage() const{ return m_oX; }

      /// return current Y-Gradient image
      const core::Img16s &getGradYImage() const{ return m_oY; }

      /// return current Gradient-Intensity image
      const core::Img32f &getGradIntensityImage() const { return m_oI; }

      /// return current Gradient-Angle image
      const core::Img32f &getGradAngleImage() const { return m_oA; }

      /// internally normalizes all images to range [0,255]
      void normalize();
      private:
      core::Img16s m_oBuf;
      core::Img16s m_oX,m_oY;
      core::Img32f m_oI,m_oA;
    };
  } // namespace filter
}

