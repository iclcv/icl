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
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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
#include <ICLUtils/Time.h>
#include <ICLFilter/LocalThresholdOpHelpers.h>

#include <stdio.h>

template<class T>
inline unsigned char myclip(T x){
  return x < 0.0 ? 0 : x > 255.0 ? 255 : x;
}

template<class T>
inline void ppm_write(const icl::core::Img<T> &image, const std::string &filename){
  FILE *fp = fopen(filename.c_str(), "wb"); /* b - binary mode */
  (void) fprintf(fp, "P6\n%d %d\n255\n", image.getWidth(),image.getHeight());
  for (int j = 0; j < image.getHeight(); ++j){
    for (int i = 0; i < image.getWidth(); ++i){
      static unsigned char color[3];
      if(image.getChannels() == 3){
        color[0] = myclip(image(i,j,0));
        color[1] = myclip(image(i,j,1));
        color[2] = myclip(image(i,j,2));
        (void) fwrite(color, 1, 3, fp);
      }else if(image.getChannels() == 1){
        color[0] = color[1] = color[2] = myclip(image(i,j,0));
        (void) fwrite(color, 1, 3, fp);
      }
    }
  }
  (void) fclose(fp);
}

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
      addProperty("mask size","range:slider","[1,100]:1",str(maskSize));
      addProperty("global threshold","range:slider","[-100,100]",str(globalThreshold));
      addProperty("gamma slope","range:slider","[-10,10]",str(gammaSlope));
      addProperty("algorithm","menu","region mean,tiled linear,tiled NN","region mean");
      addProperty("actually used mask size","info","","0");
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
      addProperty("actually used mask size","info","","0");
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
      call_callbacks("mask size",this);
    }
  
    // }}}
  
    void LocalThresholdOp::setGlobalThreshold(float globalThreshold){
      // {{{ open
      prop("global threshold").value = str(globalThreshold);
      call_callbacks("global threshold",this);
    }
  
    // }}}
  
    void LocalThresholdOp::setGammaSlope(float gammaSlope){
      // {{{ open
      prop("gamma slope").value = str(gammaSlope);
      call_callbacks("gamma slope",this);
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
      call_callbacks("algorithm",this);
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
  
    template <class S>
    static void roi_cmp(const Channel<S> &s, const Rect &r, S val, Channel8u &d, int threshold){
      const int maxy = r.bottom();
      for(int y=r.y;y<maxy;++y){
        const S *srow = &s(r.x,y);
        icl8u *drow = &d(r.x,y);
        for(int x=0;x<r.width;++x){
          drow[x] = 255 * (srow[x] > (val + threshold));
        }
      }
    }

    /// todo: fix-point approx for icl8u type
    template<class S>
    static void linear_interpolate(S a, S b, S *dst, int n){
      // fixed-point approx x 100
      float dy = ((float)b - float(a));
      float slope = dy / float(n-1);
      for(int i=0;i<n;++i){
        dst[i] = a + i * slope;
      }
    }

    /// todo: fix-point approx for icl8u type


    template<class S>
    static void linear_interpolate_cmp(S a, S b, int n, const S *src, icl8u *dst, int threshold, icl32f *cmp){
      // fixed-point approx x 100
      float dy = ((float)b - float(a));
      float slope = dy / float(n-1);
      float base = a + threshold;
      for(int i=0;i<n;++i){
        dst[i] = 255 * (src[i] > (base + i * slope));
        //  cmp[i] = base + i * slope;
      }
    }


    template<class S>
    static void compare_lin(S *cL,S *cR, int tsx, int tsy,
                            S ul, S ur, S ll, S lr, const Channel<S> &src,
                            Channel8u dst, const Rect &roi, int threshold,
                            Channel32f cmp){
      linear_interpolate(ul,ll,cL,tsy);
      linear_interpolate(ur,lr,cR,tsy);
      for(int y=0;y<tsy;++y){
        linear_interpolate_cmp<S>(cL[y], cR[y], tsx,
                                  &src(roi.x,roi.y+y), 
                                  &dst(roi.x,roi.y+y),
                                  threshold,
                                  &cmp(roi.x,roi.y+y));
        
      }
    }
   
    template<class S>
    static void apply_tiled_thresh(const Img<S> &s, Img8u &dst, 
                                   Img<S> &buf1, Img<S> &buf2, int ts, int threshold,
                                   BinaryCompareOp *cmp, bool lin){
      
      int w = s.getWidth();
      int h = s.getHeight();
      Size t(ts,ts);
      int dim = ts*ts;
      
      int NX = w/ts;
      int NY = h/ts;
      Rect r(0,0,ts,ts);

      //      Time tt2 = Time::now();
      /*
          the old version is outdated now
          Benchmarks: 1280x960 single channel (scaled lena image)
          OLD with IPP: 13.5ms
          
          NEW (no IPP involved) 2.7ms
          */
      
#ifdef ICL_HAVE_IPP 
      int bw = w/ts;
      // Time tt = tt2;
      
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

      //tt.printAge("roi-mean-downscaling");
      //tt = Time::now();
      buf1.scaledCopy(&buf2,lin?interpolateLIN:interpolateNN);
      //tt.printAge("scaled copy");
      //tt = Time::now();
      cmp->apply(&s,&buf2,bpp(dst));
      //tt.printAge("compare");
    
#else
      for(int c=s.getChannels()-1;c>=0;--c){
        //S *pbuf1 = buf1.begin(c);
        const Channel<S> srcChan = s[c];
        Channel8u dstChan = dst[c];
        
        if(lin){
          Channel<S> bufChan = buf1[0];
          const Channel<S> chan = s[c];
          for(int y=0;y<NY;++y){
            r.y = ts*y;
            for(int x=0;x<NX;++x){
              r.x = ts*x;
              bufChan(x,y) = roi_mean(chan,dim,r);
            }
          } 
          const Point o(ts/2,ts/2);
          //S c1[ts],c2[ts];
          S *c1 = new S[ts];
          S *c2 = new S[ts];

          Img32f cmp(s.getSize(),1);
          Channel32f chanCmp = cmp[0];

          /// major part of the image
          for(int y=1;y<NY;++y){
            r.y = ts*y - o.y;
            for(int x=1;x<NX;++x){
              r.x = ts*x - o.y;
              compare_lin(c1,c2,ts,ts,
                          bufChan(x-1,y-1),
                          bufChan(x,y-1),
                          bufChan(x-1,y),
                          bufChan(x,y),
                          srcChan, dstChan, 
                          r, threshold, chanCmp);
            }
          }
          /// image borders, here, the closest global value is used
          /*
                                                  xm
              +-------+-------+-------+-------+-------+
              |       |       |       |       |       |
              |   x...+...x...+ ..x...+...x...+...x   |
              |   .   |   .   |   .   |   .   |   .   |
              +---.---+---.---+---.---+---.---+---+---+
              |   .   |   .   |   .   |   .   |   .   |
              |   x...+...x...+ ..x...+...x...+...x   |
              |   .   |   .   |   .   |   .   |   .   |
              +---.---+---.---+---.---+---.---+---.---+
              |   .   |   .   |   .   |   .   |   .   |
              |   x...+...x...+ ..x...+...x...+...x   |
              |   .   |   .   |   .   |   .   |   .   |
              +---.---+---.---+---.---+---.---+---.---+
              |   .   |   .   |   .   |   .   |   .   |
              |   x...+...x...+ ..x...+...x...+...x   |   ym;
              |       |       |       |       |       |
              +-------+-------+-------+-------+-------+
              */
          const int th = ts/2;
          const int xm = NX*ts - th, ym = NY*ts - th;
          // corners
          roi_cmp(srcChan, Rect(0,0,th,th), bufChan(0,0), dstChan, threshold);
          roi_cmp(srcChan, Rect(xm,0,th,th), bufChan(NX-1,0), dstChan, threshold);
          roi_cmp(srcChan, Rect(0,ym,th,th), bufChan(0,NY-1), dstChan, threshold);
          roi_cmp(srcChan, Rect(xm,ym,th,th), bufChan(NX-1,NY-1), dstChan, threshold);
          for(int x=1; x<NX;++x){
            // top
            compare_lin(c1,c2,ts,th,
                        bufChan(x-1,0),bufChan(x,0),
                        bufChan(x-1,0),bufChan(x,0),
                        srcChan, dstChan, 
                        Rect(th+(x-1)*ts,0,ts,th), threshold, chanCmp);
            // bottom
            compare_lin(c1,c2,ts,th,
                        bufChan(x-1,NY-1),bufChan(x,NY-1),
                        bufChan(x-1,NY-1),bufChan(x,NY-1),
                        srcChan, dstChan, 
                        Rect(th+(x-1)*ts,ym,ts,th), threshold, chanCmp);
          }
          for(int y=1;y<NY;++y){
            // left
            compare_lin(c1,c2,th,ts,
                        bufChan(0,y-1),bufChan(0,y-1),
                        bufChan(0,y),bufChan(0,y),
                        srcChan, dstChan, 
                        Rect(0,th+(y-1)*ts,th,ts), threshold, chanCmp);
            // right
            compare_lin(c1,c2,th,ts,
                        bufChan(NX-1,y-1),bufChan(NX-1,y-1),
                        bufChan(NX-1,y),bufChan(NX-1,y),
                        srcChan, dstChan, 
                        Rect(xm,th+(y-1)*ts,th,ts), threshold, chanCmp);
          }

          delete[] c1;
          delete[] c2;
        }else{
          /// todo: handle right colum and bottom row!
          for(int y=0;y<NY;++y){
            r.y = ts*y;
            for(int x=0;x<NX;++x){
              r.x = ts*x;
              S mean = roi_mean(srcChan,dim,r);
              roi_cmp(srcChan,r,mean,dstChan,threshold);
            }
          }
        }
      }
#endif
    }

    inline bool is_int(float x){
      return float((int)x) == x;
    }
    inline bool can_devide_by(int i, int devider){
      return is_int(float(i)/float(devider));
    }
    
  
    template<> void LocalThresholdOp::apply_a<LocalThresholdOp::tiledNN>(const ImgBase *src, ImgBase **dst){
      // {{{ open
      
      int ts = 2*getMaskSize();
      // find closes number that devides w and h
      int w = src->getWidth(), h = src->getHeight();
      int max = iclMax(w,h);

      for(int i=0;i<max;++i){
        if(can_devide_by(w,ts+i) && can_devide_by(h,ts+i)){
          ts = ts+i;
          break;
        }
        if(can_devide_by(w,ts-i) && can_devide_by(h,ts-i)){
          ts = ts-i;
          break;
        }
      }
      setPropertyValue("actually used mask size",ts);
      //      std::cout << "orig: " << 2*getMaskSize() << " --> adapted:" << ts << std::endl;
      
      ICLASSERT_RETURN(ts>1);
      Size size = src->getSize();
      ensureCompatible(&m_tiledBuf1,src->getDepth(),size/ts, 1, formatMatrix);
      ensureCompatible(&m_tiledBuf2,src->getDepth(),size,1,formatMatrix);
  
      switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                \
        case depth##D:                                          \
        apply_tiled_thresh(*src->asImg<icl##D>(),               \
                           *(*dst)->asImg<icl8u>(),             \
                           *m_tiledBuf1->asImg<icl##D>(),       \
                           *m_tiledBuf2->asImg<icl##D>(),       \
                           ts, getGlobalThreshold(), m_cmp,     \
                           getAlgorithm() == tiledLIN);         \
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

      setPropertyValue("actually used mask size",s);
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
