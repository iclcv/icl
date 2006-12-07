#include "IntegralImg.h"

namespace icl{

  // integral image - inner class IntImg functions
  template<class T>
  IntegralImg::IntImg<T>::IntImg(){}

  template<class T>
  IntegralImg::IntImg<T>::IntImg(const Size &s, int channels):
    m_oSize(s), m_oData(channels){
    for(int i=0;getChannels();++i){
      m_oData[i]=m_oSize ? new T[m_oSize.getDim()] : 0;
    }      
  }
  template<class T>
  IntegralImg::IntImg<T>::~IntImg(){
    if(!m_oSize) return;
    for(unsigned int i=0;i<m_oData.size();i++){
      delete [] m_oData[i];
    }
  }

  template<class T>
  T *IntegralImg::IntImg<T>::getData(int channel){
    ICLASSERT_RETURN_VAL(channel<getChannels(), 0);
    return m_oData[channel];
  }

  template<class T>
  const T *IntegralImg::IntImg<T>::getData(int channel) const{
    ICLASSERT_RETURN_VAL(channel<getChannels(), 0);
    return m_oData[channel];
  }

  template<class T>
  const Size &IntegralImg::IntImg<T>::getSize() const{
    return m_oSize;
  }

  template<class T>
  void IntegralImg::IntImg<T>::setSize(const Size &size){
    if(m_oSize != size){
      for(int i=0;i<getChannels();++i){
        delete m_oData[i];
        m_oData[i] = new T[size.getDim()];
      }
      m_oSize = size;
    }
  }

  template<class T>
  void IntegralImg::IntImg<T>::setChannels(int c){
    int cc = getChannels();
    if(cc != c){
      std::vector<T*> newData;
      if(c<cc){ // remove channels
        for(int i=0;i<c;++i){
          newData.push_back(m_oData[i]);
        }
        for(int i=c;i<cc;++i){
          delete m_oData[i];
        }
        m_oData = newData;
      }else{ // add channels
        for(int i=cc;i<c;++i){
          m_oData.push_back(new T[getDim()]);
        }        
      }
    }
  }

  template<class T>
  int IntegralImg::IntImg<T>::getChannels() const{
    return (int)(m_oData.size());
  }
  template<class T>
  int IntegralImg::IntImg<T>::getDim() const{
    return m_oSize.getDim();
  }
  
  template class IntegralImg::IntImg<int>;
  template class IntegralImg::IntImg<icl32f>;

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

    // rest of the image up to last row
    src = image+w+1;
    dst = intImage+w+1;
 
    for(int y=1;y<h;++y){
      int idx = y*w+1;
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
    const int iw = w+2*border;
    const int ih = h+2*border;

    const int offs = border+iw*border;
    //move to the first integral-image pixel
    intImage+=offs;
       
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
  
  template<class T, class I>
  IntegralImg::IntImg<I> *IntegralImg::create(Img<T> *image,unsigned int border, IntImg<I> *intImage){
    // {{{ open

    ICLASSERT_RETURN_VAL(image, intImage);
    ICLASSERT_RETURN_VAL(image->getDim(),intImage);
    ICLASSERT_RETURN_VAL(image->getChannels(),intImage);
    
    int w = image->getWidth();
    int h = image->getHeight();
    Size s(w+2*border,h+2*border);
    int c = image->getChannels();
    
    if(!intImage){
      intImage = new IntImg<I>(s,c);
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
  
  
  // {{{ explicit template instantiations

  template IntegralImg::IntImg<int> *IntegralImg::create<icl8u,int>(Img8u *, unsigned int, IntegralImg::IntImg<int>*);
  template IntegralImg::IntImg<icl32f> *IntegralImg::create<icl8u,icl32f>(Img8u *, unsigned int, IntegralImg::IntImg<icl32f>*);
  template IntegralImg::IntImg<icl32f> *IntegralImg::create<icl32f,icl32f>(Img32f *, unsigned int, IntegralImg::IntImg<icl32f>*);
   
  // }}}
}
