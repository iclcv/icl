/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCore/Bayer.h                                **
** Module : ICLCore                                                **
** Authors: Michael Götting, Felix Reinhard, Christof Elbrechter   **
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

#ifndef ICL_BAYER_CONVERTER_H
#define ICL_BAYER_CONVERTER_H

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>

namespace icl {
  namespace core{
  
    /// Utiltity class for bayer pattern conversion
    /** The internal implementation was basically taken from
        the libdc files */
    class BayerConverter : public Uncopyable{
      public:
      
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
      
      /// creates a new BayerConverter instances
      /** The given size hint can be used to accellerate the first apply call, where
          the internal working buffer's size is adapted on demand */
      BayerConverter(bayerConverterMethod eConvMethod, 
                     bayerPattern eBayerPattern, 
                     Size sizeHint = Size::null);
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
      
      static std::string translateBayerConverterMethod(bayerConverterMethod ebcm);
      static bayerConverterMethod translateBayerConverterMethod(std::string sbcm);
      
      static std::string translateBayerPattern(bayerPattern ebp);
      static bayerPattern translateBayerPattern(std::string sbp);
      
      private:
      /// internal buffer;
      std::vector<icl8u> m_buffer;
  
      bayerConverterMethod m_eConvMethod;
      bayerPattern m_eBayerPattern;
      #ifdef HAVE_IPP
        IppiBayerGrid m_IppBayerPattern;
      #endif
      
      // Interpolation methods
      void nnInterpolation(const Img8u *poBayerImg);
      void bilinearInterpolation(const Img8u *poBayerImg);
      void hqLinearInterpolation(const Img8u *poBayerImg);
      void edgeSenseInterpolation(const Img8u *poBayerImg);
      void simpleInterpolation(const Img8u *poBayerImg);
      #ifdef HAVE_IPP
        void nnInterpolationIpp(const Img8u *poBayerImg);
      #endif
      
      // Utility functions
      void clearBorders(icl8u *rgb, int sx, int sy, int w);
      inline void clip(int *iIn, icl8u *iOut) {
        *iOut = *iIn = *iIn < 0 ? 0 : *iIn > 255 ? 255 : 0;
      }
    };
   
  } // namespace core
} // namespace icl

#endif
