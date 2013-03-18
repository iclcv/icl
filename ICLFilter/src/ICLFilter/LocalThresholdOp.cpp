/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/LocalThresholdOp.cpp           **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/BinaryCompareOp.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/StackTimer.h>
#include <ICLFilter/IntegralImgOp.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{
  
    LocalThresholdOp::LocalThresholdOp(unsigned int maskSize, float globalThreshold, float gammaSlope): 
      // {{{ open
      m_roiBufSrc(0), m_roiBufDst(0),
      m_iiOp(new IntegralImgOp),
      m_cmp(new BinaryCompareOp(BinaryCompareOp::gt)),
      m_tiledBuf1(0),m_tiledBuf2(0){
      
      /*
          - mask size (range:spinbox)
          - global threshold (range:slider -100000,100000)
          - gamma slope (range:slider(-10,10)
          - algorithm (menu, region mean, tiled lin, tiled NN)
          */
      addProperty("mask size","range:slider","[1,100]",str(maskSize));
      addProperty("global threshold","range:slider","[-100,100]",str(globalThreshold));
      addProperty("gamma slope","range:slider","[-10,10]",str(gammaSlope));
      addProperty("algorithm","menu","region mean,tiled linear,tiled NN","region mean");
    }
  
    // }}}
  
    LocalThresholdOp::LocalThresholdOp(LocalThresholdOp::algorithm a, int maskSize, float globalThreshold, float gammaSlope):
      // {{{ open
      m_roiBufSrc(0), m_roiBufDst(0),
      m_iiOp(new IntegralImgOp),
      m_cmp(new BinaryCompareOp(BinaryCompareOp::gt)),
      m_tiledBuf1(0),m_tiledBuf2(0){
  
      addProperty("mask size","range:slider","[1,100]",str(maskSize));
      addProperty("global threshold","range:slider","[-100,100]",str(globalThreshold));
      addProperty("gamma slope","range:slider","[-10,10]",str(gammaSlope));
      addProperty("algorithm","menu","region mean,tiled linear,tiled NN",a==regionMean?"region mean":a==tiledNN?"tiled NN":"tiled linear");
    }
      // }}}
  
  
    LocalThresholdOp::~LocalThresholdOp(){
      // {{{ open
  
      ICL_DELETE(m_roiBufDst);
      ICL_DELETE(m_iiOp);
      ICL_DELETE(m_cmp);
      ICL_DELETE(m_tiledBuf1);
      ICL_DELETE(m_tiledBuf2);
    }
  
    // }}}
  
    void LocalThresholdOp::setMaskSize(unsigned int maskSize){
      // {{{ open
      prop("mask size").value = str(maskSize);
      call_callbacks("mask size");
    }
  
    // }}}
  
    void LocalThresholdOp::setGlobalThreshold(float globalThreshold){
      // {{{ open
      prop("global threshold").value = str(globalThreshold);
      call_callbacks("global threshold");
    }
  
    // }}}
  
    void LocalThresholdOp::setGammaSlope(float gammaSlope){
      // {{{ open
      prop("gamma slope").value = str(gammaSlope);
      call_callbacks("gamma slope");
    }
  
    // }}}
  
    unsigned int LocalThresholdOp::getMaskSize() const{
      // {{{ open
      return parse<int>(prop("mask size").value);
    }
  
    // }}}
  
    float LocalThresholdOp::getGlobalThreshold() const{
      // {{{ open
      return parse<float>(prop("global threshold").value);
    }
  
    // }}}
  
    float LocalThresholdOp::getGammaSlope() const{
      // {{{ open
      return parse<float>(prop("gamma slope").value);
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
    LocalThresholdOp::algorithm LocalThresholdOp::getAlgorithm() const{
      // {{{ open
      const std::string &a = prop("algorithm").value;
      return a == "region mean" ? regionMean : a =="tiled NN" ? tiledNN : tiledLIN;
    }
  
    // }}}
    
    /// sets internally used algorithm
    void LocalThresholdOp::setAlgorithm(algorithm a){
      // {{{ open
      prop("algorithm").value = (a==regionMean?"region mean":a==tiledNN?"tiledNN":"tiled linear");
      call_callbacks("algorithm");
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
  
  #if 1
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
        case depth16s:
          if(gs!=0.0f){
            for(int c=0;c<src.getChannels();++c){
              fast_lt<S,I,icl16s,int,true>(src,ii,*dst->asImg<icl16s>(),m,int(t),gs,c);
            }
          }else{
            for(int c=0;c<src.getChannels();++c){
              fast_lt<S,I,icl16s,int,false>(src,ii,*dst->asImg<icl16s>(),m,int(t),gs,c);
            }
          }
          break;
        case depth32s:
          if(gs!=0.0f){
            for(int c=0;c<src.getChannels();++c){
              fast_lt<S,I,icl32s,int,true>(src,ii,*dst->asImg<icl32s>(),m,int(t),gs,c);
            }
          }else{
            for(int c=0;c<src.getChannels();++c){
              fast_lt<S,I,icl32s,int,false>(src,ii,*dst->asImg<icl32s>(),m,int(t),gs,c);
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
        case depth64f:
          if(gs!=0.0f){
            for(int c=0;c<src.getChannels();++c){
              fast_lt<S,I,icl64f,float,true>(src,ii,*dst->asImg<icl64f>(),m,t,gs,c);
            }
          }else{
            for(int c=0;c<src.getChannels();++c){
              fast_lt<S,I,icl64f,float,false>(src,ii,*dst->asImg<icl64f>(),m,t,gs,c);
            }
          }
          break;
        default:
          // this may not happen
          ICL_INVALID_DEPTH;
      }
  
  #else
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
  #endif
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
  
  
    template<class T, class B>
    inline T roi_mean_gen(const Channel<T> &s, int dim, const Rect &roi){
      B buf = 0;
      for(int y=roi.y; y< roi.bottom();++y){
        for(int x=roi.x; x< roi.right();++x){
          buf += s(x,y);
        }
      }
      return buf/dim;
    }
    template<class T>
    inline T roi_mean(const Channel<T> &s, int dim, const Rect &roi){
      return roi_mean_gen<T,icl64f>(s,dim,roi);
    }
    template<> inline icl8u roi_mean(const Channel<icl8u> &s, int dim, const Rect &roi){
      return roi_mean_gen<icl8u,int64_t>(s,dim,roi);
    }
    template<> inline icl16s roi_mean(const Channel<icl16s> &s, int dim, const Rect &roi){
      return roi_mean_gen<icl16s,int64_t>(s,dim,roi);
    }
    template<> inline icl32s roi_mean(const Channel<icl32s> &s, int dim, const Rect &roi){
      return roi_mean_gen<icl32s,int64_t>(s,dim,roi);
    }
  
    template<class S>
    static void apply_tiled_thresh(const Img<S> &s, Img8u &dst, 
                                   Img<S> &buf1, Img<S> &buf2, int ts, int threshold, BinaryCompareOp *cmp, bool lin){
      
      int w = s.getWidth();
      int bw = w/ts;
      int h = s.getHeight();
      Size t(ts,ts);
      int dim = ts*ts;
      
      int NX = w/ts;
      int NY = h/ts;
      Rect r(0,0,ts,ts);
      
      for(int c=s.getChannels()-1;c>=0;--c){
        S *pbuf1 = buf1.begin(c);
        const Channel<S> chan = s[c];
        for(int y=0;y<NY;++y){
          r.y = ts*y;
          for(int x=0;x<NX;++x){
            r.x = ts*x;
            pbuf1[x+bw*y] = roi_mean(chan,dim,r)+threshold;
          }
        }
      }
      buf1.scaledCopy(&buf2,lin?interpolateLIN:interpolateNN);
      cmp->apply(&s,&buf2,bpp(dst));
    }
  
    template<> void LocalThresholdOp::apply_a<LocalThresholdOp::tiledNN>(const ImgBase *src, ImgBase **dst){
      // {{{ open
      
      int ts = 2*getMaskSize();
      
      
      ICLASSERT_RETURN(ts>1);
      Size size = src->getSize();
      ensureCompatible(&m_tiledBuf1,src->getDepth(),size/ts, 1, formatMatrix);
      ensureCompatible(&m_tiledBuf2,src->getDepth(),size,1,formatMatrix);
  
      switch(src->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                           \
        case depth##D:                                       \
           apply_tiled_thresh(*src->asImg<icl##D>(),         \
                              *(*dst)->asImg<icl8u>(),       \
                              *m_tiledBuf1->asImg<icl##D>(), \
                              *m_tiledBuf2->asImg<icl##D>(), \
                              ts, getGlobalThreshold(), m_cmp,  \
                              getAlgorithm() == tiledLIN);  \
        break;
        ICL_INSTANTIATE_ALL_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH
      }
    }
    // }}}
  
    template<> void LocalThresholdOp::apply_a<LocalThresholdOp::tiledLIN>(const ImgBase *src, ImgBase **dst){
      // {{{ open
      apply_a<tiledNN>(src,dst); // LIN vs NN is handled by a runtime-bool
    }
    // }}}
  
  
    template<> void LocalThresholdOp::apply_a<LocalThresholdOp::regionMean>(const ImgBase *src, ImgBase **dst){
      // {{{ open
    
      m_iiOp->setIntegralImageDepth((src->getDepth() == depth8u || src->getDepth() == depth16s) ? depth32s : src->getDepth());
      const ImgBase *ii = m_iiOp->apply(src);
      
      float t = getGlobalThreshold();
      int s = getMaskSize();
      float gs = getGammaSlope();
      
      switch(src->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D) case depth##D: apply_local_threshold_sxx<icl##D>(*src->asImg<icl##D>(), ii, *dst, t, s, gs); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
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
  
      const ImgBase *srcOrig = src;
      bool roi = false;
      // cut the roi of src if set
      if(!(src->hasFullROI())){
        ensureCompatible(&m_roiBufSrc, src->getDepth(), src->getROISize(), src->getChannels(), src->getFormat());
        src->deepCopyROI(&m_roiBufSrc);
        src = m_roiBufSrc;
        roi = true;
      }
      ICLASSERT_RETURN(src->getWidth() > 2*(int)getMaskSize());
      ICLASSERT_RETURN(src->getHeight() > 2*(int)getMaskSize());
  
      // prepare the destination image
      depth dstDepth = getAlgorithm() == regionMean ? (getGammaSlope() ? depth32f : depth8u) : depth8u;
      ImgBase **useDst = roi ? &m_roiBufDst : dst;
      if(!prepare(useDst, dstDepth, src->getSize(), formatMatrix, src->getChannels(), Rect::null)){
        ERROR_LOG("prepare failure [code 1]");
        return;
      }
  
      switch(getAlgorithm()){
        case regionMean: apply_a<regionMean>(src,useDst); break;
        case tiledNN: apply_a<tiledNN>(src,useDst); break;
        case tiledLIN: apply_a<tiledLIN>(src,useDst); break;
        default:
          throw ICLException(std::string(__FUNCTION__)+": invalid algorithm value");
      }
  
      if(roi){
        if(!prepare(dst, srcOrig, (*useDst)->getDepth())){
          ERROR_LOG("prepare failure [code 2]");
          return;
        }
        (*useDst)->deepCopyROI(dst);
      }
      (*dst)->setTime(src->getTime());
    }  
  
    // }}}
  
    
  } // namespace filter
}
