// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus, Sergius Gaulik

#include <ICLFilter/CannyOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLUtils/SSEUtils.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    void CannyOp::property_callback(const Property &p){
    }

    CannyOp::CannyOp(icl32f lowThresh, icl32f highThresh,int preBlurRadius):
      m_lowT(lowThresh),m_highT(highThresh),m_ownOps(true),m_preBlurRadius(preBlurRadius){
      FUNCTION_LOG("");
      m_ops[0] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelX3x3));
      m_ops[1] = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::sobelY3x3));
      m_preBlurOp = 0;
      m_use_derivatives_info = false;
      setUpPreBlurOp();
      registerCallback([this](const Property &p){ property_callback(p); });
    }

    CannyOp::CannyOp(UnaryOp *dxOp, UnaryOp *dyOp,icl32f lowThresh, icl32f highThresh, bool deleteOps, int preBlurRadius):
      m_lowT(lowThresh),m_highT(highThresh),m_ownOps(deleteOps),m_preBlurRadius(preBlurRadius){
      FUNCTION_LOG("");
      m_ops[0] = dxOp;
      m_ops[1] = dyOp;
      m_preBlurOp = 0;
      m_use_derivatives_info = false;
      setUpPreBlurOp();
      registerCallback([this](const Property &p){ property_callback(p); });
    }

    CannyOp::~CannyOp(){
      FUNCTION_LOG("");
      for(int i=0;i<2;++i){
        if(m_ownOps){
          ICL_DELETE(m_ops[i]);
        }
      }
      ICL_DELETE(m_preBlurOp);
    }

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
          for(int y=-r;y<=r;++y){
            for(int x=-r;x<=r;++x){
              kc(x+r,y+r) = 10000/(2*M_PI*r) * exp (float(x*x+y*y)/float(2*r*r));
            }
          }
          m_preBlurOp = new ConvolutionOp(ConvolutionKernel(k.begin(0),s,10000));
      }
    }

