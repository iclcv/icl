#include <ICLCC/CCFunctions.h>
#include <ICLCore/Img.h>
#include <map>
#include <ICLCC/CCLUT.h>


namespace icl{
  
  using std::string;

#define AVL ccAvailable
#define IMU ccEmulated
#define UAV ccUnavailable 
#define IPS ccImpossible
#define ADP ccAdapted

  static const unsigned int NFMTS = 7;
  static const ccimpl g_aeAvailableTable[NFMTS*NFMTS] = {
    /*                      |-------------- dst format ------------------> */ 
    /*  ___                 gray   rgb    hls    yuv    lab  chroma matrix */
    /*   |        gray  */  AVL,   AVL,   AVL,   AVL,   AVL,   IPS,  ADP,   
    /*   |        rgb   */  AVL,   AVL,   AVL,   AVL,   AVL,   AVL,  ADP,   
    /*  src-      hls   */  AVL,   AVL,   AVL,   IMU,   IMU,   IMU,  ADP, 
    /* format     yuv   */  AVL,   AVL,   IMU,   AVL,   IMU,   IMU,  ADP, 
    /*   |        lab   */  AVL,   AVL,   IMU,   IMU,   AVL,   IMU,  ADP, 
    /*   |       chroma */  IPS,   IPS,   IPS,   IPS,   IPS,   AVL,  ADP, 
    /*   V       matrix */  ADP,   ADP,   ADP,   ADP,   ADP,   ADP,  ADP
  };

#undef AVL 
#undef IMU
#undef UAV
#undef IPS
  std::string translateCCImpl(ccimpl i){
    // {{{ open

    static string s_asNames[5] = { "available" , "emulated","adapted","unavailable", "impossible" };
    return s_asNames[i];
  }

  // }}}

  ccimpl translateCCImlp(const std::string &s){
    // {{{ open

    if(s.length() < 3){
      ERROR_LOG("ccimpl \""<<s<<"\" is not defined"); 
      return ccUnavailable;
    }
    switch(s[0]){
      case 'a': 
        if(s[1] == 'v') return  ccAvailable;
        else return ccAdapted;
      case 'e': return ccEmulated;
      case 'u': return ccUnavailable;
      case 'i': return ccImpossible;
      default: 
        ERROR_LOG("ccimpl \""<<s<<"\" is not defined");
        return ccUnavailable; 
    }
  }

  // }}}

  ccimpl cc_available(format srcFmt, format dstFmt){
    return g_aeAvailableTable[srcFmt*NFMTS + dstFmt];
  }

  std::map<format, std::map<format,CCLUT*> > g_mapCCLUTs;
  
  bool lut_available(format srcFmt, format dstFmt){
    // {{{ open

    return g_mapCCLUTs[srcFmt][dstFmt] != 0;
  }

  // }}}

  void createLUT(format srcFmt, format dstFmt){
    // {{{ open

    CCLUT *&lut = g_mapCCLUTs[srcFmt][dstFmt];
    if(!lut){
      lut = new CCLUT(srcFmt,dstFmt);
    }else{
      WARNING_LOG("lookup table created twice!");
    }
  }

  // }}}
  
  void releaseLUT(format srcFmt, format dstFmt){
    // {{{ open

    CCLUT *&lut = g_mapCCLUTs[srcFmt][dstFmt];
    if(lut){
      delete lut;
      lut = 0;
    }else{
      WARNING_LOG("lookup table does not exist!");
    }
  }

  // }}}

  void releaseAllLUTs(){
    // {{{ open

    typedef std::map<format,CCLUT*> fmap;
    typedef std::map<format,fmap> ffmap; 
    for(ffmap::iterator it= g_mapCCLUTs.begin(); it!= g_mapCCLUTs.end();it++){
      fmap &f = (*it).second;
      for(fmap::iterator jt = f.begin();jt != f.end(); jt++){
        if((*jt).second){
          delete (*jt).second;
          (*jt).second = 0;
        }
      }
    }
  }

  // }}}
  
  void cc_util_rgb_to_yuv(const icl32s r, const icl32s g, const icl32s b, icl32s &y, icl32s &u, icl32s &v){
    // {{{ integer open
  
    /// fixed point approximation of the the rgb2yuv version :
    y = ( 1254097*r + 2462056*g + 478151*b ) >> 22;  
    u = ( 2366989*(b-y) + 534773760        ) >> 22;
    v = ( 2991658*(r-y) + 534773760        ) >> 22;
  }
  // }}}
  void cc_util_yuv_to_rgb(const icl32s y,const icl32s u,const icl32s v, icl32s &r, icl32s &g, icl32s &b){
    // {{{ open
    icl32s u2 = 14343*u - 1828717; 
    icl32s v2 = 20231*v - 2579497; 
    
    r = y +  ( ( 290 * v2 ) >> 22 );
    g = y -  ( ( 100  * u2 + 148 * v2) >> 22 );
    b = y +  ( ( 518 * u2 ) >> 22 );
  } 
  // }}}
  void cc_util_rgb_to_hls(const icl32f r255,const icl32f g255,const icl32f b255, icl32f &h, icl32f &l, icl32f &s){
    // {{{ open

    icl32f r = r255/255;
    icl32f g = g255/255;
    icl32f b = b255/255;
    
    icl32f m,v;
    getMinAndMax(r,g,b,m,v);
    
    if((l = (m + v) / 2.0) <= 0.0){
      l=0; h=0; s=0; // just define anything!
      return;
    }
    
    icl32f vm = v-m;
    if ( vm > 0.0 ) {
      if(l<=0.5){
        s=vm/(v+m);
      }else{
        s=vm/(2.0-v-m);
      }
    }else{
      l*=255;
      s=0;
      h=0; // just define anything!
      return;
    }
  

    icl32f r2 = (v - r) / vm;
    icl32f g2 = (v - g) / vm;
    icl32f b2 = (v - b) / vm;
    
    if (r == v)
      h = (g == m ? 5.0 + b2 : 1.0 - g2);
    else if (g == v)
      h = (b == m ? 1.0 + r2 : 3.0 - b2);
    else
      h = (r == m ? 3.0 + g2 : 5.0 - r2);
    
    //    h /= 6;

    h *=255./6;
    if(h==255)h=0;
    l *=255;
    s *=255;
  }

