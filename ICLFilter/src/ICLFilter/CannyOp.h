/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/CannyOp.h                      **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus, Sergius Gaulik      **
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
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>

namespace icl {
  namespace filter{

    /// Class for the canny edge detector \ingroup UNARY
    /** @section OV Overview

        The canny edge detector detects image borders from gray-scale images. It's result
        is an Img8u binary image containing thin borders.

        @section AL Algorithm
        The canny edge detector is a very common filter for edge detection, therefore it is
        already implemented in the IPP.
        The algorithm can be split into 3 major parts:
        -# <b>Image differentiation</b> here, image x and y gradients are computed. Commonly this
           is done using sobel- X and -Y filters.
        -# <b> Non-Maximum suppression</b> The image differentiation result is converted into an
           image intensity map and into a gradient direction map. Now all pixels are suppressed,
           that are not <em>ridges</em> in gradient intensity map <em>mountain</em>.
        -# <b>Thresholding</b> Here a special threshold operation is used. Two threshold values
           (l=low threshold and h=high threshold) split the edge intensity scale into 3 parts:
           -# <em>below the lower threshold</em> pixels with such values are in any case no border
              pixels
           -# <em>above the upper threshold</em> these pixels are border pixels
           -# <em>in the middle section</em> these pixels are only border pixels if there's a
              connected chain of other pixels (each also in the middle section) that is connected
              in any way to a border pixel.

        (please see IPP's canny edge detector documentation for more detail)

        @section PB pre-blur features
        In some cases (e.g. if input images are created synthetically) the border intensity image
        has too hard edges (e.g. from edges from black to white). In this case, the canny edge
        detector implementation overlooks these borders independent on the given threshold values.


    */
    class ICLFilter_API CannyOp : public UnaryOp, public utils::Uncopyable{
      public:
        /// Constructor
        /**
          With this Constructor the derivations are computed within the CannyOp.
          If you already have computed the derivations, use the other Constructor, due to performance reasons.
          @param lowThresh lower threshold
          @param highThresh upper threshold
          @param preBlurRadius if r> 0, gaussian kernel with masksize r*2+1 is applied to the input image first
        */
      CannyOp(icl32f lowThresh=0, icl32f highThresh=255, int preBlurRadius=0);
        /// Constructor
        /**
          @param dxOp the x derivation of the src
          @param dyOp the y derivation of the src
          @param lowThresh lower threshold
          @param highThresh upper threshold
          @param deleteOps should the internaly created derivations be deleted?
          @param preBlurRadius if r> 0, gaussian kernel with masksize r*2+1 is applied to the input image first
        */
      CannyOp(UnaryOp *dxOp, UnaryOp *dyOp, icl32f lowThresh=0, icl32f highThresh=255, bool deleteOps=true, int preBlurRadius=0);

        /// Destructor
      virtual ~CannyOp();

      /// changes the Thresholds
      /**
          @param lowThresh lower threshold
          @param highThresh upper threshold
      */
      void setThresholds(icl32f lowThresh, icl32f highThresh);

      /// returns the lower threshold
      /**
          @return the lower threshold
      */
      icl32f getLowThreshold() const;

      /// returns the upper threshold
      /**
          @return the upper threshold
      */
      icl32f getHighThreshold() const;

      ///applies the Canny Operator
      /**
          @param src the source image
          @param dst pointer to the destination image
      */
      virtual void apply(const core::ImgBase *src, core::ImgBase **dst);

	  ///applies the Canny Operator
	  /**
		  @param src the source image
		  @param dst pointer to the destination image
	  */
	  virtual void apply(const core::ImgBase *src_x, const core::ImgBase *src_y, core::ImgBase **dst);

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      /// sets the pre-blur-radius
      /** if r> 0, gaussian kernel with masksize r*2+1 is applied to the input image first */
      void setPreBlurRadius(int preBlurRadius){
        m_preBlurRadius = preBlurRadius;
        ICL_DELETE(m_preBlurOp);
        setUpPreBlurOp();
      }

	  void setUseDerivativesInfo(bool use_derivatives) {
		  m_use_derivatives_info = use_derivatives;
	  }

      /// returns current pre-blur feature state
      bool getPreBlurRadius() const {
        return m_preBlurRadius;
      }

      private:
      
      void property_callback(const Property &p);

      void setUpPreBlurOp();

      /// applies canny for one channel
      void applyCanny32f(const core::ImgBase *dx, const core::ImgBase *dy, core::ImgBase *dst, int c);
      void applyCanny16s(const core::ImgBase *dx, const core::ImgBase *dy, core::ImgBase *dst, int c);

      /// buffer for ippiCanny
      std::vector<icl8u> m_cannyBuf;
      core::ImgBase *m_derivatives[2];
      UnaryOp *m_ops[2];
      UnaryOp *m_preBlurOp;
      icl32f m_lowT,m_highT;
      bool m_ownOps;
	  bool m_use_derivatives_info;
      core::Img32f m_buffer;
      int m_preBlurRadius;
    };
  } // namespace filter
} // namespace icl
