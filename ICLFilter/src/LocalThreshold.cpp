#include "LocalThreshold.h"
#include "Size.h"
#include "Macros.h"

namespace icl{

  void create_roi_size_image(const Size &s, int r, IntegralImg::IntImg32s& dst){
    // {{{ open

    dst.setSize(s);
    dst.setChannels(1);
    int *p = dst.getData(0);
    
    int w = s.width;
    int h = s.height;
    int r1 = r+1;
    int rr1 = r+r+1;
    int dim = rr1*rr1;
    
    // corners:
    // top left / right
    for(int y=0;y<r;y++){ 
      for(int x=0;x<r;x++){
        p[x+w*y] = (r1+x)*(r1+y); //>>>>>>>> left
      }
      for(int x=w-r;x<w;x++){
        p[x+w*y] = (r1+(w-x-1))*(r1+y); //>> right
      }
      // center
      for(int x=r,xEnd=w-r;x<xEnd;++x){
        p[x+w*y] = rr1*(r1+y);
      }    
    }
    
    // bottom left / right
    for(int y=h-r;y<h;++y){ 
      for(int x=0;x<r;++x){
        p[x+w*y] = (r1+x)*(r1+(h-y-1)); //>>>>>>>> left
      }
      for(int x=w-r;x<w;x++){
        p[x+w*y] = (r1+(w-x-1))*(r1+(h-y-1)); //>> right
      }
      // center
      for(int x=r,xEnd=w-r;x<xEnd;++x){
        p[x+w*y] = rr1*(r1+(h-y-1));
      }  
    }
    
    // left and right
    for(int y=r,yEnd=h-r;y<yEnd;++y){ 
      for(int x=0;x<r;x++){
        p[x+w*y] = (r1+x)*rr1; //>>>>>>>> left
      }
      for(int x=w-r;x<w;++x){
        p[x+w*y] = (r1+(w-x-1))*rr1; //>> right
      }
    }
    for(int y=r,yEnd=h-r; y<yEnd;++y){
      for(int x=r,xEnd=w-r; x<xEnd; ++x){
        p[x+w*y]=dim;
      }
    }
  }

  // }}}
  
  
  LocalThreshold::LocalThreshold(unsigned int maskSize, int globalThreshold): 
    // {{{ open

    m_uiMaskSize(maskSize),m_iGlobalThreshold(globalThreshold),m_poROIImage(0)
  {
    //ImgBase *m_poROIImage;
    //IntegralImg::IntImg32s m_oROISizeImage;
    //IntegralImg::IntImg32s m_oIntegralImage;
    
  }

  // }}}

  void LocalThreshold::setMaskSize(unsigned int maskSize){
    // {{{ open

    m_uiMaskSize = maskSize;    
  }

  // }}}

  void LocalThreshold::setGlobalThreshold(int globalThreshold){
    // {{{ open

    m_iGlobalThreshold = globalThreshold;
  }

  // }}}

  template<class T,class T2>
  void local_threshold_algorithm(Img<T> *src, 
                                 Img<T>* dst, 
                                 IntegralImg::IntImg<T2> *integralImage,
                                 int *roiSizeImage,  
                                 const int globalThreshold){
    // {{{ open

    /*********************************************************
    ***  local Threshold algorithm  **************************
    **********************************************************/
    
    /* r=2
    .C....A...
    ..+++++...
    ..+++++...
    ..++X++...
    ..+++++...    
    .B++++D...
    ..........
    |+| = D - A - B + C 
    */
    
    Size s = src->getSize();
    const int w = s.width;
    const int h = s.height;
    const int r = 10;
    const int r1 = r+1;
    const int r_1 = -r-1;
       
    int yu, yl, xr, xl;
    T2 thresh;
    
    int iw =w+2*(r1);
    //    int ih =h+2*(r1);
   
    for(int channel=0;channel<src->getChannels();++channel){
      T *S = src->getData(channel);
      T *D = dst->getData(channel);
      
      T2 *I = integralImage->getData(channel)+(r1+r1*iw);
      
      for(int y=0;y<h;y++){
        yu = (y-r1)*iw;  // (y-(r+1))*iw
        yl = (y+r)*iw;   // (y+r)*iw 
        xr = r;          // r
        xl = r_1;        // -r-1
        for(int idx=w*y,idxEnd=w*(y+1); idx<idxEnd ; ++idx,++xr,++xl){
          thresh = (I[xr+yl] - (I[xr+yu] + I[xl+yl]) + I[xl+yu]) / roiSizeImage[idx]; 
          D[idx] = S[idx] < (thresh+globalThreshold) ? 0 : 255;
        }
      }
    }
  }

  // }}}

  void LocalThreshold::apply(ImgBase *src, ImgBase **dst){
    // {{{ open

    // cut the roi of src if set
    if(!(src->hasFullROI())){
      if(!m_poROIImage){
        m_poROIImage = src->deepCopyROI();
      }else if(src->getDepth() != m_poROIImage->getDepth()){
        delete m_poROIImage;
        m_poROIImage = src->deepCopyROI();
      }else{
        src->deepCopyROI(m_poROIImage);
      }
      src = m_poROIImage;
    }
    
    // prepare the destination image
    if(!prepare(dst,src)){
      ERROR_LOG("prepare failure in LocalThreshold! ??");
    }
    
    // prepare the roi size image
    create_roi_size_image(src->getSize(),m_uiMaskSize,m_oROISizeImage);
    
    // create the integral images with border 1+roiSize
    switch(src->getDepth()){
      case depth8u:
        IntegralImg::create(src->asImg<icl8u>(), m_uiMaskSize+1, &m_oIntegralImage);
        local_threshold_algorithm<icl8u>(src->asImg<icl8u>(),(*dst)->asImg<icl8u>(),&m_oIntegralImage,m_oROISizeImage.getData(0),m_iGlobalThreshold);
        break;
      case depth32f:
        IntegralImg::create(src->asImg<icl32f>(), m_uiMaskSize+1, &m_oIntegralImageF);
        local_threshold_algorithm<icl32f>(src->asImg<icl32f>(),(*dst)->asImg<icl32f>(),&m_oIntegralImageF,m_oROISizeImage.getData(0),m_iGlobalThreshold);
        break;
      default:
        ICL_INVALID_FORMAT;
    }
  }  

  // }}}
}