  // }}}
  void cc_util_rgb_to_xyz(const icl32f r, const icl32f g, const icl32f b, icl32f &X, icl32f &Y, icl32f &Z){
    // {{{ open
    static icl32f m[3][3] = {{ 0.412453, 0.35758 , 0.180423},
                             { 0.212671, 0.71516 , 0.072169},
                             { 0.019334, 0.119193, 0.950227}};
    X = m[0][0] * r + m[0][1] * g + m[0][2] * b;
    Y = m[1][0] * r + m[1][1] * g + m[1][2] * b;
    Z = m[2][0] * r + m[2][1] * g + m[2][2] * b;
  }
  // }}}
  void cc_util_xyz_to_lab(const icl32f X, const icl32f Y, const icl32f Z, icl32f &L, icl32f &a, icl32f &b){
    // {{{ open

    static const icl32f wX = 95.0456;
    static const icl32f wY = 100.0;
    static const icl32f wZ = 108.8754;
    static const icl32f _13 = 1.0/3.0;
    
    icl32f XXn = X / wX;
    icl32f YYn = Y / wY;
    icl32f ZZn = Z / wZ;

    L = (YYn > 0.008856) ? ((116 * pow (YYn, _13))-16) : (903.3 * YYn);
    
    icl32f fX = (XXn > 0.008856) ? pow (XXn, _13) : 7.787 * XXn + (16 / 116);
    icl32f fY = (YYn > 0.008856) ? pow (YYn, _13) : 7.787 * YYn + (16 / 116); 
    icl32f fZ = (ZZn > 0.008856) ? pow (ZZn, _13) : 7.787 * ZZn + (16 / 116);
    
    a = 500.0 * (fX - fY) + 127;
    b = 200.0 * (fY - fZ) + 127;
    
    
    // Cs = sqrt ((as * as) + (bs * bs));
    // hab = atan (bs / as);
  }

  // }}}
  void cc_util_hls_to_rgb(const icl32f h255, const icl32f l255, const icl32f sl255, icl32f &r, icl32f &g, icl32f &b){
    // {{{ open

    // H,L,S,R,G,B in range [0,255]
    icl32f h   = h255/255;
    icl32f l   = l255/255;
    icl32f sl  = sl255/255;
    
    icl32f v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if (v <= 0) {
      r = g = b = 0.0;
      return;
    } 
    
    icl32f m = l + l - v;
    icl32f sv = (v - m ) / v;
    h *= 6.0;
    int sextant = (int)h;	
    icl32f fract = h - sextant;
    icl32f vsf = v * sv * fract;
    icl32f mid1 = m + vsf;
    icl32f mid2 = v - vsf;
    switch (sextant) {
      case 0: r = v;    g = mid1; b = m;    break;
      case 1: r = mid2; g = v;    b = m;    break;
      case 2: r = m;    g = v;    b = mid1; break;
      case 3: r = m;    g = mid2; b = v;    break;
      case 4: r = mid1; g = m;    b = v;    break;
      case 5: r = v;    g = m;    b = mid2; break;
    }
    
    r *= 255;
    g *= 255;
    b *= 255;
  }

  // }}}
  void cc_util_rgb_to_chroma(const icl32f r, const icl32f g, const icl32f b, icl32f &chromaR, icl32f &chromaG){
    // {{{ open

    icl32f sum = r+g+b;
    sum+=!sum; //avoid division by zero
    chromaR=r*255/sum;
    chromaG=g*255/sum;
  }

  // }}}
  void cc_util_lab_to_xyz(const icl32f l, const icl32f a, const icl32f b, icl32f &x, icl32f &y, icl32f &z){
    // {{{ open

    static const icl32f d = 6.0/29.0;
    static const icl32f n = 16.0/116.0;
    static const icl32f f = 3*d*d;

    // white points values ???
    static const icl32f wX = 95.0456;
    static const icl32f wY = 100.0;
    static const icl32f wZ = 108.8754;

    icl32f fy = (l+16)/116;
    icl32f fx = fy+a/500;
    icl32f fz = fy-b/200;

    y = (fy>d) ?  wY*pow(fy,3) : (fy-n)*f*wY;
    x = (fx>d) ?  wX*pow(fx,3) : (fx-n)*f*wX;
    z = (fz>d) ?  wZ*pow(fz,3) : (fz-n)*f*wZ;
  }

  // }}}
  void cc_util_xyz_to_rgb(const icl32f x, const icl32f y, const icl32f z, icl32f &r, icl32f &g, icl32f &b){
    // {{{ open

    static icl32f m[3][3] = {{ 3.2405, -1.5372,-0.4985},
                             {-0.9693,  1.8760, 0.0416},
                             { 0.0556, -0.2040, 1.0573}};
    // xyz = m rgb
    // rgb = m^-1xyz
    r = m[0][0] * x + m[0][1] * y + m[0][2] * z;
    g = m[1][0] * x + m[1][1] * y + m[1][2] * z;
    b = m[2][0] * x + m[2][1] * y + m[2][2] * z;
  }

  // }}}
 
  
#define GET_3_CHANNEL_POINTERS_DIM(T,I,P1,P2,P3,DIM) T *P1=I->getData(0),*P2=I->getData(1),*P3=I->getData(2); int DIM = I->getDim() 
#define GET_3_CHANNEL_POINTERS_NODIM(T,I,P1,P2,P3) T *P1=I->getData(0),*P2=I->getData(1),*P3=I->getData(2)
#define GET_2_CHANNEL_POINTERS_DIM(T,I,P1,P2,DIM) T *P1=I->getData(0),*P2=I->getData(1); int DIM = I->getDim()
#define GET_2_CHANNEL_POINTERS_NODIM(T,I,P1,P2) T *P1=I->getData(0),*P2=I->getData(1)