#ifdef SSE3
    inline void subCalcMag(const icl32f *dx, const icl32f *dy, icl32f *mag) {
      *mag = fabs(*dx) + fabs(*dy);
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

    static void followEdge(icl8u *mag, int w) {
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

    void CannyOp::applyCanny32f(const Img32f &dx, const Img32f &dy, Img8u &dst, int c) {
      icl32f low  = m_lowT;
      icl32f high = m_highT;
      const icl32f *src0 = dx.getData(c);
      const icl32f *src1 = dy.getData(c);
      icl8u *dstBase = dst.getROIData(c);
      int dxW = dx.getWidth();
      int dxH = dx.getHeight();
      int dxDim = dx.getDim();
      int dstStride = dst.getWidth();

      std::vector<icl32f> magVec(dxDim);
      icl32f *mag = magVec.data();

#ifdef SSE3
      sse_for(src0, src1, mag, mag+dxDim, subCalcMag, subCalcMag, 4, 4);
#else
      for (int i = 0; i < dxDim; ++i) {
        mag[i] = fabs(src0[i]) + fabs(src1[i]);
      }
#endif

      // set borders to invalid values (in magnitude array, using derivative stride)
      for (int i = 0; i < dxW; ++i) {
        mag[i] = -1.0f;
        mag[dxDim - dxW + i] = -1.0f;
      }
      for (int i = 0; i < dxDim; i += dxW) {
        mag[i] = -1.0f;
        mag[i + dxW - 1] = -1.0f;
      }

      // non-maximum-suppression: mag uses derivative stride, dst uses dst stride
      for (int y = 0; y < dxH; ++y) {
        for (int x = 0; x < dxW; ++x) {
          int i = y * dxW + x;
          icl8u &out = dstBase[y * dstStride + x];
          if (mag[i] >= low) {
            float dir = src0[i] / src1[i];
            if (fabs(dir) >= 2.414213562373095f) {
              if (mag[i] <= mag[i-1] || mag[i] < mag[i+1]) { out = 0; continue; }
            } else if (dir > 0.4142135623730950f) {
              if (mag[i] <= mag[i-1-dxW] || mag[i] < mag[i+1+dxW]) { out = 0; continue; }
            } else if (dir < -0.4142135623730950f) {
              if (mag[i] <= mag[i-1+dxW] || mag[i] < mag[i+1-dxW]) { out = 0; continue; }
            } else {
              if (mag[i] <= mag[i-dxW] || mag[i] < mag[i+dxW]) { out = 0; continue; }
            }
            out = (mag[i] > high) ? 2 : 1;
          } else {
            out = 0;
          }
        }
      }

      // hysteresis thresholding (followEdge uses dst stride for neighbor access)
      for (int y = 0; y < dxH; ++y) {
        for (int x = 0; x < dxW; ++x) {
          icl8u &px = dstBase[y * dstStride + x];
          if (px == 2) followEdge(&px, dstStride);
        }
      }
    }

    void CannyOp::applyCanny16s(const Img16s &dx, const Img16s &dy, Img8u &dst, int c) {
      icl16s low  = m_lowT;
      icl16s high = m_highT;
      const icl16s *src0 = dx.getData(c);
      const icl16s *src1 = dy.getData(c);
      icl8u *dstBase = dst.getROIData(c);
      int dxW = dx.getWidth();
      int dxH = dx.getHeight();
      int dxDim = dx.getDim();
      int dstStride = dst.getWidth();

      std::vector<icl16s> magVec(dxDim);
      icl16s *mag = magVec.data();

#ifdef SSE3
      sse_for(src0, src1, mag, mag+dxDim, subCalcMag, subSSECalcMag, 8, 8);
#else
      for (int i = 0; i < dxDim; ++i) {
        mag[i] = abs(src0[i]) + abs(src1[i]);
      }
#endif

      // set borders to invalid values (in magnitude array, using derivative stride)
      for (int i = 0; i < dxW; ++i) {
        mag[i] = -1;
        mag[dxDim - dxW + i] = -1;
      }
      for (int i = 0; i < dxDim; i += dxW) {
        mag[i] = -1;
        mag[i + dxW - 1] = -1;
      }

      // non-maximum-suppression: mag uses derivative stride, dst uses dst stride
      for (int y = 0; y < dxH; ++y) {
        for (int x = 0; x < dxW; ++x) {
          int i = y * dxW + x;
          icl8u &out = dstBase[y * dstStride + x];
          if (mag[i] >= low) {
            float dir = src0[i] / static_cast<float>(src1[i]);
            if (fabs(dir) >= 2.414213562373095f) {
              if (mag[i] < mag[i-1] || mag[i] < mag[i+1]) { out = 0; continue; }
            } else if (dir > 0.4142135623730950f) {
              if (mag[i] < mag[i-1-dxW] || mag[i] < mag[i+1+dxW]) { out = 0; continue; }
            } else if (dir < -0.4142135623730950f) {
              if (mag[i] < mag[i-1+dxW] || mag[i] < mag[i+1-dxW]) { out = 0; continue; }
            } else {
              if (mag[i] < mag[i-dxW] || mag[i] < mag[i+dxW]) { out = 0; continue; }
            }
            out = (mag[i] > high) ? 2 : 1;
          } else {
            out = 0;
          }
        }
      }

      // hysteresis thresholding (followEdge uses dst stride for neighbor access)
      for (int y = 0; y < dxH; ++y) {
        for (int x = 0; x < dxW; ++x) {
          icl8u &px = dstBase[y * dstStride + x];
          if (px == 2) followEdge(&px, dstStride);
        }
      }
    }


    void CannyOp::applyCannyCore(const Image &derivX, const Image &derivY,
                                 Image &dst, const Image &srcForMeta) {
      Size dstSize;
      Rect dstROI;
      format dstFmt;
      int dstCh;
      if(getClipToROI()) {
        dstSize = derivX.getSize();
        dstROI = Rect(Point::null, dstSize);
        dstFmt = derivX.getFormat();
        dstCh = derivX.getChannels();
      } else {
        dstSize = srcForMeta.getSize();
        dstROI = Rect(Point(1,1), derivX.getSize());
        const Image &metaSrc = m_use_derivatives_info ? derivX : srcForMeta;
        dstFmt = metaSrc.getFormat();
        dstCh = metaSrc.getChannels();
      }
      if(!prepare(dst, depth8u, dstSize, dstFmt, dstCh, dstROI,
                  srcForMeta.getTime())) return;

      Img8u &d = dst.as8u();

      for(int c = derivX.getChannels()-1; c >= 0; --c) {
        switch(derivX.getDepth()) {
          case depth32f:
            applyCanny32f(derivX.as32f(), derivY.as32f(), d, c);
            break;
          case depth16s:
            applyCanny16s(derivX.as16s(), derivY.as16s(), d, c);
            break;
          default: ICL_INVALID_DEPTH;
        }
      }
    }

    void CannyOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN(!src.isNull());

      const Image *input = &src;
      if(m_preBlurRadius > 0) {
        input = &m_preBlurOp->apply(src);
      }

      for(int i = 0; i < 2; i++){
        m_ops[i]->setClipToROI(true);
        m_ops[i]->apply(*input, m_derivatives[i]);
      }

      applyCannyCore(m_derivatives[0], m_derivatives[1], dst, *input);
    }

    void CannyOp::apply(const ImgBase *src_x, const ImgBase *src_y, ImgBase **ppoDst) {
      FUNCTION_LOG("");
      ICLASSERT_RETURN( src_x );
      ICLASSERT_RETURN( src_y );
      ICLASSERT_RETURN( ppoDst );
      ICLASSERT_RETURN( src_x->getChannels() == 1 );
      ICLASSERT_RETURN( src_y->getChannels() == 1 );

      Image imgX(*src_x), imgY(*src_y);
      const Image *inputX = &imgX;
      const Image *inputY = &imgY;

      if(m_preBlurRadius > 0){
        const Image &bx = m_preBlurOp->apply(*inputX);
        inputX = &bx;
        // Note: can't blur both with same op (overwrites internal buffer),
        // but the original code had the same limitation with static locals
      }

      m_ops[0]->setClipToROI(true);
      m_ops[0]->apply(*inputX, m_derivatives[0]);
      m_ops[1]->setClipToROI(true);
      m_ops[1]->apply(*inputY, m_derivatives[1]);

      // Use member to keep result alive past this call
      m_legacyResult = (ppoDst && *ppoDst) ? Image(**ppoDst) : Image();
      applyCannyCore(m_derivatives[0], m_derivatives[1], m_legacyResult, *inputX);
      *ppoDst = m_legacyResult.ptr();
    }


    void CannyOp::setThresholds(icl32f lo, icl32f hi){
      m_lowT = lo;
      m_highT = hi;
    }

    icl32f CannyOp::getLowThreshold()const {
      return m_lowT;
    }

    icl32f CannyOp::getHighThreshold()const {
      return m_highT;
    }

  } // namespace filter
}
