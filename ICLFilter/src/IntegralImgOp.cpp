#include <ICLFilter/IntegralImgOp.h>
#include <cstring>
using std::memset;
using std::memcpy;

namespace icl{

  
  IntegralImgOp::IntegralImgOp(unsigned int borderSize, depth d):
    // {{{ open

    UnaryOp(),m_uiBorderSize(borderSize),m_eIntegralImageDepth(d){
  }

  // }}}
  void IntegralImgOp::setBorderSize(unsigned int borderSize){
    // {{{ open

    m_uiBorderSize = borderSize;
  }

  // }}}
  void IntegralImgOp::setIntegralImageDepth(depth integralImageDepth){
    // {{{ open

    m_eIntegralImageDepth = integralImageDepth;
  }

  // }}}
  unsigned int IntegralImgOp::getBorderSize() const{
    // {{{ open

    return m_uiBorderSize;
  }

  // }}}
  depth IntegralImgOp::getIntegralImageDepth() const{
    // {{{ open

    return m_eIntegralImageDepth;
  }

  // }}}
  
  void IntegralImgOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( poSrc != *ppoDst );
    
    if(!prepare(ppoDst,  m_eIntegralImageDepth, poSrc->getSize()+Size(2*m_uiBorderSize,2*m_uiBorderSize),
                formatMatrix, poSrc->getChannels(), Rect::null)) return;
    
    switch(m_eIntegralImageDepth){
      case depth32s:
        switch(poSrc->getDepth()){
          case depth8u: create(poSrc->asImg<icl8u>(),m_uiBorderSize,(*ppoDst)->asImg<icl32s>()); break;
          case depth16s: create(poSrc->asImg<icl16s>(),m_uiBorderSize,(*ppoDst)->asImg<icl32s>()); break;
          case depth32s: create(poSrc->asImg<icl32s>(),m_uiBorderSize,(*ppoDst)->asImg<icl32s>()); break;
          case depth32f: create(poSrc->asImg<icl32f>(),m_uiBorderSize,(*ppoDst)->asImg<icl32s>()); break;
          case depth64f: create(poSrc->asImg<icl64f>(),m_uiBorderSize,(*ppoDst)->asImg<icl32s>()); break;
        }
        break;
      case depth32f:
        switch(poSrc->getDepth()){
          case depth8u: create(poSrc->asImg<icl8u>(),m_uiBorderSize,(*ppoDst)->asImg<icl32f>()); break;
          case depth16s: create(poSrc->asImg<icl16s>(),m_uiBorderSize,(*ppoDst)->asImg<icl32f>()); break;
          case depth32s: create(poSrc->asImg<icl32s>(),m_uiBorderSize,(*ppoDst)->asImg<icl32f>()); break;
          case depth32f: create(poSrc->asImg<icl32f>(),m_uiBorderSize,(*ppoDst)->asImg<icl32f>()); break;
          case depth64f: create(poSrc->asImg<icl64f>(),m_uiBorderSize,(*ppoDst)->asImg<icl32f>()); break;
        }
        break;
      case depth64f:
        switch(poSrc->getDepth()){
          case depth8u: create(poSrc->asImg<icl8u>(),m_uiBorderSize,(*ppoDst)->asImg<icl64f>()); break;
          case depth16s: create(poSrc->asImg<icl16s>(),m_uiBorderSize,(*ppoDst)->asImg<icl64f>()); break;
          case depth32s: create(poSrc->asImg<icl32s>(),m_uiBorderSize,(*ppoDst)->asImg<icl64f>()); break;
          case depth32f: create(poSrc->asImg<icl32f>(),m_uiBorderSize,(*ppoDst)->asImg<icl64f>()); break;
          case depth64f: create(poSrc->asImg<icl64f>(),m_uiBorderSize,(*ppoDst)->asImg<icl64f>()); break;
        }
        break;
      default: ICL_INVALID_DEPTH;
    }
    
  }

  // }}}

  template<class T,class  I>
  inline void create_integral_channel_no_border(const T *image,int w, int h, I *intImage){
    // {{{ open

    FUNCTION_LOG("");
    /* algorithm: 
    +++++..
    +++CA..
    +++BX..
    .......
    X = src(x) + A + B - C
    */
       
    const T *src = image;
    I *dst = intImage;
  
    // fist pixel
    *dst++ = clipped_cast<T,I>(*src++);
    
    // fist row
    for(const T *srcEnd=src+w-1;src<srcEnd;++src,++dst){
      *dst = clipped_cast<T,I>(*src) + *(dst-1);
    }
    
    // first column
    src = image+w;
    dst = intImage+w;
    for(const T *srcEnd=src+w*(h-1);src<srcEnd;src+=w,dst+=w){
      *dst =  clipped_cast<T,I>(*src) + *(dst-w);
    }

    // rest of the image up to last row
    src = image+w+1;
    dst = intImage+w+1;
 
    for(int y=1;y<h;++y){
      int idx = y*w+1;
      for(int x=1;x<w;++x,++idx){
        dst[idx] =  clipped_cast<T,I>(src[idx]) + dst[idx-1] + dst[idx-w] - dst[idx-w-1];
      }
    } 
  }

  // }}}

  template<class I>
  void fill_integral_image_borders(I *intImage,int w, int h, int iw, int ih, int border){
    // {{{ open

    /*******************************
    left and top: set to zero
    right and bottom: "copy replicate border"
    <----------------->
    00000000000
    00000000000
    00xxxxxxAAA^
    00xxxxxxBBB|
    00xxxxxxFFF|
    00abcdqrGGGV
    00abcdprGGG
    00abcdprGGG
    ********************************/
 
    //top
    intImage-=(border+iw*border);
    memset(intImage,0,border*iw*sizeof(I));
 

    // left and right
    int bw = border+w;
    int bw1 =bw-1;
    int bh = border+h;
    I bufVal;
    for(int y=border;y<bh;++y){
      //left
      for(int x=0;x<border;++x){
        intImage[x+iw*y] = 0;
      }
      bufVal = intImage[y*iw+bw1];
      //right
      for(int x=bw;x<iw;++x){
        intImage[x+iw*y] = bufVal;
      }
    }
    
    // bottom
    I *srcLine = intImage+(bh-1)*iw;
    for(int y=bh;y<ih;++y){
      memcpy(intImage+y*iw,srcLine,iw*sizeof(I));
    }
  }
  // }}}
             
  template<class T,class  I>
  void create_integral_channel_with_border(const T *image,int w, int h, I *intImage, int border){
    // {{{ open

    FUNCTION_LOG("");
    /* algorithm: 
    +++++..
    +++CA..
    +++BX..
    .......
    X = src(x) + A + B - C
    */

    // integral image size
    const int iw = w+2*border;
    const int ih = h+2*border;

    const int offs = border+iw*border;
    //move to the first integral-image pixel
    intImage+=offs;
       
    const T *src = image;
    I *dst = intImage;
  
    // first pixel
    *dst++ =  clipped_cast<T,I>(*src++);
  
  
    // fist row
    for(const T *srcEnd=src+w-1;src<srcEnd;++src,++dst){
      *dst =  clipped_cast<T,I>(*src) + *(dst-1);
    }
  
    
    // first column
    src = image+w;
    dst = intImage+iw;
    for(const T *srcEnd=src+w*(h-1);src<srcEnd;src+=w,dst+=iw){
      *dst =  clipped_cast<T,I>(*src) + *(dst-iw);
    }

    
    // rest of the image
    src = image;
    dst = intImage;


    int idx;
    for(int y=1;y<h;++y){
      idx = y*iw+1;
      for(int x=1;x<w;++x,++idx){
        dst[idx] =  clipped_cast<T,I>(src[x+y*w]) + dst[idx-1] + dst[idx-iw] - dst[idx-iw-1];
      }
    } 
    
    fill_integral_image_borders(intImage,w,h,iw,ih,border);
  }

  // }}}