  template<class S, class D, format srcFmt, format dstFmt> struct CCFunc{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      (void)src; (void)dst; (void)roiOnly;
    }
  };

  // }}}
  template<class S, class D, format srcDstFmt> struct CCFunc<S,D,srcDstFmt,srcDstFmt>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      if(roiOnly){
        src->convertROI(dst);
      }else{
        src->convert(dst);
      }
    }
    
  };

  // }}}

  /// FROM FORMAT RGB
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        const ImgIterator<S> itR = src->beginROI(0);
        const ImgIterator<S> itG = src->beginROI(1);
        const ImgIterator<S> itB = src->beginROI(2);
        ImgIterator<D> itGray = dst->beginROI(0);
        const ImgIterator<D> itEnd = dst->endROI(0);
        for(;itGray!= itEnd;++itR,++itG,++itB,++itGray){
          *itGray = clipped_cast<S,D>((*itR + *itG + *itB)/3);
        }
      }else{
        GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
        D *gr = dst->getData(0);
        for(int i=0;i<dim;++i){
          gr[i] = clipped_cast<S,D>((r[i]+g[i]+b[i])/3);
        }
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatHLS>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");

      register icl32f reg_h, reg_l, reg_s;
      if(roiOnly){
        const ImgIterator<S> itR = src->beginROI(0);
        const ImgIterator<S> itG = src->beginROI(1);
        const ImgIterator<S> itB = src->beginROI(2);
        ImgIterator<D> itH = dst->beginROI(0);
        ImgIterator<D> itL = dst->beginROI(1);
        ImgIterator<D> itS = dst->beginROI(2);
        const ImgIterator<S> itEnd = src->endROI(0);
        for(;itR!= itEnd;++itR,++itG,++itB,++itH,++itL,++itS){
          cc_util_rgb_to_hls(clipped_cast<S,icl32f>(*itR),
                             clipped_cast<S,icl32f>(*itG),
                             clipped_cast<S,icl32f>(*itB),
                             reg_h,reg_l,reg_s);
          *itH = clipped_cast<icl32f,D>(reg_h);
          *itL = clipped_cast<icl32f,D>(reg_l);
          *itS = clipped_cast<icl32f,D>(reg_s);
        }
      }else{
        GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
        GET_3_CHANNEL_POINTERS_NODIM(D,dst,h,l,s);
        
        for(int i=0;i<dim;++i){
          cc_util_rgb_to_hls(clipped_cast<S,icl32f>(r[i]),
                             clipped_cast<S,icl32f>(g[i]),
                             clipped_cast<S,icl32f>(b[i]),
                             reg_h,reg_l,reg_s);
          h[i] = clipped_cast<icl32f,D>(reg_h);
          l[i] = clipped_cast<icl32f,D>(reg_l);
          s[i] = clipped_cast<icl32f,D>(reg_s);
        }
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatChroma>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      GET_2_CHANNEL_POINTERS_NODIM(D,dst,cromaR,cromaG);
      register S sum;
      for(int i=0;i<dim;++i){
        sum = r[i]+g[i]+b[i];
        sum+=!sum; //avoid division by zero
        cromaR[i]=clipped_cast<S,D>((r[i]*255)/sum);
        cromaG[i]=clipped_cast<S,D>((g[i]*255)/sum);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatYUV>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,y,u,v);
      register icl32s reg_y, reg_u, reg_v;
      for(int i=0;i<dim;++i){ 
        cc_util_rgb_to_yuv(clipped_cast<S,icl32s>(r[i]),
                           clipped_cast<S,icl32s>(g[i]),
                           clipped_cast<S,icl32s>(b[i]),
                           reg_y, reg_u, reg_v);
        y[i] = clipped_cast<icl32s,D>(reg_y);
        u[i] = clipped_cast<icl32s,D>(reg_u);
        v[i] = clipped_cast<icl32s,D>(reg_v);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatLAB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,LL,aa,bb);
      
      register icl32f reg_X,reg_Y,reg_Z,reg_L, reg_a, reg_b;
      for(int i=0;i<dim;++i){ 
        cc_util_rgb_to_xyz(clipped_cast<S,icl32f>(r[i]),
                           clipped_cast<S,icl32f>(g[i]),
                           clipped_cast<S,icl32f>(b[i]),
                           reg_X,reg_Y,reg_Z);
        cc_util_xyz_to_lab(reg_X,reg_Y,reg_Z,reg_L,reg_a,reg_b);
        LL[i] = clipped_cast<icl32f,D>(reg_L);
        aa[i] = clipped_cast<icl32f,D>(reg_a);
        bb[i] = clipped_cast<icl32f,D>(reg_b);
      }
    }
    
  };

  // }}}


  /// FROM FORMAT GRAY
  template<class S, class D> struct CCFunc<S,D,formatGray,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        const ImgIterator<S> itGray = src->beginROI(0);
        ImgIterator<D> itR = dst->beginROI(0);
        ImgIterator<D> itG = dst->beginROI(1);
        ImgIterator<D> itB = dst->beginROI(2);
        const ImgIterator<D> itEnd = dst->endROI(0);
        for(;itR!= itEnd;++itG,++itR,++itGray,++itB){
          *itR = *itG = *itB = clipped_cast<S,D>(*itGray);
        }
      }else{
        const S *gr = src->getData(0);
        GET_3_CHANNEL_POINTERS_DIM(D,dst,r,g,b,dim);
        for(int i=0;i<dim;++i){
          r[i] = g[i] = b[i] = clipped_cast<S,D>(gr[i]);
        }
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatHLS>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        const ImgIterator<S> itG = src->beginROI(0);
        ImgIterator<D> itH = dst->beginROI(0);
        ImgIterator<D> itL = dst->beginROI(1);
        ImgIterator<D> itS = dst->beginROI(2);
        const ImgIterator<D> itEnd = dst->endROI(0);
        for(;itH!= itEnd;++itG,++itH,++itL,++itS){
          *itL = clipped_cast<S,D>(*itG);
          *itH = *itS = D(0);
        }
      }else{
        const S *gr = src->getData(0);
        GET_3_CHANNEL_POINTERS_DIM(D,dst,h,l,s,dim);
        for(int i=0;i<dim;++i){
          h[i] = s[i] = D(0);
          l[i] = clipped_cast<S,D>(gr[i]);
        }
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatYUV>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        const ImgIterator<S> itG = src->beginROI(0);
        ImgIterator<D> itY = dst->beginROI(0);
        ImgIterator<D> itU = dst->beginROI(1);
        ImgIterator<D> itV = dst->beginROI(2);

        const ImgIterator<S> itEnd = src->endROI(0);
        for(;itG!= itEnd;++itG,++itY,++itU,++itV){
          *itY = clipped_cast<S,D>(*itG);
          *itU = *itV = D(127);
        }
      }else{
        const S *gr = src->getData(0);
        GET_3_CHANNEL_POINTERS_DIM(D,dst,y,u,v,dim);
        for(int i=0;i<dim;++i){
          y[i] = clipped_cast<S,D>(gr[i]);
          u[i] = v[i] = D(127);
        }
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatLAB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        const ImgIterator<S> itG = src->beginROI(0);
        ImgIterator<D> itL = dst->beginROI(0);
        ImgIterator<D> itA = dst->beginROI(1);
        ImgIterator<D> itB = dst->beginROI(2);
        const ImgIterator<S> itEnd = src->endROI(0);
        for(;itG!= itEnd;++itG,++itL,++itA,++itB){
          *itL = clipped_cast<S,D>(*itG);
          *itA = *itB = D(127);
        }
      }else{
        const S *gr = src->getData(0);
        GET_3_CHANNEL_POINTERS_DIM(D,dst,L,a,b,dim);
        for(int i=0;i<dim;++i){
          L[i] = clipped_cast<S,D>(gr[i]);
          a[i] = b[i] = D(127);
        }
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatChroma>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      WARNING_LOG("converting formatGray to formatChroma does not make sense");
      (void) src;
      dst->clear(-1,85,roiOnly);
    }
    
  };

  // }}}
 
  /// FROM FORMAT HLS
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        convertChannelROI(src,1,src->getROIOffset(),src->getROISize(), 
                          dst,0,dst->getROIOffset(),dst->getROISize());
      }else{
        icl::convert(src->getData(1),src->getData(1)+src->getDim(), dst->getData(0));
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      
      register icl32f reg_r(0), reg_g(0), reg_b(0);
      if(roiOnly){
        const ImgIterator<S> itH = src->beginROI(0);
        const ImgIterator<S> itL = src->beginROI(1);
        const ImgIterator<S> itS = src->beginROI(2);
        ImgIterator<D> itR = dst->beginROI(0);
        ImgIterator<D> itG = dst->beginROI(1);
        ImgIterator<D> itB = dst->beginROI(2);
        const ImgIterator<S> itEnd = src->endROI(0);
        for(;itH!= itEnd;++itH,++itL,++itS,++itR,++itG,++itB){
          cc_util_hls_to_rgb(clipped_cast<S,icl32f>(*itH),
                             clipped_cast<S,icl32f>(*itL),
                             clipped_cast<S,icl32f>(*itS),
                             reg_r,reg_g,reg_b);
          *itR = clipped_cast<icl32f,D>(reg_r);
          *itG = clipped_cast<icl32f,D>(reg_g);
          *itB = clipped_cast<icl32f,D>(reg_b);
        }
      }else{
        GET_3_CHANNEL_POINTERS_DIM(const S,src,h,l,s,dim);
        GET_3_CHANNEL_POINTERS_NODIM(D,dst,r,g,b);
        
        for(int i=0;i<dim;++i){
          cc_util_hls_to_rgb(clipped_cast<S,icl32f>(h[i]),
                             clipped_cast<S,icl32f>(l[i]),
                             clipped_cast<S,icl32f>(s[i]),
                             reg_r,reg_g,reg_b);
          r[i] = clipped_cast<icl32f,D>(reg_r);
          g[i] = clipped_cast<icl32f,D>(reg_g);
          b[i] = clipped_cast<icl32f,D>(reg_b);
        }
      }
    }
    
  };

  // }}}

  /// FROM FORMAT LAB
  template<class S, class D> struct CCFunc<S,D,formatLAB,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        convertChannelROI(src,0,src->getROIOffset(),src->getROISize(), 
                          dst,0,dst->getROIOffset(),dst->getROISize());
      }else{
        icl::convert(src->getData(0),src->getData(0)+src->getDim(), dst->getData(0));        
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatLAB,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
      if(roiOnly){
        const ImgIterator<S> itL = src->beginROI(0);
        const ImgIterator<S> itA = src->beginROI(1);
        const ImgIterator<S> itB = src->beginROI(2);
        ImgIterator<D> itR = dst->beginROI(0);
        ImgIterator<D> itG = dst->beginROI(1);
        ImgIterator<D> itBl = dst->beginROI(2);
        const ImgIterator<S> itEnd = src->endROI(0);
        for(;itL!= itEnd;++itL,++itA,++itB,++itR,++itG,++itBl){
          cc_util_lab_to_xyz(clipped_cast<S,icl32f>(*itL),
                             clipped_cast<S,icl32f>(*itA),
                             clipped_cast<S,icl32f>(*itB),
                             reg_x,reg_y,reg_z);
          cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
          *itR  = clipped_cast<icl32f,D>(reg_r);
          *itG  = clipped_cast<icl32f,D>(reg_g);
          *itBl = clipped_cast<icl32f,D>(reg_b);
        }
      }else{
        GET_3_CHANNEL_POINTERS_DIM(const S,src,ll,aa,bb,dim);
        GET_3_CHANNEL_POINTERS_NODIM(D,dst,r,g,b);
        for(int i=0;i<dim;++i){
          cc_util_lab_to_xyz(clipped_cast<S,icl32f>(ll[i]),
                             clipped_cast<S,icl32f>(aa[i]),
                             clipped_cast<S,icl32f>(bb[i]),
                             reg_x,reg_y,reg_z);
          cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
          r[i] = clipped_cast<icl32f,D>(reg_r);
          g[i] = clipped_cast<icl32f,D>(reg_g);
          b[i] = clipped_cast<icl32f,D>(reg_b);
        }
      }
    }
    
  };

  // }}}


  /// FROM FORMAT YUV
  template<class S, class D> struct CCFunc<S,D,formatYUV,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      if(roiOnly){
        convertChannelROI(src,0,src->getROIOffset(),src->getROISize(), 
                          dst,0,dst->getROIOffset(),dst->getROISize());
      }else{
        icl::convert(src->getData(0),src->getData(0)+src->getDim(), dst->getData(0));        
      }
    }    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatYUV,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
      FUNCTION_LOG("");
      register icl32s reg_r, reg_g, reg_b;
      if(roiOnly){
        const ImgIterator<S> itY = src->beginROI(0);
        const ImgIterator<S> itU = src->beginROI(1);
        const ImgIterator<S> itV = src->beginROI(2);
        ImgIterator<D> itR = dst->beginROI(0);
        ImgIterator<D> itG = dst->beginROI(1);
        ImgIterator<D> itB = dst->beginROI(2);
        const ImgIterator<S> itEnd = src->endROI(0);
        for(;itY!= itEnd;++itY,++itU,++itV,++itR,++itG,++itB){
          cc_util_yuv_to_rgb(clipped_cast<S,icl32s>(*itY),
                             clipped_cast<S,icl32s>(*itU),
                             clipped_cast<S,icl32s>(*itV),
                             reg_r, reg_g, reg_b);
          *itR = clipped_cast<icl32s,D>(reg_r);
          *itG = clipped_cast<icl32s,D>(reg_g);
          *itB = clipped_cast<icl32s,D>(reg_b);
        }
      }else{
        GET_3_CHANNEL_POINTERS_DIM(const S,src,y,u,v,dim);
        GET_3_CHANNEL_POINTERS_NODIM(D,dst,r,g,b);
        for(int i=0;i<dim;++i){
          cc_util_yuv_to_rgb(clipped_cast<S,icl32s>(y[i]),
                             clipped_cast<S,icl32s>(u[i]),
                             clipped_cast<S,icl32s>(v[i]),
                             reg_r, reg_g, reg_b);
          r[i] = clipped_cast<icl32s,D>(reg_r);
          g[i] = clipped_cast<icl32s,D>(reg_g);
          b[i] = clipped_cast<icl32s,D>(reg_b);
        }
      }
    }
    
  };

  // }}}

  template<class S, class D> void cc_sd(const Img<S> *src, Img<D> *dst, bool roiOnly){
  // {{{ open

    // {{{ definition of CASE_LABEL(XXX)
    
#define INNER_CASE_LABEL(XXX,YYY) \
  case format##YYY: CCFunc<S,D,format##XXX,format##YYY>::convert(src,dst,roiOnly); break
    
#define CASE_LABEL(XXX)                         \
      case format##XXX:                         \
        switch(dst->getFormat()){               \
          INNER_CASE_LABEL(XXX,RGB);            \
          INNER_CASE_LABEL(XXX,HLS);            \
          INNER_CASE_LABEL(XXX,LAB);            \
          INNER_CASE_LABEL(XXX,YUV);            \
          INNER_CASE_LABEL(XXX,Gray);           \
          INNER_CASE_LABEL(XXX,Chroma);         \
          default: ICL_INVALID_FORMAT;          \
        }                                       \
        break

    // }}}
    
    switch(src->getFormat()){
      CASE_LABEL(RGB);
      CASE_LABEL(HLS);
      CASE_LABEL(YUV);
      CASE_LABEL(Gray);
      CASE_LABEL(Chroma);
      CASE_LABEL(LAB);
      default:
        ICL_INVALID_FORMAT;
    }
#undef CASE_LABEL                                          
  }  

  // }}}
 
  template<class S> void cc_s(const Img<S> *src, ImgBase *dst, bool roiOnly){
    // {{{ open

    switch(dst->getDepth()){      //TODO depth macro
      case depth8u: cc_sd(src, dst->asImg<icl8u>(),roiOnly); break;
      case depth16s: cc_sd(src, dst->asImg<icl16s>(),roiOnly); break;
      case depth32s: cc_sd(src, dst->asImg<icl32s>(),roiOnly); break;
      case depth32f: cc_sd(src, dst->asImg<icl32f>(),roiOnly); break;
      case depth64f: cc_sd(src, dst->asImg<icl64f>(),roiOnly); break;
      default:
        ICL_INVALID_DEPTH;
    }
  }

  // }}}

  void cc(const ImgBase *src, ImgBase *dst, bool roiOnly){
    // {{{ open

    ICLASSERT_RETURN( src );
    ICLASSERT_RETURN( dst );
    
    if(roiOnly){
      /// check for equal roi sizes
      ICLASSERT_RETURN(src->getROISize() == dst->getROISize());
    }else{
      /// adapt the size of the destination image
      dst->setSize(src->getSize());
    }
    /// ensure, that the roiOnly - flag is necessary
    if(roiOnly && src->hasFullROI() && dst->hasFullROI()){
      roiOnly = false;
    }
    
    if(lut_available(src->getFormat(),dst->getFormat())){
      g_mapCCLUTs[src->getFormat()][dst->getFormat()]->cc(src,dst,roiOnly);
      return;
    }
    
    switch(cc_available(src->getFormat(), dst->getFormat())){
      case ccAvailable:
        switch(src->getDepth()){ //TODO depth macro
          case depth8u: cc_s(src->asImg<icl8u>(),dst,roiOnly); break;
          case depth16s: cc_s(src->asImg<icl16s>(),dst,roiOnly); break;
          case depth32s: cc_s(src->asImg<icl32s>(),dst,roiOnly); break;
          case depth32f: cc_s(src->asImg<icl32f>(),dst,roiOnly); break;
          case depth64f: cc_s(src->asImg<icl64f>(),dst,roiOnly); break;
          default:
            ICL_INVALID_DEPTH;
        }
        break;
      case ccEmulated:{
        if(roiOnly){
          ImgBase *buf=imgNew(src->getDepth(), src->getROISize(),formatRGB);
          cc(src,buf,true);
          cc(buf,dst,true);
          delete buf;
        }else{
          ImgBase *buf=imgNew(src->getDepth(), src->getSize(), formatRGB);
          cc(src,buf);
          cc(buf,dst);
          delete buf;
        }
        break; 
      }
      case ccAdapted:{
        int n = iclMin(src->getChannels(),dst->getChannels());
        switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)  case depth##D:                                                                          \
          switch(dst->getDepth()){                                                                                        \
            case depth8u: for(int i=0;i<n;i++){                                                                           \
              convertChannelROI(src->asImg<icl##D>(),i,src->getROIOffset(),src->getROISize(),                             \
                                dst->asImg<icl8u>(),i,dst->getROIOffset(),dst->getROISize());                             \
            }                                                                                                             \
            break;                                                                                                        \
            case depth16s: for(int i=0;i<n;i++){                                                                          \
              convertChannelROI(src->asImg<icl##D>(),i,src->getROIOffset(),src->getROISize(),                             \
                                dst->asImg<icl16s>(),i,dst->getROIOffset(),dst->getROISize());                            \
            }                                                                                                             \
            break;                                                                                                        \
            case depth32s: for(int i=0;i<n;i++){                                                                          \
              convertChannelROI(src->asImg<icl##D>(),i,src->getROIOffset(),src->getROISize(),                             \
                                dst->asImg<icl32s>(),i,dst->getROIOffset(),dst->getROISize());                            \
            }                                                                                                             \
            break;                                                                                                        \
            case depth32f: for(int i=0;i<n;i++){                                                                          \
              convertChannelROI(src->asImg<icl##D>(),i,src->getROIOffset(),src->getROISize(),                             \
                                dst->asImg<icl32f>(),i,dst->getROIOffset(),dst->getROISize());                            \
            }                                                                                                             \
            break;                                                                                                        \
            case depth64f: for(int i=0;i<n;i++){                                                                          \
              convertChannelROI(src->asImg<icl##D>(),i,src->getROIOffset(),src->getROISize(),                             \
                                dst->asImg<icl64f>(),i,dst->getROIOffset(),dst->getROISize());                            \
            }                                                                                                             \
            break;                                                                                                        \
          }                                                                                                               \
          break;
          ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
          
        }     
        break;
      }
      case ccUnavailable:{
        ERROR_LOG("no color conversion [" << 
                  src->getFormat() << 
                  "-->" << 
                  dst->getFormat() << 
                  "] (depth:" <<
                  src->getDepth() <<
                  "-->" << 
                  dst->getDepth() <<
                  ") available!" );
        break;
      }
      case ccImpossible:{
        ERROR_LOG("color conversion [" << 
                  src->getFormat() << 
                  "-->" << 
                  dst->getFormat() << 
                  "] (depth:" <<
                  src->getDepth() <<
                  "-->" << 
                  dst->getDepth() <<
                  ") impossible!" );
        break;
      }
        
    }

  }

  // }}}


  /// additional misc (planar -> interleaved and interleaved -> planar)
  
  template<class S, class D>
  inline void planarToInterleaved_POD(int channels,int len, const S** src, D* dst){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);
    ICLASSERT_RETURN(channels>0);
    switch(channels){
      case 1:
        convert<S,D>(*src,*src+len,dst);
        break;
      case 2:{
        D* dstEnd=dst+channels*len;
        const S* s0 = src[0];
        const S* s1 = src[1];
        while (dst<dstEnd){
          *dst++ = clipped_cast<S,D>(*s0++);
          *dst++ = clipped_cast<S,D>(*s1++);
        }
        break;
      }case 3:{
        D* dstEnd=dst+channels*len;
        const S* s0 = src[0];
        const S* s1 = src[1];
        const S* s2 = src[2];
        while (dst<dstEnd){
          *dst++ = clipped_cast<S,D>(*s0++);
          *dst++ = clipped_cast<S,D>(*s1++);
          *dst++ = clipped_cast<S,D>(*s2++);
        }
        break;
      }case 4:{
        D* dstEnd=dst+channels*len;
        const S* s0 = src[0];
        const S* s1 = src[1];
        const S* s2 = src[2];
        const S* s3 = src[3];
        while (dst<dstEnd){
          *dst++ = clipped_cast<S,D>(*s0++);
          *dst++ = clipped_cast<S,D>(*s1++);
          *dst++ = clipped_cast<S,D>(*s2++);
          *dst++ = clipped_cast<S,D>(*s3++);
        }
        break;
      }default:{
        D* dstEnd=dst+channels*len;
        const S** srcEnd = src+channels;
        while (dst<dstEnd){
          for (const S** p=src;p<srcEnd;++(*p),++p,++dst ){
            *dst=clipped_cast<S,D>(*(*p));
          }
        }
      }
    }
  }

  // }}}
  
  template<class S, class D>
  inline void planarToInterleaved_Generic_NO_ROI(const Img<S> *src, D*dst){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);

    int c=src->getChannels();    
    const S **srcData = new const S*[c];
    for(int i=0;i<c;i++){
      srcData[i] = src->getData(i);
    }
    planarToInterleaved_POD(c,src->getDim(),srcData,dst);
    delete [] srcData;
  }

  // }}}
  
  template<class S, class D>
  inline void planarToInterleaved_Generic_WITH_ROI(const Img<S> *src, D *dst,int dstLineStep){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);

    int c=src->getChannels();
    int lineLength = src->getROIWidth();
    int dstImageWidth = dstLineStep/(sizeof(D));
    int dstLineJump = dstImageWidth-lineLength;

    int srcImageWidth = src->getWidth();
    int srcLineJump = srcImageWidth-lineLength;

    if(dstLineJump<0){
      ERROR_LOG("destination images linestep is too small!"); 
      return;
    }
    
    srcLineJump+=lineLength;    
    dstLineJump+=lineLength;
    
    const S** srcData=new const S* [c];
    for(int i=0;i<c;++i){
      srcData[i] = src->getROIData(i);
    }
    
    for(int y=0;y<src->getROIHeight();++y){
      planarToInterleaved_POD(c,lineLength,srcData,dst);
      for(int i=0;i<c;++i){
        srcData[i]+=srcLineJump;
      }
      dst+=dstLineJump;
    }
    delete [] srcData;
  }

  // }}}
  
  template<class S, class D>
  inline void planarToInterleaved_Generic(const Img<S> *src, D* dst, int dstLineStep){
    // {{{ open
    
    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);
    
    if(src->hasFullROI() && ( dstLineStep == -1 || dstLineStep/((int)sizeof(D)) == src->getWidth())){
      planarToInterleaved_Generic_NO_ROI(src,dst);
    }else{
      planarToInterleaved_Generic_WITH_ROI(src,dst,dstLineStep);
    }
  }
  
  // }}}
  
  
  template<class S, class D>
  inline void interleavedToPlanar_POD(int channels,int len, const S* src, D **dst){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);
    ICLASSERT_RETURN(channels>0);
    switch(channels){
      case 1:
        convert<S,D>(src,src+len,*dst);
        break;
      case 2:{
        const S* srcEnd=src+channels*len;
        D* d0 = dst[0];
        D* d1 = dst[1];
        while (src<srcEnd){
          *d0++ = clipped_cast<S,D>(*src++);
          *d1++ = clipped_cast<S,D>(*src++);
        }
        break;
      }case 3:{
        const S* srcEnd=src+channels*len;
        D* d0 = dst[0];
        D* d1 = dst[1];
        D* d2 = dst[2];
        while (src<srcEnd){
          *d0++ = clipped_cast<S,D>(*src++);
          *d1++ = clipped_cast<S,D>(*src++);
          *d2++ = clipped_cast<S,D>(*src++);
        }
        break;
      }
      case 4:{
        const S* srcEnd=src+channels*len;
        D* d0 = dst[0];
        D* d1 = dst[1];
        D* d2 = dst[2];      
        D* d3 = dst[3];
        while (src<srcEnd){
          *d0++ = clipped_cast<S,D>(*src++);
          *d1++ = clipped_cast<S,D>(*src++);
          *d2++ = clipped_cast<S,D>(*src++);
          *d3++ = clipped_cast<S,D>(*src++);
        }
        break;
      }
      default:{
        const S* srcEnd=src+channels*len;
        D** dstEnd = dst+channels;
        while (src<srcEnd){
          for (D** p=dst;p<dstEnd;++(*p),++p,++src ){
            *(*p) = clipped_cast<S,D>(*src);
          }
        }
      }
    }
  }

  // }}}
  
  template<class S, class D>
  inline void interleavedToPlanar_Generic_NO_ROI(const S* src, Img<D> *dst){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);

    int c=dst->getChannels();    
    D **dstData = new D*[c];
    for(int i=0;i<c;i++){
      dstData[i] = dst->getData(i);
    }
    interleavedToPlanar_POD(c,dst->getDim(),src,dstData);
    delete [] dstData;
  }

  // }}}
  
  template<class S, class D>
  inline void interleavedToPlanar_Generic_WITH_ROI(const S *src, Img<D> *dst, int srcLineStep){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);

    int c=dst->getChannels();
    int lineLength = dst->getROIWidth();
    int srcImageWidth = srcLineStep/(sizeof(S));
    int srcLineJump = srcImageWidth-lineLength;

    int dstImageWidth = dst->getWidth();
    int dstLineJump = dstImageWidth-lineLength;

    if(srcLineJump<0){
      ERROR_LOG("destination images linestep is too small!"); 
      return;
    }

    dstLineJump+=lineLength;    
    srcLineJump+=lineLength;    

    D** dstData=new D*[c];
    for(int i=0;i<c;++i){
      dstData[i] = dst->getROIData(i);
    }
    
    for(int y=0;y<dst->getROIHeight();++y){
      interleavedToPlanar_POD(c,lineLength,src,dstData);
      for(int i=0;i<c;++i){
        dstData[i]+=dstLineJump;
      }
      src+=srcLineJump;
    }
    delete [] dstData;
  }

  // }}}
  
  template<class S, class D>
  inline void interleavedToPlanar_Generic( const S *src, Img<D> *dst, int srcLineStep){
    // {{{ open
    
    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);
    
    if(dst->hasFullROI() && ( srcLineStep == -1 || srcLineStep/((int)sizeof(S)) == dst->getWidth())){
      interleavedToPlanar_Generic_NO_ROI(src,dst);
    }else{
      interleavedToPlanar_Generic_WITH_ROI(src,dst,srcLineStep < 0 ? dst->getLineStep()*dst->getChannels() : srcLineStep);
    }
  }
  
  // }}}


  /****

  template<class S, class D>
  inline void interleavedToPlanar_Generic(const S *src,  Img<D> *dst, int srcLineStep){
    // {{{ open
    printf("i2p called \n");
    FUNCTION_LOG("");
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);
    int srcWidth = srcLineStep/sizeof(S);
    int srcROIWidth = dst->getROIWidth(); // assume that this are equal
    int c = dst->getChannels();
    Size srcSize = dst->getROISize();
    
    if(srcLineStep != -1 &&  srcROIWidth != srcWidth){
      int lineJump = srcWidth-srcROIWidth;
      if(lineJump < 0){
        ERROR_LOG("srcLine step is too small. The source image must be at least as wide as the destination image.");
        return;
      }
      ImgIterator<D> *itDsts = new ImgIterator<D>[c];
      for(int i=0;i<c;i++){
        itDsts[i] = dst->beginROI(i);
      }
      for(int y=0;y<srcSize.height;++y){
        for(int x=0;x<srcSize.width;++x){
          for (int i=0;i<c;++i, ++src){
            *(itDsts[i]) = clipped_cast<S,D>(*src);
            itDsts[i]++;
          }
        }
        src+=lineJump;
      }
      delete [] itDsts;
      return;
    }
    if(c==1){
      if(dst->hasFullROI()){
        convert<S,D>(src,src+srcSize.getDim(),dst->getData(0));
      }else{
        std::vector<S*> tmpSrcChannelData;
        tmpSrcChannelData.push_back(const_clipped_cast<S*>(src));
        Img<S> tmpSrcImg(srcSize,c,tmpSrcChannelData);
        tmpSrcImg.convertROI(dst);
      }
      return;
    }    

    if(dst->hasFullROI()){
      D** pp=new D* [c];
      D** ppEnd=pp+c;
      for (int i=0;i<c;i++){
        pp[i]=dst->getData(i);
      }
      const S* srcEnd=src+srcSize.getDim()*c;
      while (src<srcEnd){
        for (D** p=pp;p<ppEnd;++(*p),++p,++src){
          *(*p)= clipped_cast<S,D>(*src);
        }
      }
      delete [] pp;
    }else{ // roi handling with iterators
      ImgIterator<D> *itDsts = new ImgIterator<D>[c];
      for(int i=0;i<c;i++){
        itDsts[i] = dst->beginROI(i);
      }
      const S* srcEnd=src+srcSize.getDim()*c;
      while (src<srcEnd){
        for (int i=0;i<c;++i, ++src){
          *(itDsts[i]) = clipped_cast<S,D>(*src);
          itDsts[i]++;
        }
      }
      delete [] itDsts;
    }
  }
  
  // }}}

  ********/
  
  template<class S,class D>
  void planarToInterleaved(const Img<S> *src, D* dst,int dstLineStep){
    //TODO:: special case for src->getChannels == 1
    planarToInterleaved_Generic(src,dst,dstLineStep);
  }
  
  template<class S, class D>
  void interleavedToPlanar(const S *src, Img<D> *dst, int srcLineStep){
    //TODO:: special case for src->getChannels == 1
    interleavedToPlanar_Generic(src,dst, srcLineStep);
  }
  
