#include <IntegralImg.h>

namespace icl{

  template<class T,class  I>
  inline void create_integral_channel(T *image,int w, int h, I *intImage){
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
  template<class T,class  I>
  std::vector<I*> IntegralImg::create(Img<T> *image, std::vector<I*> &dst){
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
    for(unsigned int i=0;i<dst.size();i++){
      if(!dst[i]){
        dst[i] = new I[image->getDim()];
      }
      create_integral_channel<T,I>(image->getData(i),image->getWidth(),image->getHeight(),dst[i]);
    }    
    return dst;
  } 

  template<class T,class  I>
  std::vector<I*> IntegralImg::create(Img<T> *image){
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(image, std::vector<I*>(0));
    ICLASSERT_RETURN_VAL(image->getDim(),std::vector<I*>(0));
    ICLASSERT_RETURN_VAL(image->getChannels(),std::vector<I*>(0));
    
    std::vector<I*> dst(image->getChannels());
    for(unsigned int i=0;i<dst.size();i++){
      dst[i] = new I[image->getDim()];
      create_integral_channel<T,I>(image->getData(i),image->getWidth(),image->getHeight(),dst[i]);
    }
    return dst;
  }   
  
  template std::vector<int*> IntegralImg::create<icl8u,int>(Img<icl8u>*, std::vector<int*>&);
  template std::vector<icl32f*> IntegralImg::create<icl32f,icl32f>(Img<icl32f>*, std::vector<icl32f*>&);
  template std::vector<icl32f*> IntegralImg::create<icl8u,icl32f>(Img<icl8u>*, std::vector<icl32f*>&);

  template std::vector<int*> IntegralImg::create<icl8u,int>(Img<icl8u>*);
  template std::vector<icl32f*> IntegralImg::create<icl32f,icl32f>(Img<icl32f>*);
  template std::vector<icl32f*> IntegralImg::create<icl8u,icl32f>(Img<icl8u>*);
}
