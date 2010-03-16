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
#include <ICLFilter/IntegralImgOp.h>

namespace icl{

  LocalThresholdOp::LocalThresholdOp(unsigned int maskSize, float globalThreshold, float gammaSlope): 
    // {{{ open
    m_maskSize(maskSize),m_globalThreshold(globalThreshold),
    m_gammaSlope(gammaSlope),m_roiBufSrc(0), m_roiBufDst(0),
    m_iiOp(new IntegralImgOp),m_algorithm(regionMean){
  }

  // }}}

  LocalThresholdOp::LocalThresholdOp(LocalThresholdOp::algorithm a, int maskSize, float globalThreshold, float gammaSlope):
    // {{{ open
    m_maskSize(maskSize),m_globalThreshold(globalThreshold),
    m_gammaSlope(gammaSlope),m_roiBufSrc(0), m_roiBufDst(0),
    m_iiOp(new IntegralImgOp),m_algorithm(a){
  }
    // }}}


  LocalThresholdOp::~LocalThresholdOp(){
    // {{{ open

    ICL_DELETE(m_roiBufDst);
    ICL_DELETE(m_iiOp);
  }

  // }}}

  void LocalThresholdOp::setMaskSize(unsigned int maskSize){
    // {{{ open

    m_maskSize = maskSize;    
  }

  // }}}

  void LocalThresholdOp::setGlobalThreshold(float globalThreshold){
    // {{{ open

    m_globalThreshold = globalThreshold;
  }

  // }}}

  void LocalThresholdOp::setGammaSlope(float gammaSlope){
    // {{{ open

    this->m_gammaSlope = gammaSlope;
  }

  // }}}

  unsigned int LocalThresholdOp::getMaskSize() const{
    // {{{ open

    return m_maskSize;
  }

  // }}}

  float LocalThresholdOp::getGlobalThreshold() const{
    // {{{ open

      return m_globalThreshold;
  }

  // }}}

  float LocalThresholdOp::getGammaSlope() const{
    // {{{ open

      return m_gammaSlope;
  }

  // }}}

  void LocalThresholdOp::setup(unsigned int maskSize, float globalThreshold, LocalThresholdOp::algorithm a, float gammaSlope){
    // {{{ open

    setMaskSize(maskSize);
    setGlobalThreshold(globalThreshold);
    setGammaSlope(gammaSlope);
    setAlgorithm(a);
  }

  // }}}

  /// returns currently used algorithm type
  LocalThresholdOp::algorithm LocalThresholdOp::getAlgorithms() const{
    // {{{ open

    return m_algorithm;
  }

  // }}}
  
  /// sets internally used algorithm
  void LocalThresholdOp::setAlgorithm(algorithm a){
    // {{{ open

    m_algorithm = a;
  }

  // }}}