#ifdef HAVE_IPP

 // {{{ PLANAR_2_INTERLEAVED_IPP

#define PLANAR_2_INTERLEAVED_IPP(DEPTH)                                                                       \
  template<> void planarToInterleaved(const Img<icl##DEPTH>*src, icl##DEPTH *dst, int dstLineStep){           \
    ICLASSERT_RETURN( src );                                                                                  \
    ICLASSERT_RETURN( dst );                                                                                  \
    ICLASSERT_RETURN( src->getChannels() );                                                                   \
    if(dstLineStep == -1) dstLineStep = src->getLineStep()*src->getChannels();                                \
    switch(src->getChannels()){                                                                               \
      case 3: {                                                                                               \
        const icl##DEPTH* apucChannels[3]={src->getROIData(0),src->getROIData(1),src->getROIData(2)};         \
        ippiCopy_##DEPTH##_P3C3R(apucChannels,src->getLineStep(),dst,dstLineStep,src->getROISize());          \
        break;                                                                                                \
      }                                                                                                       \
      case 4: {                                                                                               \
        const icl##DEPTH* apucChannels[4]={src->getROIData(0),src->getROIData(1),src->getROIData(2),src->getROIData(3)}; \
        ippiCopy_##DEPTH##_P4C4R(apucChannels,src->getLineStep(),dst,dstLineStep,src->getROISize());          \
        break;                                                                                                \
      }                                                                                                       \
      default:                                                                                                \
        planarToInterleaved_Generic(src,dst,dstLineStep);                                                     \
        break;                                                                                                \
                                                                                                              \
    }                                                                                                         \
  }
  PLANAR_2_INTERLEAVED_IPP(8u)
  PLANAR_2_INTERLEAVED_IPP(16s)
  PLANAR_2_INTERLEAVED_IPP(32s)
  PLANAR_2_INTERLEAVED_IPP(32f)
