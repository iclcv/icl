// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/IntegralImgOp.h>
#include <icl/core/CoreFunctions.h>
#include <icl/core/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  IntegralImgOp::IntegralImgOp(depth d):
    m_integralImageDepth(d),m_buf(0){
  }


  IntegralImgOp::~IntegralImgOp(){
    ICL_DELETE(m_buf);
  }

  void IntegralImgOp::setIntegralImageDepth(depth integralImageDepth){

    m_integralImageDepth = integralImageDepth;
  }

  depth IntegralImgOp::getIntegralImageDepth() const{

    return m_integralImageDepth;
  }



  template<class S,class  D>
  static void create_integral_channel_cpp(const S *image,int w, int h, D *intImage){
    /* algorithm:
        +++++..
        +++CA..
        +++BX..
        .......
        X = src(x) + A + B - C
    */

    const S *src = image;
    D *dst = intImage;

    // first pixel
    *dst++ = D(*src++);

    // first row
    for(const S *srcEnd=src+w-1;src<srcEnd;++src,++dst){
      *dst = D(*src) + *(dst-1);
    }


    // rest of the image up to last row
    for(int y=1;y<h;++y){
      const S *s = image+y*w;
      const S * const sEnd = s+w;
      D *d = intImage+y*w;
      D *dl = d-w;

      // first pix in row
      *d = *dl + *s;
      ++s;
      ++d;
      ++dl;

#define STEP *d =  -*(dl-1) + *dl + *(d-1) + D(*s);  ++s; ++d; ++dl;

      // we use 16x loop-unrolling here. This is about 5% faster then 8x
      for(int n = (static_cast<int>(sEnd - s)) >> 4; n>0; --n){
        STEP STEP STEP STEP STEP STEP STEP STEP
        STEP STEP STEP STEP STEP STEP STEP STEP
      }
      while(s < sEnd){ STEP; }
#undef STEP

    }
  }


  template<class S, class D>
  static void create_integral_image_sd(const Img<S> &src, Img<D> &dst, ImgBase**){
    for(int c=src.getChannels()-1;c>=0;--c){
      create_integral_channel_cpp(src.begin(c), src.getWidth(), src.getHeight(), dst.begin(c));
    }
  }

  template<class D>
  static void create_integral_image_xd(const ImgBase *src, Img<D> &dst, ImgBase **buf){

    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: create_integral_image_sd(*src->asImg<icl##D>(), dst, buf) ; break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }



  void IntegralImgOp::apply(const Image &src, Image &dst) {
    ICLASSERT_RETURN(!src.isNull());
    ICLASSERT_RETURN(m_integralImageDepth == depth32s ||
                     m_integralImageDepth == depth32f ||
                     m_integralImageDepth == depth64f);

    if(!prepare(dst, m_integralImageDepth, src.getSize(), formatMatrix,
                src.getChannels(), Rect::null, src.getTime())) return;

    switch(m_integralImageDepth){
      case depth32s: {
        Img32s &d = dst.as32s();
        src.visit([&](const auto &s) { create_integral_image_sd(s, d, &m_buf); });
        break;
      }
      case depth32f: {
        Img32f &d = dst.as32f();
        src.visit([&](const auto &s) { create_integral_image_sd(s, d, &m_buf); });
        break;
      }
      case depth64f: {
        Img64f &d = dst.as64f();
        src.visit([&](const auto &s) { create_integral_image_sd(s, d, &m_buf); });
        break;
      }
      default: break;
    }
  }

  } // namespace icl::filter