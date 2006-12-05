#include <IntegralImg.h>

namespace icl{

  template<class T,class  I>
  inline void create_integral_channel_no_border(T *image,int w, int h, I *intImage){
    // {{{ open

    FUNCTION_LOG("");
    /* algorithm: 
    +++++..
    +++CA..
    +++BX..
    .......
    X = src(x) + A + B - C
    */
       
    T *src = image;
    I *dst = intImage;
  
    // fist pixel
    *dst++ = *src++;
    
    // fist row
    for(T *srcEnd=src+w-1;src<srcEnd;++src,++dst){
      *dst = *src + *(dst-1);
    }
    
    // first column
    src = image+w;
    dst = intImage+w;
    for(T *srcEnd=src+w*(h-1);src<srcEnd;src+=w,dst+=w){
      *dst = *src + *(dst-w);
    }

    // rest of the image
    src = image+w+1;
    dst = intImage+w+1;
    int idx;
    for(int y=1;y<h;++y){
      idx = y*w+1;
      for(int x=1;x<w;++x,++idx){
        dst[idx] = src[idx] + dst[idx-1] + dst[idx-w] - dst[idx-w-1];
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
  void create_integral_channel_with_border(T *image,int w, int h, I *intImage, int border){
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
    int iw = w+2*border;
    int ih = h+2*border;

    //move to the first integral-image pixel
    intImage+=(border+iw*border);
       
    T *src = image;
    I *dst = intImage;
  
    // first pixel
    *dst++ = *src++;
  
  
    // fist row
    for(T *srcEnd=src+w-1;src<srcEnd;++src,++dst){
      *dst = *src + *(dst-1);
    }
  
    
    // first column
    src = image+w;
    dst = intImage+iw;
    for(T *srcEnd=src+w*(h-1);src<srcEnd;src+=w,dst+=iw){
      *dst = *src + *(dst-iw);
    }

    
    // rest of the image
    src = image;
    dst = intImage;
    int idx;
    for(int y=1;y<h;++y){
      idx = y*iw+1;
      for(int x=1;x<w;++x,++idx){
        dst[idx] = src[x+y*w] + dst[idx-1] + dst[idx-iw] - dst[idx-iw-1];
      }
    } 
 
    fill_integral_image_borders(intImage,w,h,iw,ih,border);
  }

  // }}}


#ifdef WITH_IPP_OPTIMIZATION
  template<>
  void create_integral_channel_with_border<icl8u,Ipp32s>(icl8u *image, int w, int h, Ipp32s *intImage, int border){
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
  void create_integral_channel_with_border<icl8u,Ipp32f>(icl8u *image, int w, int h, Ipp32f *intImage, int border){
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
  
  template<class T,class  I>
  std::vector<I*> IntegralImg::create(Img<T> *image, std::vector<I*> &dst, unsigned int border){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(image, dst);
    ICLASSERT_RETURN_VAL(image->getDim(),dst);
    ICLASSERT_RETURN_VAL(image->getChannels(),dst);
    ICLASSERT_RETURN_VAL(image->getChannels() == (int)dst.size() || !dst.size(), dst);
    
    if(!dst.size()){
      dst.resize(image->getChannels());
      for(unsigned int i=0;i<dst.size();i++){
        dst[i]=0;
      }
    }
    if(!border){
      for(unsigned int i=0;i<dst.size();i++){
        if(!dst[i]){
          dst[i] = new I[image->getDim()];
        }
        create_integral_channel_no_border<T,I>(image->getData(i),image->getWidth(),image->getHeight(),dst[i]);
      }    
    }else{
      for(unsigned int i=0;i<dst.size();i++){
        if(!dst[i]){
          dst[i] = new I[(image->getWidth()+2*border)*(image->getHeight()+2*border)];
        }
        create_integral_channel_with_border<T,I>(image->getData(i),image->getWidth(),image->getHeight(),dst[i],border);
      }        
    }
    return dst;
  } 

  // }}}

  template<class T,class  I>
  std::vector<I*> IntegralImg::create(Img<T> *image,unsigned int border){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(image, std::vector<I*>(0));
    ICLASSERT_RETURN_VAL(image->getDim(),std::vector<I*>(0));
    ICLASSERT_RETURN_VAL(image->getChannels(),std::vector<I*>(0));
    
    std::vector<I*> dst(image->getChannels());
    for(unsigned int i=0;i<dst.size();i++){
     
      if(border){
        dst[i] = new I[(image->getWidth()+2*border)*(image->getHeight()+2*border)];
        create_integral_channel_with_border<T,I>(image->getData(i),image->getWidth(),image->getHeight(),dst[i],border);
      }else{
        dst[i] = new I[image->getDim()];
        create_integral_channel_no_border<T,I>(image->getData(i),image->getWidth(),image->getHeight(),dst[i]);
      }
    }
    return dst;
  }   

  // }}}
  
  // {{{ explicit template instantiations

  template std::vector<int*> IntegralImg::create<icl8u,int>(Img<icl8u>*, std::vector<int*>&,unsigned int);
  template std::vector<icl32f*> IntegralImg::create<icl32f,icl32f>(Img<icl32f>*, std::vector<icl32f*>&,unsigned int);
  template std::vector<icl32f*> IntegralImg::create<icl8u,icl32f>(Img<icl8u>*, std::vector<icl32f*>&,unsigned int);

  template std::vector<int*> IntegralImg::create<icl8u,int>(Img<icl8u>*,unsigned int);
  template std::vector<icl32f*> IntegralImg::create<icl32f,icl32f>(Img<icl32f>*,unsigned int);
  template std::vector<icl32f*> IntegralImg::create<icl8u,icl32f>(Img<icl8u>*,unsigned int);

  // }}}
}