#undef PLANAR_2_INTERLEAVED_IPP

  // }}}

  // {{{ INTERLEAVED_2_PLANAR_IPP

#define INTERLEAVED_2_PLANAR_IPP(DEPTH)                                                                               \
  template<> void interleavedToPlanar(const icl##DEPTH *src, Img<icl##DEPTH> *dst, int srcLineStep){                  \
    ICLASSERT_RETURN( src );                                                                                          \
    ICLASSERT_RETURN( dst );                                                                                          \
    int c = dst->getChannels();                                                                                       \
    ICLASSERT_RETURN( c );                                                                                            \
    Size s = dst->getROISize();                                                                                       \
    int dstStep = dst->getLineStep();                                                                                 \
    int srcStep = (srcLineStep == -1) ? c*s.width*sizeof(icl##DEPTH) : srcLineStep;                                   \
    switch(c){                                                                                                        \
      case 3: {                                                                                                       \
        icl##DEPTH* apucChannels[3]={dst->getROIData(0),dst->getROIData(1),dst->getROIData(2)};                       \
        ippiCopy_##DEPTH##_C3P3R(src,srcStep,apucChannels,dstStep,s);                                                 \
        break;                                                                                                        \
      }                                                                                                               \
      case 4: {                                                                                                       \
        icl##DEPTH* apucChannels[4]={dst->getROIData(0),dst->getROIData(1),dst->getROIData(2),dst->getROIData(3)};    \
        ippiCopy_##DEPTH##_C4P4R(src,srcStep,apucChannels,dstStep,s);                                                 \
        break;                                                                                                        \
      }                                                                                                               \
      default:                                                                                                        \
        interleavedToPlanar_Generic(src,dst,srcLineStep);                                                             \
        break;                                                                                                        \
    }                                                                                                                 \
  }
  INTERLEAVED_2_PLANAR_IPP(8u)
  INTERLEAVED_2_PLANAR_IPP(16s)
  INTERLEAVED_2_PLANAR_IPP(32s)
  INTERLEAVED_2_PLANAR_IPP(32f)
#undef INTERLEAVED_2_PLANAR_IPP

  // }}}

#endif // WITH_IPP_OPTINIZATION

  // {{{ explicit template instatiations for interleavedToPlanar and planarToInterleaved

#define EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,TYPEB)                            \
  template void planarToInterleaved<TYPEB,TYPEA>(const Img<TYPEB>*,TYPEA*,int);             \
  template void interleavedToPlanar<TYPEA,TYPEB>(const TYPEA*,Img<TYPEB>*,int)

  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl16s);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl32s);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl32f);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl64f);