  template<class TS,  class TI, class TD, class TT, bool WITH_GAMMA>
  static void fast_lt(const Img<TS> &src, const Img<TI> &iim, Img<TD> &dst, int r, TT t, float gs, int channel){
    // {{{ open

    const TI *ii = iim.begin(channel);
    
    const TS *psrc = src.begin(channel);
    TD *pdst = dst.begin(channel);
    
    // first, we leave out the borders:
    const int w = src.getWidth();
    const int h = src.getHeight();
    const int r2 = 2*r;
    const int yEnd = h-r;
    const int dim = r2*r2;
    t*=dim; // help to avoid /dim in the loop
    
    /* Explanation
        B-----C
        |     |
        |  x<-|--- here we are mean value in rect is B - C - D + A
        |     |
        D-----A

        Image parts for all border Regions (id = 1..8) we have to 
        work with a pixel dependend rectangle dimension
        
       1|       2       |3
       -------------------
        |               | 
       4|    CENTER     |5
        |               |
       -------------------
       6|       7       |8
     */
  
#define GET_II(x,y) ii[(x)+(y)*w]
#define GET_A(rx,ry,rw,rh) GET_II((rx+rw),(ry+rh))
#define GET_B(rx,ry,rw,rh) GET_II((rx),(ry))
#define GET_C(rx,ry,rw,rh) GET_II((rx+rw),(ry))
#define GET_D(rx,ry,rw,rh) GET_II((rx),(ry+rh))

#define GET_RECT(rx,ry,rw,rh) (GET_B((rx),(ry),(rw),(rh)) - GET_C((rx),(ry),(rw),(rh)) - GET_D((rx),(ry),(rw),(rh)) + GET_A((rx),(ry),(rw),(rh)) + t)
#define COMPLEX_STEP(rx,ry,rw,rh) pdst[x+w*y] = (!WITH_GAMMA) ?         \
    (255 * (psrc[x+w*y]*((rw)*(rh)) > (GET_RECT((rx),(ry),(rw),(rh)))) ) : \
    ((TD)clip<float>( gs * (psrc[x+w*y] - float(GET_RECT((rx),(ry),(rw),(rh)))/((rw)*(rh)) ) + 128,float(0),float(255)))


    // [1][2][3]
    for(int y=0;y<r;++y){
      for(int x=0;x<r;++x){    //[1]
        COMPLEX_STEP(0,0,r+x,r+y);
      }
      for(int x=r;x<w-r;++x){ //[2]
        COMPLEX_STEP(x-r,0,r2,r+y);
      }
      for(int x=w-r;x<w;++x){ //[3]
        COMPLEX_STEP(x-r,0,w+r-x-1,y+r);
      }
    }
    
    
    // [4][CENTER][5]
    for(int y=r; y<yEnd; ++y){
      //[4]
      for(int x=0;x<r;++x){
        COMPLEX_STEP(0,y-r,x+r,r2);
      }
      
      // [CENTER]
      const TI *B = ii+(y-r)*w;
      const TI *C = B + r2;
      const TI *D = B + r2*w;
      const TI *A = D + r2;
      const TS *s = psrc+y*w + r;
      TD *d = pdst+y*w + r;
      const TS *ends = s+w-r2;
      
#define STEP *d = (!WITH_GAMMA) ? \
      (255 * ( (*s*dim) > (*B - *C - *D + *A + t))) : \
      ((TD)clip<float>( gs * (*s - float(*B - *C - *D + *A + t)/dim ) + 128,float(0),float(255))) \
      ;  ++B; ++C; ++D; ++A; ++s; ++d;
      
      // 16x loop unrolling here
      for(int n = ((int)(ends-s)) >> 4; n > 0; --n){
        STEP STEP STEP STEP STEP STEP STEP STEP
        STEP STEP STEP STEP STEP STEP STEP STEP  
        }
      
      while(s<ends){
        STEP
        }
#undef STEP
      //[5]
      for(int x=w-r;x<w;++x){
        COMPLEX_STEP(x-r,y-r,w+r-x-1,r2);
      }
    }
    
    // [6][7][8]
    for(int y=h-r;y<h;++y){
      for(int x=0;x<r;++x){    //[6]
        COMPLEX_STEP(0,y-r,r+x,h+r-y-1);
      }
      for(int x=r;x<w-r;++x){ //[7]
        COMPLEX_STEP(x-r,y-r,r2,h+r-y-1);
      }
      for(int x=w-r;x<w;++x){ //[8]
        COMPLEX_STEP(x-r,y-r,w+r-x-1,h+r-y-1);
      }
    }
#undef GET_II
#undef GET_A
#undef GET_B
#undef GET_C
#undef GET_D
#undef GET_RECT
#undef COMPLEX_STEP
  }

  // }}}

  /// this template resolves the destination images depths and if a gamma slope is set or not
  template<class S, class I>
  void apply_local_threshold_six(const Img<S> &src,const Img<I> &ii, ImgBase *dst, float t, int m, float gs){
    // {{{ open

    switch(dst->getDepth()){
      case depth8u:
        if(gs!=0.0f){
          for(int c=0;c<src.getChannels();++c){
            fast_lt<S,I,icl8u,int,true>(src,ii,*dst->asImg<icl8u>(),m,int(t),gs,c);
          }
        }else{
          for(int c=0;c<src.getChannels();++c){
            fast_lt<S,I,icl8u,int,false>(src,ii,*dst->asImg<icl8u>(),m,int(t),gs,c);
          }
        }
        break;
      case depth32f:
        if(gs!=0.0f){
          for(int c=0;c<src.getChannels();++c){
            fast_lt<S,I,icl32f,float,true>(src,ii,*dst->asImg<icl32f>(),m,t,gs,c);
          }
        }else{
          for(int c=0;c<src.getChannels();++c){
            fast_lt<S,I,icl32f,float,false>(src,ii,*dst->asImg<icl32f>(),m,t,gs,c);
          }
        }
        break;
      default:
        // this may not happen
        ICL_INVALID_DEPTH;
    }
  }

