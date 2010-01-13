#include <ICLFilter/GradientImage.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLUtils/Mutex.h>
#include <ICLFilter/LUT2D.h>

namespace icl{
  
  namespace{
    void calculate_xy_gradient(const Img16s &src, Img16s &gx, Img16s &gy){
      // {{{ open

      gx.setParams(src.getParams());    
      gy.setParams(src.getParams());
      
      gx.setROI(src.getROI().enlarged(-1));
      gy.setROI(src.getROI().enlarged(-1));
      
      static ConvolutionOp convX(ConvolutionKernel(ConvolutionKernel::sobelX3x3));
      static ConvolutionOp convY(ConvolutionKernel(ConvolutionKernel::sobelY3x3));
      static Mutex convXYMutex;
      static bool first = true;
      convXYMutex.lock();

      if(first){
        convX.setCheckOnly(true);
        convX.setClipToROI(false);
        convY.setCheckOnly(true);
        convY.setClipToROI(false);
        first = false;
      }

      convX.apply(&src,bpp(gx));
      convY.apply(&src,bpp(gy));
            
      convXYMutex.unlock();
    }

    // }}}
    
    float intensity_func(icl16s a, icl16s b){
      // {{{ open
      return sqrt(a*a+b*b);
    }

    // }}}

    float angle_func(icl16s a, icl16s b){
      // {{{ open
      return atan2(b,a);
    }
    // }}}


    void apply_lut_func(bool roiOnly,const Img16s &X,const Img16s &Y, Img32f &R,const LUT2D<icl32f,icl16s> &LUT){
      // {{{ open
      
      for(int i=0;i<X.getChannels();++i){
        if(roiOnly){
          const ImgIterator<icl16s> itX = X.beginROI(i);
          const ImgIterator<icl16s> itXEnd = X.endROI(i);
          const ImgIterator<icl16s> itY = Y.beginROI(i);
          ImgIterator<icl32f> itR = R.beginROI(i);
          for(;itX != itXEnd ;++itX,++itY,++itR){
            *itR = LUT(*itX,*itY);
          }
        }else{
          const icl16s *gx = X.getData(i);
          const icl16s *gy = Y.getData(i);
          float *gr = R.getData(i);
          for(int i=X.getDim()-1;i>=0;--i){
            gr[i] = LUT(gx[i],gy[i]);
          }
        }
      }
    }

    // }}}
  }
  
  void GradientImage::update(const ImgBase *src, GradientImage::calculationMode mode){
    /// ensure input image is of depht depht8u

    const Img16s *useSrc = 0;
    if(src->getDepth() == depth16s){
      useSrc = src->asImg<icl16s>();
    }else{
      src->convert(&m_oBuf);
      useSrc = &m_oBuf;
    }
    calculate_xy_gradient(*useSrc,m_oX,m_oY);
    
    m_oX.setFullROI();    
    m_oY.setFullROI();
    
    if(mode & calculateIntensity){
      static LUT2D<float,icl16s> I_LUT(intensity_func,-1020,1020);
      m_oI.setSize(useSrc->getSize());
      m_oI.setChannels(useSrc->getChannels());
      m_oI.setROI(useSrc->getROI());
      apply_lut_func(!useSrc->hasFullROI(),m_oX,m_oY,m_oI,I_LUT);
    }
    if(mode & calculateAngle){
      static LUT2D<float,icl16s> A_LUT(angle_func,-1020,1020);
      m_oA.setSize(useSrc->getSize());
      m_oA.setChannels(useSrc->getChannels());
      m_oA.setROI(useSrc->getROI());
      apply_lut_func(!useSrc->hasFullROI(),m_oX,m_oY,m_oA,A_LUT);
    }
  }
  void GradientImage::normalize(){
    m_oI.normalizeAllChannels(Range<icl32f>(0,255));
    m_oA.normalizeAllChannels(Range<icl32f>(0,255));
  }
}