#ifndef HAVE_IPP
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl8u);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl16s,icl16s);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32s,icl32s);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32f,icl32f);
#endif
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl64f,icl64f);

#undef EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION

  // }}}

  void convertYUV420ToRGB8(const unsigned char *pucSrc,const Size &s, Img8u *poDst){
    // {{{ open
#ifdef HAVE_IPP
    const icl8u *apucSrc[] = {pucSrc,pucSrc+s.getDim(), pucSrc+s.getDim()+s.getDim()/4};
    icl8u *apucDst[] = {poDst->getData(0),poDst->getData(1),poDst->getData(2)};
    ippiYUV420ToRGB_8u_P3(apucSrc,apucDst,s); 
#else
    
    // allocate memory for lookup tables
    static float fy_lut[256];
    static float fu_lut[256];
    static float fv_lut[256];
    static int r_lut[65536];
    static int b_lut[65536];
    static float g_lut1[65536];
    static float g_lut2[256];
    static int iInitedFlag=0;
    
    // initialize to lookup tables
    if(!iInitedFlag){
      float fy,fu,fv;
      for(int i=0;i<256;i++){
        fy_lut[i] = (255* (i - 16)) / 219;
        fu_lut[i] = (127 * (i - 128)) / 112;
        fv_lut[i] = (127 * (i - 128)) / 112;
      }
      
      for(int v=0;v<256;v++){
        g_lut2[v] = 0.714 * fv_lut[v];
      }
      
      for(int y=0;y<256;y++){
        fy = fy_lut[y];
        for(int u=0;u<256;u++){
          g_lut1[y+256*u] = fy - 0.344 * fu_lut[u];
        }
      }  
      
      for(int y=0;y<256;y++){
        fy = fy_lut[y];
        for(int v=0;v<256;v++){
          fv = fv_lut[v];
          r_lut[y+256*v]= (int)( fy + (1.402 * fv) );
          fu = fu_lut[v];
          b_lut[y+256*v]= (int)( fy + (1.772 * fu) ); 
        }
      }    
      iInitedFlag = 1;
    }
    
    // creating temporary pointer for fast data access
    int iW = s.width;
    int iH = s.height;
    
    icl8u *pucR = poDst->getData(0);
    icl8u *pucG = poDst->getData(1);
    icl8u *pucB = poDst->getData(2);
    const icl8u *ptY = pucSrc;
    const icl8u *ptU = ptY+iW*iH;
    const icl8u *ptV = ptU+(iW*iH)/4;
    
    register int r,g,b,y,u,v;
    
    register int Xflag=0;
    register int Yflag=1;
    register int w2 = iW/2;
    
    // converting the image (ptY,ptU,ptV)----->(pucR,pucG,pucB)
    for(int yy=iH-1; yy >=0 ; yy--){
      for(int xx=0; xx < iW; xx++){
        u=*ptU;
        v=*ptV;
        y=*ptY;
        
        r = r_lut[y+256*v];
        g = (int) ( g_lut1[y+256*u] - g_lut2[v]);
        b = b_lut[y+256*u];
        
        *pucR++=clip(r,0,255);
        *pucG++=clip(g,0,255);
        *pucB++=clip(b,0,255);
        
        if(Xflag++){
          ptV++;
          ptU++;
          Xflag=0;
        }
        ptY++;
      }
      if(Yflag++){
        ptU -= w2;
        ptV -= w2;
        Yflag = 0;
      }
    }
#endif
  }
  
  // }}}

}
