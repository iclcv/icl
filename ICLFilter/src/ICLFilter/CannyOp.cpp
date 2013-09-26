/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/CannyOp.cpp                    **
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

#include <ICLFilter/CannyOp.h>
#include <ICLCore/Img.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLUtils/SSEUtils.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    // without ipp non of the function is implemented
    CannyOp::CannyOp(icl32f lowThresh, icl32f highThresh,int preBlurRadius):
      // {{{ open
      m_lowT(lowThresh),m_highT(highThresh),m_ownOps(true),m_preBlurRadius(preBlurRadius){
      FUNCTION_LOG("");
      m_ops[0] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelX3x3));
      m_ops[1] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelY3x3));
      m_derivatives[0]=m_derivatives[1]=0;
      m_preBlurOp = 0;
      setUpPreBlurOp();
    }

    // }}}

    CannyOp::CannyOp(UnaryOp *dxOp, UnaryOp *dyOp,icl32f lowThresh, icl32f highThresh, bool deleteOps, int preBlurRadius):
      // {{{ open
      m_lowT(lowThresh),m_highT(highThresh),m_ownOps(deleteOps),m_preBlurRadius(preBlurRadius){
      FUNCTION_LOG("");
      m_ops[0] = dxOp;
      m_ops[1] = dyOp;

      m_derivatives[0]=m_derivatives[1]=0;

      m_preBlurOp = 0;
      setUpPreBlurOp();
    }

    // }}}

    CannyOp::~CannyOp(){
      // {{{ open

      FUNCTION_LOG("");

      for(int i=0;i<2;++i){
        if(m_ownOps){
          ICL_DELETE(m_ops[i]);
        }
        ICL_DELETE(m_derivatives[i]);
      }
      ICL_DELETE(m_preBlurOp);
    }

    // }}}

    void CannyOp::setUpPreBlurOp(){
      int r = m_preBlurRadius;
      if(r <=0) return;
      switch(r){
        case 1: m_preBlurOp = new ConvolutionOp(ConvolutionKernel::gauss3x3); break;
        case 2: m_preBlurOp = new ConvolutionOp(ConvolutionKernel::gauss5x5); break;
        default:
          WARNING_LOG("higher pre blur radii than 2 are not yet supported correctly ...");
          Size s(2*r+1,2*r+1);
          Img32s k(s,1);
          Channel32s kc = k[0];
          int sum;
          for(int y=-r;y<=r;++y){
            for(int x=-r;x<=r;++x){
              kc(x+r,y+r) = 10000/(2*M_PI*r) * exp (float(x*x+y*y)/float(2*r*r));
              sum += kc(x+r,y+r);
            }
          }
          m_preBlurOp = new ConvolutionOp(ConvolutionKernel(k.begin(0),s,10000));
      }
    }

  #ifdef SSE3

    inline void subCalcMag(const icl32f *dx, const icl32f *dy, icl32f *mag) {
      *mag = fabs(*dx) + fabs(*dy);
//      *mag = sqrt(*dx * *dx + *dy * *dy);
    }

    inline void subSSECalcMag(const icl32f *dx, const icl32f *dy, icl32f *mag) {
      icl128 vDx = abs(icl128(dx));
      icl128 vDy = abs(icl128(dy));

      vDx += vDy;

      vDx.storeu(mag);
    }

    inline void subCalcMag(const icl16s *dx, const icl16s *dy, icl16s *mag) {
      *mag = abs(*dx) + abs(*dy);
    }

    inline void subSSECalcMag(const icl16s *dx, const icl16s *dy, icl16s *mag) {
      icl128i vDx = abs16(icl128i(dx));
      icl128i vDy = abs16(icl128i(dy));

      vDx.add16(vDy);

      vDx.storeu(mag);
    }

  #endif

    void followEdge(icl8u *mag, int w) {
      *mag = 255;
      if (mag[-w-1] == 1) followEdge(&mag[-w-1], w);
      if (mag[-w] == 1) followEdge(&mag[-w], w);
      if (mag[-w+1] == 1) followEdge(&mag[-w+1], w);
      if (mag[-1] == 1) followEdge(&mag[-1], w);
      if (mag[1] == 1) followEdge(&mag[1], w);
      if (mag[w-1] == 1) followEdge(&mag[w-1], w);
      if (mag[w] == 1) followEdge(&mag[w], w);
      if (mag[w+1] == 1) followEdge(&mag[w+1], w);
    }

    void CannyOp::applyCanny32f(const ImgBase *dx, const ImgBase *dy, ImgBase *dst, int c) {
      icl32f low  = m_lowT;
      icl32f high = m_highT;
      const icl32f *src0 = dx->asImg<icl32f>()->getData(c);
      const icl32f *src1 = dy->asImg<icl32f>()->getData(c);
      icl8u *dst0 = dst->asImg<icl8u>()->getData(c);

      int w = dx->getWidth();

      icl32f *mag = (icl32f*)malloc(dx->getDim()*sizeof(icl32f));

    #ifdef SSE3
      // calculate magnitudes
      sse_for(src0, src1, mag, mag+dx->getDim(), subCalcMag, subCalcMag, 4, 4);
    #else
      icl32f *it = mag;
      icl32f *itEnd = mag + dx->getDim();
      for (const icl32f *sIt0 = src0, *sIt1 = src1; it != itEnd; ++it, ++sIt0, ++sIt1) {
        *it = fabs(*sIt0) + fabs(*sIt1);
      }
    #endif

      // set borders to unvalid values
      for (int i = 0; i < w; ++i) {
        mag[i] = -1.0f;
        mag[dx->getDim() - w + i] = -1.0f;
      }
      for (int i = 0; i < dx->getDim(); i += w) {
        mag[i] = -1.0f;
        mag[i + w - 1] = -1.0f;
      }

      // non-maximum-suppression with filtering of low magnitude values
      for (int i = 0; i < dx->getDim(); ++i) {
        if (mag[i] >= low) {
          float dir = src0[i] / src1[i];

          if (fabs(dir) >= 2.414213562373095f) {
            if (mag[i] <= mag[i-1] || mag[i] < mag[i+1]) {
              dst0[i] = 0;
              continue;
            }
          } else if (dir > 0.4142135623730950f) {
            if (mag[i] <= mag[i-1-w] || mag[i] < mag[i+1+w]) {
              dst0[i] = 0;
              continue;
            }
          } else if (dir < -0.4142135623730950f) {
            if (mag[i] <= mag[i-1+w] || mag[i] < mag[i+1-w]) {
              dst0[i] = 0;
              continue;
            }
          } else {
            if (mag[i] <= mag[i-w] || mag[i] < mag[i+w]) {
              dst0[i] = 0;
              continue;
            }
          }

          if (mag[i] > high) dst0[i] = 2;
          else dst0[i] = 1;
        } else {
          dst0[i] = 0;
        }
      }

      // hysteresis thresholding
      icl8u *ittEnd = &dst0[dx->getDim()-1];
      for (;dst0 != ittEnd; ++dst0) {
        if (*dst0 == 2) {
          followEdge(dst0, w);
        }
      }

      free(mag);
    }

    void CannyOp::applyCanny16s(const ImgBase *dx, const ImgBase *dy, ImgBase *dst, int c) {
      icl16s low  = m_lowT;
      icl16s high = m_highT;
      const icl16s *src0 = dx->asImg<icl16s>()->getData(c);
      const icl16s *src1 = dy->asImg<icl16s>()->getData(c);
      icl8u *dst0 = dst->asImg<icl8u>()->getData(c);

      int w = dx->getWidth();

      icl16s *mag = (icl16s*)malloc(dx->getDim()*sizeof(icl16s));

    #ifdef SSE3
      // calculate magnitudes
      sse_for(src0, src1, mag, mag+dx->getDim(), subCalcMag, subSSECalcMag, 8, 8);
    #else
      icl16s *it = mag;
      icl16s *itEnd = mag + dx->getDim();
      for (const icl16s *sIt0 = src0, *sIt1 = src1; it != itEnd; ++it, ++sIt0, ++sIt1) {
        *it = abs(*sIt0) + abs(*sIt1);
      }
    #endif

      // set borders to unvalid values
      for (int i = 0; i < w; ++i) {
        mag[i] = -1;
        mag[dx->getDim() - w + i] = -1;
      }
      for (int i = 0; i < dx->getDim(); i += w) {
        mag[i] = -1;
        mag[i + w - 1] = -1;
      }

      // non-maximum-suppression with filtering of low magnitude values
      for (int i = 0; i < dx->getDim(); ++i) {
        if (mag[i] >= low) {
          float dir = src0[i] / (float)(src1[i]);

          if (fabs(dir) >= 2.414213562373095f) {
            if (mag[i] <= mag[i-1] || mag[i] < mag[i+1]) {
              dst0[i] = 0;
              continue;
            }
          } else if (dir > 0.4142135623730950f) {
            if (mag[i] <= mag[i-1-w] || mag[i] < mag[i+1+w]) {
              dst0[i] = 0;
              continue;
            }
          } else if (dir < -0.4142135623730950f) {
            if (mag[i] <= mag[i-1+w] || mag[i] < mag[i+1-w]) {
              dst0[i] = 0;
              continue;
            }
          } else {
            if (mag[i] <= mag[i-w] || mag[i] < mag[i+w]) {
              dst0[i] = 0;
              continue;
            }
          }

          if (mag[i] > high) dst0[i] = 2;
          else dst0[i] = 1;
        } else {
          dst0[i] = 0;
        }
      }

      // hysteresis thresholding
      icl8u *ittEnd = &dst0[dx->getDim()-1];
      for (;dst0 != ittEnd; ++dst0) {
        if (*dst0 == 2) {
          followEdge(dst0, w);
        }
      }

      free(mag);
    }

    /// no CannyOp::apply without ipp

    void CannyOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
        // {{{ open
      FUNCTION_LOG("");
      ICLASSERT_RETURN( poSrc );
      ICLASSERT_RETURN( ppoDst );
      ICLASSERT_RETURN( poSrc != *ppoDst);

      if(m_preBlurRadius>0){
        poSrc = m_preBlurOp->apply(poSrc);
      }

      for(int i=0;i<2;i++){
        m_ops[i]->setClipToROI (true);
        m_ops[i]->apply(poSrc,&m_derivatives[i]);
      }

      if (!prepare (ppoDst, m_derivatives[0], depth8u)) return;

  #ifdef HAVE_IPP
      int minSize=0;
      ippiCannyGetSize(m_derivatives[0]->getSize(), &minSize);
      m_cannyBuf.resize(minSize);
      for (int c=m_derivatives[0]->getChannels()-1; c >= 0; --c) {
        switch(m_derivatives[0]->getDepth()){
          case depth32f:
            ippiCanny_32f8u_C1R (m_derivatives[0]->asImg<icl32f>()->getROIData(c), m_derivatives[0]->getLineStep(),
                                 m_derivatives[1]->asImg<icl32f>()->getROIData(c), m_derivatives[1]->getLineStep(),
                                 (*ppoDst)->asImg<icl8u>()->getROIData(c), (*ppoDst)->getLineStep(),
                                 (*ppoDst)->getROISize(),m_lowT,m_highT,m_cannyBuf.data());
            break;
          case depth16s:
            ippiCanny_16s8u_C1R (m_derivatives[0]->asImg<icl16s>()->getROIData(c), m_derivatives[0]->getLineStep(),
                                 m_derivatives[1]->asImg<icl16s>()->getROIData(c), m_derivatives[1]->getLineStep(),
                                 (*ppoDst)->asImg<icl8u>()->getROIData(c), (*ppoDst)->getLineStep(),
                                 (*ppoDst)->getROISize(),m_lowT,m_highT,m_cannyBuf.data());
            break;
          default:
            ICL_INVALID_DEPTH;
        }
      }
  #else
      for (int c=m_derivatives[0]->getChannels()-1; c >= 0; --c) {
        switch(m_derivatives[0]->getDepth()){
          case depth32f:
            applyCanny32f(m_derivatives[0], m_derivatives[1], *ppoDst, c);
            break;
          case depth16s:
            applyCanny16s(m_derivatives[0], m_derivatives[1], *ppoDst, c);
            break;
          default:
            ICL_INVALID_DEPTH;
        }
      }
  #endif

    }
     // }}}

    void CannyOp::setThresholds(icl32f lo, icl32f hi){
      // {{{ open

      m_lowT = lo;
      m_highT = hi;
    }

    // }}}

    icl32f CannyOp::getLowThreshold()const {
      // {{{ open

      return m_lowT;
    }

    // }}}
    icl32f CannyOp::getHighThreshold()const {
      // {{{ open

      return m_highT;
    }

    // }}}


  } // namespace filter
}

