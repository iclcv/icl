// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus, Sergius Gaulik

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Image.h>

namespace icl::filter {
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
    class ICLFilter_API CannyOp : public UnaryOp{
      public:
      CannyOp(const CannyOp&) = delete;
      CannyOp& operator=(const CannyOp&) = delete;

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
      void apply(const core::Image &src, core::Image &dst) override;

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

      /// applies canny for one channel (C++ fallback)
      void applyCanny32f(const core::Img32f &dx, const core::Img32f &dy, core::Img8u &dst, int c);
      void applyCanny16s(const core::Img16s &dx, const core::Img16s &dy, core::Img8u &dst, int c);

      /// core canny logic shared by both apply overloads
      void applyCannyCore(const core::Image &derivX, const core::Image &derivY,
                          core::Image &dst, const core::Image &srcForMeta);

      /// buffer for ippiCanny
      std::vector<icl8u> m_cannyBuf;
      core::Image m_derivatives[2];
      core::Image m_legacyResult;  ///< keeps 3-arg apply result alive
      UnaryOp *m_ops[2];
      UnaryOp *m_preBlurOp;
      icl32f m_lowT,m_highT;
      bool m_ownOps;
      bool m_use_derivatives_info;
      int m_preBlurRadius;
    };
  } // namespace icl::filter