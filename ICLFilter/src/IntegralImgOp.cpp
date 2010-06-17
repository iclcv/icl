/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/IntegralImgOp.cpp                        **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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

#include <ICLFilter/IntegralImgOp.h>
namespace icl{

  
  IntegralImgOp::IntegralImgOp(depth d):
    // {{{ open
    m_integralImageDepth(d),m_buf(0){
  }

  // }}}

  IntegralImgOp::~IntegralImgOp(){
    // {{{ open
    ICL_DELETE(m_buf);
  }
  // }}} 
 
  void IntegralImgOp::setIntegralImageDepth(depth integralImageDepth){
    // {{{ open

    m_integralImageDepth = integralImageDepth;
  }

  // }}}
  depth IntegralImgOp::getIntegralImageDepth() const{
    // {{{ open

    return m_integralImageDepth;
  }

  // }}}


  template<class S,class  D>
  static void create_integral_channel_cpp(const S *image,int w, int h, D *intImage){
    // {{{ open
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
      for(int n = ((int)(sEnd - s)) >> 4; n>0; --n){
        STEP STEP STEP STEP STEP STEP STEP STEP
        STEP STEP STEP STEP STEP STEP STEP STEP
      }
      while(s < sEnd){ STEP; }
#undef STEP

    } 
  }

  // }}}

  template<class S, class D>
  static void create_integral_image_sd(const Img<S> &src, Img<D> &dst, ImgBase**){
    // {{{ open
    for(int c=src.getChannels()-1;c>=0;--c){
      create_integral_channel_cpp(src.begin(c), src.getWidth(), src.getHeight(), dst.begin(c));
    }
  }
  // }}} 

#ifdef HAVE_IPP_DEACTIVATED_BECAUSE_IT_IS_MUCH_SLOWER

  template<class S, class D, class B, class IPP_FUNC>
  void create_integral_image_ipp(const Img<S> &src, Img<D> &dst, ImgBase **buf,  IPP_FUNC ippfunc){
    // {{{ open

    ensureCompatible(buf,getDepth<B>(), src.getSize()+Size(1,1), 1, formatMatrix, Rect::null);
    Img<B> &dbuf = *(*buf)->asImg<B>();
    
    for(int c=src.getChannels()-1;c>=0;--c){
      IppStatus s = ippfunc(src.begin(c),src.getLineStep(),dbuf.begin(0),dbuf.getLineStep(), src.getSize(), 0);
      if(s != ippStsNoErr) throw ICLException("error in ippiIntegral_8u32s_C1R in " +std::string(__FUNCTION__) + ":" +std::string(ippGetStatusString(s)));
      dbuf.setROI(Rect(Point(1,1),src.getSize()));
      TODO_LOG("this does not work in case of N>1 channel-images!!");
      if(dst.getDepth() == dbuf.getDepth()){ // i really wonder why this lasts 2 msec??
        dbuf.deepCopyROI(bpp(dst));
      }else{
        dbuf.convertROI(&dst);
      }
      dbuf.setFullROI();
    }    
  }

  // }}}

  template<> void create_integral_image_sd<icl8u,icl32s>(const Img<icl8u> &src, Img<icl32s> &dst, ImgBase **buf){
    // {{{ open

    create_integral_image_ipp<icl8u,icl32s,icl32s>(src,dst, buf, ippiIntegral_8u32s_C1R);
  }

  // }}}
  template<> void create_integral_image_sd<icl8u,icl32f>(const Img<icl8u> &src, Img<icl32f> &dst, ImgBase **buf){
    // {{{ open

    create_integral_image_ipp<icl8u,icl32f,icl32f>(src,dst, buf, ippiIntegral_8u32f_C1R);
  }

  // }}}
  template<> void create_integral_image_sd<icl8u,icl64f>(const Img<icl8u> &src, Img<icl64f> &dst, ImgBase **buf){
    // {{{ open

    create_integral_image_ipp<icl8u,icl64f,icl32f>(src,dst, buf, ippiIntegral_8u32f_C1R);
  }

  // }}}

#endif

  template<class D>
  static void create_integral_image_xd(const ImgBase *src, Img<D> &dst, ImgBase **buf){
    // {{{ open

    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: create_integral_image_sd(*src->asImg<icl##D>(), dst, buf) ; break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  // }}}
  
  
  void IntegralImgOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( poSrc != *ppoDst );
    
    if(!prepare(ppoDst,  m_integralImageDepth, poSrc->getSize(),
                formatMatrix, poSrc->getChannels(), Rect::null)){
      ERROR_LOG("unable to prepare destination image");
      return;
    } 
    
    switch(m_integralImageDepth){
      case depth32s:
        create_integral_image_xd(poSrc, *(*ppoDst)->asImg<icl32s>(), &m_buf);
        break;
      case depth32f:
        create_integral_image_xd(poSrc, *(*ppoDst)->asImg<icl32f>(), &m_buf);
        break;
      case depth64f:
        create_integral_image_xd(poSrc, *(*ppoDst)->asImg<icl64f>(), &m_buf);
        break;
      default:
        ERROR_LOG("integral image destination depth must be 32s, 32f, or 64f");
        ICL_INVALID_DEPTH;
    }
    
  }

  // }}}


}
