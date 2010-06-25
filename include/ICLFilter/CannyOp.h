/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/CannyOp.h                            **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#ifndef CANNY_H
#define CANNY_H

#ifdef HAVE_IPP


#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>

namespace icl {
  
  /// Class for the canny edge detector (IPP only!) \ingroup UNARY
  /** @section OV Overview

      The canny edge detector detects image borders from gray-scale images. It's result
      is an Img8u binary image containing thin borders.

      @section AL Algorithm
      The canny edge detector is a very common filter for edge detection, therefore it is
      already implemented in the IPP. Currently no non-IPP implementation is available.
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
      has too hard edges (e.g. from edges from black to white). In this case, the IPP canny edge 
      detector implementation overlooks these borders independent on the given threshold values.
      

  */
  class CannyOp : public UnaryOp, public Uncopyable{
    public:
      /// Constructor
      /**
        With this Constructor the derivations are computed within the CannyOp. 
        If you already have computed the derivations, use the other Constructor, due to performance reasons.
        @param lowThresh lower threshold
        @param highThresh upper threshold
        @param preBlur value for the preBlur flag
      */
    CannyOp(icl32f lowThresh=0, icl32f highThresh=255, bool preBlur=false);
      /// Constructor
      /**
        @param dxOp the x derivation of the src
        @param dyOp the y derivation of the src
        @param lowThresh lower threshold
        @param highThresh upper threshold
        @param deleteOps should the internaly created derivations be deleted?
        @param preBlur value for the preBlur flag
      */
    CannyOp(UnaryOp *dxOp, UnaryOp *dyOp, icl32f lowThresh=0, icl32f highThresh=255, bool deleteOps=true, bool preBlur=false);

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
    virtual void apply(const ImgBase *src, ImgBase **dst);
        
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    /// sets pre-blur feature enabled or disabled
    void setPreBlur(bool enabled){
      m_preBlur = enabled;
    }

    /// returns current pre-blur feature state
    bool getPreBlur() const {
      return m_preBlur;
    }

    private:
    /// buffer for ippiCanny
    std::vector<icl8u> m_cannyBuf;
    ImgBase *m_derivatives[2];
    UnaryOp *m_ops[2];
    icl32f m_lowT,m_highT;
    bool m_ownOps;
    Img32f m_buffer;
    bool m_preBlur;
    ImgBase *m_preBlurBuffer;

  };
} // namespace icl


#endif 
// HAVE_IPP

#endif 
// guard