#ifdef HAVE_IPP
  template<>
  void create_integral_channel_with_border<icl8u,Ipp32s>(const icl8u *image, int w, int h, Ipp32s *intImage, int border){
    // {{{ open

    FUNCTION_LOG("");
    
    // integral image size
    int iw = w+2*border;
    int ih = h+2*border;
    int b1 = border-1;
    ippiIntegral_8u32s_C1R(image,w*sizeof(icl8u),intImage+b1+b1*iw,iw*sizeof(Ipp32s), Size(w,h),0);
    
    intImage+=(border+iw*border);
    fill_integral_image_borders(intImage,w,h,iw,ih,border);
  }

  // }}}

  template<>
  void create_integral_channel_with_border<icl8u,Ipp32f>(const icl8u *image, int w, int h, Ipp32f *intImage, int border){
    // {{{ open

    FUNCTION_LOG("");
    
    // integral image size
    int iw = w+2*border;
    int ih = h+2*border;
    int b1 = border-1;
    ippiIntegral_8u32f_C1R(image,w*sizeof(icl8u),intImage+b1+b1*iw,iw*sizeof(Ipp32f), Size(w,h),0);
    
    intImage+=(border+iw*border);
    fill_integral_image_borders(intImage,w,h,iw,ih,border);
  }

  // }}}
#endif
  
  template<class T, class I>
  Img<I> *IntegralImgOp::create(const Img<T> *image,unsigned int border, Img<I> *intImage){
    // {{{ open

    ICLASSERT_RETURN_VAL(image, intImage);
    ICLASSERT_RETURN_VAL(image->getDim(),intImage);
    ICLASSERT_RETURN_VAL(image->getChannels(),intImage);
    
    int w = image->getWidth();
    int h = image->getHeight();
    Size s(w+2*border,h+2*border);
    int c = image->getChannels();
    
    if(!intImage){
      intImage = new Img<I>(s,c);
    }else{
      intImage->setSize(s);
      intImage->setChannels(c);
    }
    if(!border){
      for(int i=0;i<c;i++){
        create_integral_channel_no_border<T,I>(image->getData(i),w,h,intImage->getData(i));
      }    
    }else{
      for(int i=0;i<c;i++){
        create_integral_channel_with_border<T,I>(image->getData(i),w,h,intImage->getData(i),border);
      }    
    }
    return intImage;
  }

  // }}}
    
}