  // }}}

  /// this template resolves the integral image depths
  template<class S>
  void apply_local_threshold_sxx(const Img<S> &src,const ImgBase *ii, ImgBase *dst, float t,unsigned int m, float gs){
    // {{{ open

    switch(ii->getDepth()){
      case depth32s:
        apply_local_threshold_six(src,*ii->asImg<icl32s>(),dst,t,m,gs);
        break;
      case depth32f:
        apply_local_threshold_six(src,*ii->asImg<icl32f>(),dst,t,m,gs);
        break;
      case depth64f:
        apply_local_threshold_six(src,*ii->asImg<icl64f>(),dst,t,m,gs);
        break;
      default:
        // this may not happen
        ICL_INVALID_DEPTH;
    }
  }

  // }}}


  template<LocalThresholdOp::algorithm a>
  void LocalThresholdOp::apply_a(const ImgBase*, ImgBase**){
    // {{{ open
    throw ICLException("this algorithm is not yet implemented for the LocalThresholdOp class");
  }
  // }}}

  template<> void LocalThresholdOp::apply_a<LocalThresholdOp::tiledNN>(const ImgBase *src, ImgBase **dst){
    // {{{ open
    
  }
  // }}}

  template<> void LocalThresholdOp::apply_a<LocalThresholdOp::tiledLIN>(const ImgBase *src, ImgBase **dst){
    // {{{ open

  }
  // }}}


  template<> void LocalThresholdOp::apply_a<LocalThresholdOp::regionMean>(const ImgBase *src, ImgBase **dst){
    // {{{ open
    const ImgBase *srcOrig = src;
    bool roi = false;
    // cut the roi of src if set
    if(!(src->hasFullROI())){
      ensureCompatible(&m_roiBufSrc, src->getDepth(), src->getROISize(), src->getChannels(), src->getFormat());
      src->deepCopyROI(&m_roiBufSrc);
      src = m_roiBufSrc;
      roi = true;
    }
    ICLASSERT_RETURN(src->getWidth() > 2*(int)m_maskSize);
    ICLASSERT_RETURN(src->getHeight() > 2*(int)m_maskSize);
    
    
    // prepare the destination image
    depth dstDepth = m_gammaSlope ? depth32f : depth8u;
    depth iiDepth = (src->getDepth() == depth8u || src->getDepth() == depth16s) ? depth32s : src->getDepth();
    ImgBase **useDst = roi ? &m_roiBufDst : dst;
    if(!prepare(useDst, dstDepth, src->getSize(), formatMatrix, src->getChannels(), Rect::null)){
      ERROR_LOG("prepare failure [code 1]");
      return;
    }
    
    m_iiOp->setIntegralImageDepth(iiDepth);
    const ImgBase *ii = m_iiOp->apply(src);
    
    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_local_threshold_sxx<icl##D>(*src->asImg<icl##D>(), ii, *useDst, m_globalThreshold, m_maskSize, m_gammaSlope); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
    
    if(roi){
      if(!prepare(dst, srcOrig, (*useDst)->getDepth())){
        ERROR_LOG("prepare failure [code 2]");
        return;
      }
      (*useDst)->deepCopyROI(dst);
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

    switch(m_algorithm){
      case regionMean: apply_a<regionMean>(src,dst); break;
      case tiledNN: apply_a<tiledNN>(src,dst); break;
      case tiledLIN: apply_a<tiledLIN>(src,dst); break;
      default:
        throw ICLException(std::string(__FUNCTION__)+": invalid algorithm value");
    }
  }  

  // }}}

  
}
