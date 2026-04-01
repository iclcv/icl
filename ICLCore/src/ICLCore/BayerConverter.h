// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Michael Goetting, Felix Reinhard, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>

namespace icl {
  namespace core{

    /// Utiltity class for bayer pattern conversion
    /** The internal implementation was basically taken from
        the libdc files */
    class ICLCore_API BayerConverter {
      public:
      BayerConverter(const BayerConverter&) = delete;
      BayerConverter& operator=(const BayerConverter&) = delete;


      enum bayerConverterMethod {
        nearestNeighbor = 0,
        simple,
        bilinear,
        hqLinear,
        edgeSense,
        vng
      };

      enum bayerPattern {
        bayerPattern_RGGB = 512,
        bayerPattern_GBRG,
        bayerPattern_GRBG,
        bayerPattern_BGGR
      };


      /// creates a bayer converter with given string-based parameters
      BayerConverter(const std::string &pattern="RGGB",
                     const std::string &method="bilinear");

      /// creates a new BayerConverter instances
      /** The given size hint can be used to accellerate the first apply call, where
          the internal working buffer's size is adapted on demand */
      BayerConverter(bayerPattern eBayerPattern,
                     bayerConverterMethod eConvMethod=bilinear,
                     const utils::Size &sizeHint = utils::Size::null);
      ~BayerConverter();

      /// converts the source image with bayer pattern into the
      /** given destination image. Dst will become an Img8u */
      void apply(const Img8u *src, ImgBase **dst);

      inline void setBayerPattern(bayerPattern eBayerPattern) {
        m_eBayerPattern = eBayerPattern;
      }

      inline void setConverterMethod(bayerConverterMethod eConvMethod) {
        m_eConvMethod = eConvMethod;
      }

      inline void setBayerPattern(const std::string &pattern){
        setBayerPattern(translateBayerPattern(pattern));
      }

      inline void setMethod(const std::string &method){
        setConverterMethod(translateBayerConverterMethod(method));
      }


      static std::string translateBayerConverterMethod(bayerConverterMethod ebcm);
      static bayerConverterMethod translateBayerConverterMethod(std::string sbcm);

      static std::string translateBayerPattern(bayerPattern ebp);
      static bayerPattern translateBayerPattern(std::string sbp);

      /// static utility method to convert a given bayer image to grayscale
      /** This is much faster than applying the standard bayer method,
          which will run though the image several times:
          - creating the interlearved rgb-image
          - converting the interleaved rgb-image to a planar rgb image
          - converting the planar rgb image to a one channel gray scale image

          The destination image is adapted in size and format
      **/
      static void convert_bayer_to_gray(const Img8u &src, Img8u &dst, const std::string &pattern);

      private:
      /// internal buffer;
      std::vector<icl8u> m_buffer;

      bayerConverterMethod m_eConvMethod;
      bayerPattern m_eBayerPattern;
      // Interpolation methods
      void nnInterpolation(const Img8u *poBayerImg);
      void bilinearInterpolation(const Img8u *poBayerImg);
      void hqLinearInterpolation(const Img8u *poBayerImg);
      void edgeSenseInterpolation(const Img8u *poBayerImg);
      void simpleInterpolation(const Img8u *poBayerImg);

      // Utility functions
      void clearBorders(icl8u *rgb, int sx, int sy, int w);
      inline void clip(int *iIn, icl8u *iOut) {
        *iOut = *iIn = *iIn < 0 ? 0 : *iIn > 255 ? 255 : 0;
      }
    };

  } // namespace core
} // namespace icl
