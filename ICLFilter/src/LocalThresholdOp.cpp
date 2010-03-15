/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/LocalThresholdOp.cpp                     **
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
*********************************************************************/

#include <ICLFilter/LocalThresholdOp.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/StackTimer.h>
namespace icl{

  void create_roi_size_image(const Size &s, int r, Img32s& dst){
    // {{{ open
    dst.setSize(s);
    dst.setChannels(1);

    ICLASSERT_RETURN( s.width > 2*r && s.height > 2*r );
 
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
  
  
  LocalThresholdOp::LocalThresholdOp(unsigned int maskSize, int globalThreshold, float gammaSlope): 
    // {{{ open

    m_uiMaskSize(maskSize),m_iGlobalThreshold(globalThreshold),
    m_fGammaSlope(gammaSlope),m_poROIImage(0)
  {

    // prepare the roi size image
    //ImgBase *m_poROIImage;
    //IntegralImg::IntImg32s m_oROISizeImage;
    //IntegralImg::IntImg32s m_oIntegralImage;
    
  }

  // }}}

  void LocalThresholdOp::setMaskSize(unsigned int maskSize){
    // {{{ open

    m_uiMaskSize = maskSize;    
  }

  // }}}

  void LocalThresholdOp::setGlobalThreshold(int globalThreshold){
    // {{{ open

    m_iGlobalThreshold = globalThreshold;
  }

  // }}}

  void LocalThresholdOp::setGammaSlope(float gammaSlope){
    // {{{ open

    this->m_fGammaSlope = gammaSlope;
  }

  // }}}

  template<class T,class T2>
  void local_threshold_algorithm(const Img<T> *src, 
                                 Img<T>* dst, 
                                 Img<T2> *integralImage,
                                 int *roiSizeImage,  
                                 int globalThreshold,
                                 int r,
                                 float gammaSlope){
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
    const int r1 = r+1;
    const int r_1 = -r-1;
       
    int yu, yl, xr, xl;
    T2 thresh;
    
    int iw =w+2*(r1);
    //    int ih =h+2*(r1);
    for(int channel=0;channel<src->getChannels();++channel){
      const T *S = src->getData(channel);
      T *D = dst->getData(channel);
      T2 *I = integralImage->getData(channel)+(r1+r1*iw);
      if(gammaSlope){
        /**
         using function f(x) = m*x + b    (with clipping)
         with m = gammaSlope
              k = localThresh+globalThresh
              f(k) = 128
         
         f(x) = clip( m(x-k)+128 , 0 , 255 )
        */
        for(int y=0;y<h;y++){
          yu = (y-r1)*iw;  // (y-(r+1))*iw
          yl = (y+r)*iw;   // (y+r)*iw 
          xr = r;          // r
          xl = r_1;        // -r-1
          for(int idx=w*y,idxEnd=w*(y+1); idx<idxEnd ; ++idx,++xr,++xl){
            thresh = (I[xr+yl] - (I[xr+yu] + I[xl+yl]) + I[xl+yu]) / roiSizeImage[idx]; 
            D[idx] = (T2)clip( gammaSlope * (S[idx] - thresh+globalThreshold) + 128,float(0),float(255));
          }
        }
      }else{
        for(int y=0;y<h;y++){
          yu = (y-r1)*iw;  // (y-(r+1))*iw
          yl = (y+r)*iw;   // (y+r)*iw 
          xr = r;          // r
          xl = r_1;        // -r-1
          for(int idx=w*y,idxEnd=w*(y+1); idx<idxEnd ; ++idx,++xr,++xl){
            thresh = (I[xr+yl] - (I[xr+yu] + I[xl+yl]) + I[xl+yu]) / roiSizeImage[idx]; 
            D[idx] = 255 * (S[idx] > (thresh+globalThreshold));
          }
        }
      }
    }
  }

  // }}}

  void LocalThresholdOp::apply(const ImgBase *src, ImgBase **dst){
    // {{{ open
    ICLASSERT_RETURN( src );
    ICLASSERT_RETURN( src->getSize() != Size::null );
    ICLASSERT_RETURN( src->getChannels() );
    ICLASSERT_RETURN( dst );
    ICLASSERT_RETURN( src != *dst );

    // cut the roi of src if set
    if(!(src->hasFullROI())){
      src->deepCopy(&m_poROIImage);
      src = m_poROIImage;
    }
    // prepare the destination image
    if(!prepare(dst,src)){
      ERROR_LOG("prepare failure in LocalThreshold! ??");
    }
    
    // prepare the roi size image
    if(m_oROISizeImage.getSize().isNull() || 
       m_uiROISizeImagesMaskSize != m_uiMaskSize || 
       m_oROISizeImage.getSize() != src->getSize() ) {
      create_roi_size_image(src->getSize(),m_uiMaskSize,m_oROISizeImage);
      m_uiROISizeImagesMaskSize = m_uiMaskSize;
    }

    // create the integral images with border 1+roiSize
    switch(src->getDepth()){
      case depth8u:{
        //        IntegralImg::create(src->asImg<icl8u>(), m_uiMaskSize+1, &m_oIntegralImage);
        ImgBase *ii = &m_oIntegralImage;
        {
          BENCHMARK_THIS_SECTION("II");
        IntegralImgOp(m_uiMaskSize+1, depth32s).apply(src, &ii);
        }
        {
          BENCHMARK_THIS_SECTION("LT");
        local_threshold_algorithm<icl8u>(src->asImg<icl8u>(),
                                         (*dst)->asImg<icl8u>(),
                                         &m_oIntegralImage,
                                         m_oROISizeImage.getData(0),
                                         m_iGlobalThreshold,
                                         m_uiMaskSize,
                                         m_fGammaSlope);
        }
        break;
      }
      case depth32f:{
        //IntegralImg::create(src->asImg<icl32f>(), m_uiMaskSize+1, &m_oIntegralImageF);
        ImgBase *ii = &m_oIntegralImageF;
        IntegralImgOp(m_uiMaskSize+1, depth32f).apply(src, &ii);
        local_threshold_algorithm<icl32f>(src->asImg<icl32f>(),
                                          (*dst)->asImg<icl32f>(),
                                          &m_oIntegralImageF,
                                          m_oROISizeImage.getData(0),
                                          m_iGlobalThreshold,
                                          m_uiMaskSize,
                                          m_fGammaSlope);
        break;
      }
      default:
        ICL_INVALID_FORMAT;
    }
   
  }  

  // }}}

  unsigned int LocalThresholdOp::getMaskSize() const{
    return m_uiMaskSize;
  }
  int LocalThresholdOp::getGlobalThreshold() const{
      return m_iGlobalThreshold;
  }
  float LocalThresholdOp::getGammaSlope() const{
      return m_fGammaSlope;
  }
  
    
}
