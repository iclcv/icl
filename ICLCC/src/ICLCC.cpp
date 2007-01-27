#include "ICLCC.h"
#include "Macros.h"
#include "ICLCore.h"
#include "Img.h"
#include <string>


namespace icl{
  
  using std::string;

#define AVL ccAvailable
#define IMU ccEmulated
#define UAV ccUnavailable 
#define IPS ccImpossible

  static const unsigned int NFMTS = 7;
  static const ccimpl g_aeAvailableTable[NFMTS*NFMTS] = {
    /*                      |-------------- dst format ------------------> */ 
    /*  ___                 gray   rgb    hls    yuv    lab  chroma matrix */
    /*   |        gray  */  AVL,   AVL,   AVL,   AVL,   AVL,   IPS,   IPS,   
    /*   |        rgb   */  AVL,   AVL,   AVL,   AVL,   AVL,   AVL,   IPS,   
    /*  src-      hls   */  AVL,   AVL,   AVL,   IMU,   IMU,   IMU,   IPS, 
    /* format     yuv   */  AVL,   AVL,   IMU,   AVL,   IMU,   IMU,   IPS, 
    /*   |        lab   */  AVL,   AVL,   IMU,   IMU,   AVL,   IMU,   IPS, 
    /*   |       chroma */  IPS,   IPS,   IPS,   IPS,   IPS,   AVL,   IPS, 
    /*   V       matrix */  IPS,   IPS,   IPS,   IPS,   IPS,   IPS,   IPS
  };
#undef AVL 
#undef IMU
#undef UAV
#undef IPS
  std::string translateCCImpl(ccimpl i){
    // {{{ open

    static string s_asNames[4] = { "available" , "emulated", "unavailable", "impossible" };
    return s_asNames[i];
  }

  // }}}

  ccimpl translateCCImlp(const std::string &s){
    // {{{ open

    if(!s.length()){
      ERROR_LOG("ccimpl \""<<s<<"\" is not defined"); 
      return ccUnavailable;
    }
    switch(s[0]){
      case 'a': return ccAvailable;
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
    
    a = 500.0 * (fX - fY);
    b = 200.0 * (fY - fZ);
    
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
    static void convert(const Img<S> *src, Img<D> *dst){
      (void)src; (void)dst;
    }
  };

  // }}}
  template<class S, class D, format srcDstFmt> struct CCFunc<S,D,srcDstFmt,srcDstFmt>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      src->deepCopy(dst);
    }
    
  };

  // }}}

  /// FROM FORMAT RGB
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      D *gr = dst->getData(0);
      for(int i=0;i<dim;++i){
        gr[i] = Cast<S,D>::cast((r[i]+g[i]+b[i])/3);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatHLS>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,h,l,s);
      register icl32f reg_h, reg_l, reg_s;
      for(int i=0;i<dim;++i){
        cc_util_rgb_to_hls(Cast<S,icl32f>::cast(r[i]),
                           Cast<S,icl32f>::cast(g[i]),
                           Cast<S,icl32f>::cast(b[i]),
                           reg_h,reg_l,reg_s);
        h[i] = Cast<icl32f,D>::cast(reg_h);
        l[i] = Cast<icl32f,D>::cast(reg_l);
        s[i] = Cast<icl32f,D>::cast(reg_s);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatChroma>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      GET_2_CHANNEL_POINTERS_NODIM(D,dst,cromaR,cromaG);
      register S sum;
      for(int i=0;i<dim;++i){
        sum = r[i]+g[i]+b[i];
        sum+=!sum; //avoid division by zero
        cromaR[i]=Cast<S,D>::cast((r[i]*255)/sum);
        cromaG[i]=Cast<S,D>::cast((g[i]*255)/sum);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatYUV>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,y,u,v);
      register icl32s reg_y, reg_u, reg_v;
      for(int i=0;i<dim;++i){ 
        cc_util_rgb_to_yuv(Cast<S,icl32s>::cast(r[i]),
                           Cast<S,icl32s>::cast(g[i]),
                           Cast<S,icl32s>::cast(b[i]),
                           reg_y, reg_u, reg_v);
        y[i] = Cast<icl32s,D>::cast(reg_y);
        u[i] = Cast<icl32s,D>::cast(reg_u);
        v[i] = Cast<icl32s,D>::cast(reg_v);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatLAB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,LL,aa,bb);
      
      register icl32f reg_X,reg_Y,reg_Z,reg_L, reg_a, reg_b;
      for(int i=0;i<dim;++i){ 
        cc_util_rgb_to_xyz(Cast<S,icl32f>::cast(r[i]),
                           Cast<S,icl32f>::cast(g[i]),
                           Cast<S,icl32f>::cast(b[i]),
                           reg_X,reg_Y,reg_Z);
        cc_util_xyz_to_lab(reg_X,reg_Y,reg_Z,reg_L,reg_a,reg_b);
        LL[i] = Cast<icl32f,D>::cast(reg_L);
        aa[i] = Cast<icl32f,D>::cast(reg_a);
        bb[i] = Cast<icl32f,D>::cast(reg_b);
      }
    }
    
  };

  // }}}


  /// FROM FORMAT GRAY
  template<class S, class D> struct CCFunc<S,D,formatGray,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      const S *gr = src->getData(0);
      GET_3_CHANNEL_POINTERS_DIM(D,dst,r,g,b,dim);
      for(int i=0;i<dim;++i){
        r[i] = g[i] = b[i] = Cast<S,D>::cast(gr[i]);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatHLS>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      const S *gr = src->getData(0);
      GET_3_CHANNEL_POINTERS_DIM(D,dst,h,l,s,dim);
      for(int i=0;i<dim;++i){
        h[i] = s[i] = D(0);
        l[i] = Cast<S,D>::cast(gr[i]);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatYUV>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      const S *gr = src->getData(0);
      GET_3_CHANNEL_POINTERS_DIM(D,dst,y,u,v,dim);
      for(int i=0;i<dim;++i){
        y[i] = Cast<S,D>::cast(gr[i]);
        u[i] = v[i] = D(127);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatLAB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      const S *gr = src->getData(0);
      GET_3_CHANNEL_POINTERS_DIM(D,dst,L,a,b,dim);
      for(int i=0;i<dim;++i){
        L[i] = Cast<S,D>::cast(gr[i]);
        a[i] = b[i] = D(127);
      }
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatChroma>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      WARNING_LOG("converting formatGray to formatChroma does not make sense");
      (void) src;
      GET_2_CHANNEL_POINTERS_DIM(D,dst,chromaR,chromaG,dim);
      std::fill(chromaR,chromaR+dim,D(85));
      std::fill(chromaG,chromaG+dim,D(85));
    }
    
  };

  // }}}
 
  /// FROM FORMAT HLS
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      icl::copy(src->getData(1),src->getData(1)+src->getDim(), dst->getData(0));
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(const S,src,h,l,s,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,r,g,b);
      register icl32f reg_r(0), reg_g(0), reg_b(0);
      for(int i=0;i<dim;++i){
        cc_util_hls_to_rgb(Cast<S,icl32f>::cast(h[i]),
                           Cast<S,icl32f>::cast(l[i]),
                           Cast<S,icl32f>::cast(s[i]),
                           reg_r,reg_g,reg_b);
        r[i] = Cast<icl32f,D>::cast(reg_r);
        g[i] = Cast<icl32f,D>::cast(reg_g);
        b[i] = Cast<icl32f,D>::cast(reg_b);
      }
    }
    
  };

  // }}}

  /// FROM FORMAT LAB
  template<class S, class D> struct CCFunc<S,D,formatLAB,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      icl::copy(src->getData(0),src->getData(0)+src->getDim(), dst->getData(0));
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatLAB,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(const S,src,ll,aa,bb,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,r,g,b);
      register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
      for(int i=0;i<dim;++i){
        cc_util_lab_to_xyz(Cast<S,icl32f>::cast(ll[i]),
                           Cast<S,icl32f>::cast(aa[i]),
                           Cast<S,icl32f>::cast(bb[i]),
                           reg_x,reg_y,reg_z);
        cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
        r[i] = Cast<icl32f,D>::cast(reg_r);
        g[i] = Cast<icl32f,D>::cast(reg_g);
        b[i] = Cast<icl32f,D>::cast(reg_b);
      }
    }
    
  };

  // }}}


  /// FROM FORMAT YUV
  template<class S, class D> struct CCFunc<S,D,formatYUV,formatGray>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      icl::copy(src->getData(0),src->getData(0)+src->getDim(), dst->getData(0));
    }
    
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatYUV,formatRGB>{
    // {{{ open
    static void convert(const Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(const S,src,y,u,v,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,r,g,b);
      register icl32s reg_r, reg_g, reg_b;
      for(int i=0;i<dim;++i){
        cc_util_yuv_to_rgb(Cast<S,icl32s>::cast(y[i]),
                           Cast<S,icl32s>::cast(u[i]),
                           Cast<S,icl32s>::cast(v[i]),
                           reg_r, reg_g, reg_b);
        r[i] = Cast<icl32s,D>::cast(reg_r);
        g[i] = Cast<icl32s,D>::cast(reg_g);
        b[i] = Cast<icl32s,D>::cast(reg_b);
      }
    }
    
  };

  // }}}

  template<class S, class D> void cc_sd(const Img<S> *src, Img<D> *dst){
  // {{{ open

    // {{{ definition of CASE_LABEL(XXX)
    
#define INNER_CASE_LABEL(XXX,YYY) \
  case format##YYY: CCFunc<S,D,format##XXX,format##YYY>::convert(src,dst); break
    
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
 
  template<class S> void cc_s(const Img<S> *src, ImgBase *dst){
    // {{{ open

    switch(dst->getDepth()){      //TODO depth macro
      case depth8u: cc_sd(src, dst->asImg<icl8u>()); break;
      case depth16s: cc_sd(src, dst->asImg<icl16s>()); break;
      case depth32s: cc_sd(src, dst->asImg<icl32s>()); break;
      case depth32f: cc_sd(src, dst->asImg<icl32f>()); break;
      case depth64f: cc_sd(src, dst->asImg<icl64f>()); break;
      default:
        ICL_INVALID_DEPTH;
    }
  }

  // }}}

  void cc(const ImgBase *src, ImgBase *dst){
    // {{{ open

    ICLASSERT_RETURN( src );
    ICLASSERT_RETURN( dst );
    dst->setSize( src->getSize() );
    
    switch(cc_available(src->getFormat(), dst->getFormat())){
      case ccAvailable:
        switch(src->getDepth()){ //TODO depth macro
          case depth8u: cc_s(src->asImg<icl8u>(),dst); break;
          case depth16s: cc_s(src->asImg<icl16s>(),dst); break;
          case depth32s: cc_s(src->asImg<icl32s>(),dst); break;
          case depth32f: cc_s(src->asImg<icl32f>(),dst); break;
          case depth64f: cc_s(src->asImg<icl64f>(),dst); break;
          default:
            ICL_INVALID_DEPTH;
        }
        break;
      case ccEmulated:{
        ImgBase *buf=imgNew(src->getDepth(), src->getSize(), formatRGB);
        cc(src,buf);
        cc(buf,dst);
        delete buf;
        break;
      }
      case ccUnavailable:{
        ERROR_LOG("no color conversion [" << 
                  translateFormat(src->getFormat()) << 
                  "-->" << 
                  translateFormat(dst->getFormat()) << 
                  "] (depth:" <<
                  translateDepth(src->getDepth()) <<
                  "-->" << 
                  translateDepth(dst->getDepth()) <<
                  ") available!" );
      }
      case ccImpossible:{
        ERROR_LOG("color conversion [" << 
                  translateFormat(src->getFormat()) << 
                  "-->" << 
                  translateFormat(dst->getFormat()) << 
                  "] (depth:" <<
                  translateDepth(src->getDepth()) <<
                  "-->" << 
                  translateDepth(dst->getDepth()) <<
                  ") impossible!" );
      
      }
        
    }

  }

  // }}}


  /// additional misc
  template<class S, class D>
  inline void planarToInterleaved_Generic(const Img<S> *src, D* dst){
    // {{{ open
  FUNCTION_LOG("");
  ICLASSERT_RETURN(src);
  ICLASSERT_RETURN(dst);


  int c=src->getChannels();
  if(c==1){
    copy<S,D>(src->getData(0),src->getData(0)+src->getDim(),dst);
    return;
  }
  int dim=src->getDim();
  const S** pp=new const S* [c];
  const S** ppEnd=pp+c;
  
  for (int i=0;i<c;i++){
    pp[i]=src->getData(i);
  }
  
  D* dstEnd=dst+c*dim;
  while (dst<dstEnd){
    for (const S** p=pp;p<ppEnd;++(*p),++p,++dst ){
      *dst=Cast<S,D>::cast(*(*p));
    }
  }
  delete [] pp;
}
  
  // }}}
  
  template<class S, class D>
  inline void interleavedToPlanar_Generic(const S *src,const Size &srcSize, int c,  Img<D> *dst){
    // {{{ open
  
  FUNCTION_LOG("");
  ICLASSERT_RETURN(src);
  ICLASSERT_RETURN(dst);
  dst->setChannels(c);
  dst->setSize(srcSize);
  if(c==1){
    copy<S,D>(src,src+srcSize.getDim(),dst->getData(0));
    return;
  }    
  D** pp=new D* [c];
  D** ppEnd=pp+c;
  for (int i=0;i<c;i++){
    pp[i]=dst->getData(i);
  }
  const S* srcEnd=src+srcSize.getDim()*c;
  while (src<srcEnd){
    for (D** p=pp;p<ppEnd;++(*p),++p,++src){
      *(*p)= Cast<S,D>::cast(*src);
    }
  }
  delete [] pp;
}
  
  // }}}
  
  template<class S,class D>
  void planarToInterleaved(const Img<S> *src, D* dst){
    planarToInterleaved_Generic(src,dst);
  }
  
  template<class S, class D>
  void interleavedToPlanar(const S *src,const Size &srcSize, int c,  Img<D> *dst){
    interleavedToPlanar_Generic(src,srcSize,c,dst);
  }
  
#ifdef WITH_IPP_OPTIMIZATION

  // {{{ PLANAR_2_INTERLEAVED_IPP

#define PLANAR_2_INTERLEAVED_IPP(DEPTH)                                                                       \
  template<> void planarToInterleaved(const Img<icl##DEPTH>*src, icl##DEPTH *dst){                            \
    ICLASSERT_RETURN( src );                                                                                  \
    ICLASSERT_RETURN( dst );                                                                                  \
    ICLASSERT_RETURN( src->getChannels() );                                                                   \
    switch(src->getChannels()){                                                                               \
      case 3: {                                                                                               \
        const icl##DEPTH* apucChannels[3]={src->getData(0),src->getData(1),src->getData(2)};                        \
        ippiCopy_##DEPTH##_P3C3R(apucChannels,src->getLineStep(),dst,src->getLineStep()*3,src->getSize());    \
        break;                                                                                                \
      }                                                                                                       \
      case 4: {                                                                                               \
        const icl##DEPTH* apucChannels[4]={src->getData(0),src->getData(1),src->getData(2),src->getData(3)};        \
        ippiCopy_##DEPTH##_P4C4R(apucChannels,src->getLineStep(),dst,src->getLineStep()*4,src->getSize());    \
        break;                                                                                                \
      }                                                                                                       \
      default:                                                                                                \
        planarToInterleaved_Generic(src,dst);                                                                 \
        break;                                                                                                \
    }                                                                                                         \
  }
  PLANAR_2_INTERLEAVED_IPP(8u)
  PLANAR_2_INTERLEAVED_IPP(16s)
  PLANAR_2_INTERLEAVED_IPP(32s)
  PLANAR_2_INTERLEAVED_IPP(32f)
#undef PLANAR_2_INTERLEAVED_IPP

  // }}}

  // {{{ INTERLEAVED_2_PLANAR_IPP

#define INTERLEAVED_2_PLANAR_IPP(DEPTH)                                                                                         \
  template<> void interleavedToPlanar(const icl##DEPTH *src, const Size &srcSize, int srcChannels,  Img<icl##DEPTH> *dst){      \
    ICLASSERT_RETURN( src );                                                                                                    \
    ICLASSERT_RETURN( dst );                                                                                                    \
    ICLASSERT_RETURN( srcChannels );                                                                                            \
    dst->setChannels( srcChannels );                                                                                            \
    switch(srcChannels){                                                                                                        \
      case 3: {                                                                                                                 \
        icl##DEPTH* apucChannels[3]={dst->getData(0),dst->getData(1),dst->getData(2)};                                          \
        ippiCopy_##DEPTH##_C3P3R(src,srcSize.width*srcChannels,apucChannels,srcSize.width,srcSize);                             \
        break;                                                                                                                  \
      }                                                                                                                         \
      case 4: {                                                                                                                 \
        icl##DEPTH* apucChannels[4]={dst->getData(0),dst->getData(1),dst->getData(2),dst->getData(3)};                          \
        ippiCopy_##DEPTH##_C4P4R(src,srcSize.width*srcChannels,apucChannels,srcSize.width,srcSize);                             \
        break;                                                                                                                  \
      }                                                                                                                         \
      default:                                                                                                                  \
        interleavedToPlanar_Generic(src,srcSize,srcChannels,dst);                                                               \
        break;                                                                                                                  \
    }                                                                                                                           \
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
  template void planarToInterleaved<TYPEA,TYPEB>(const Img<TYPEA>*,TYPEB*);                 \
  template void interleavedToPlanar<TYPEA,TYPEB>(const TYPEA*,const Size&,int,Img<TYPEB>*)

#define EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(TYPEA)  \
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl8u);               \
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl16s);              \
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl32s);              \
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl32f);              \
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl64f)
  
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl8u);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl16s);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl32s);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl32f);
  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl64f);

#undef EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION
#undef EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB

  // }}}

  void convertYUV420ToRGB8(const unsigned char *pucSrc,const Size &s, Img8u *poDst){
    // {{{ open
#ifdef WITH_IPP_OPTIMIZATION
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
        
#define LIMIT(x) (x)>255?255:(x)<0?0:(x);
        *pucR++=LIMIT(r);
        *pucG++=LIMIT(g);
        *pucB++=LIMIT(b);
#undef LIMIT
        
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



/** old misc:


void convertToARGB32Interleaved(unsigned char *pucDst, Img8u *poSrc){
  if(!poSrc || poSrc->getChannels() != 4 ) return;
#ifdef WITH_IPP_OPTIMIZATION
  icl8u* apucChannels[4]={poSrc->getData(0),poSrc->getData(1),poSrc->getData(2),poSrc->getData(3)};
  ippiCopy_8u_P4C4R(apucChannels,poSrc->getLineStep(),pucDst,poSrc->getLineStep()*4,poSrc->getSize());
#else
  printf("c++ fallback for convertToRGB8Interleaved(unsigned char *pucDst, Img8u *poSrc) not yet implemented \n");
#endif
} 


void convertToARGB32Interleaved(unsigned char *pucDst, Img32f *poSrc, Img8u *poBuffer){
  if(!poSrc || poSrc->getChannels() != 4 ) return;
#ifdef WITH_IPP_OPTIMIZATION
  poSrc->convertTo<icl8u>(poBuffer);
  icl8u* apucChannels[4]={poBuffer->getData(0),poBuffer->getData(1),poBuffer->getData(2),poBuffer->getData(3)};
  ippiCopy_8u_P4C4R(apucChannels,poBuffer->getLineStep(),pucDst,poBuffer->getLineStep()*4,poBuffer->getSize());
#else
  printf("c++ fallback for convertToRGB8Interleaved(unsigned char *pucDst,"
         " Img32f *poSrc, Img8u* poBuffer) not yet implemented \n");
#endif
} 

void cc_util_yuv_to_rgb(const icl8u y, const icl8u u, const icl8u v, icl8u &r, icl8u &g, icl8u &b){
  register int buf;
  buf = g_a32s_yuv_r_lut[y+256*v];
  r = buf < 0 ? 0 : buf > 255 ? 255 : buf; //optimize clipped lut
   
  buf = (int) ( g_a32f_yuv_g_lut1[y+256*u] - g_a32f_yuv_g_lut2[v]);
  g = buf < 0 ? 0 : buf > 255 ? 255 : buf;
    
  buf = g_a32s_yuv_b_lut[y+256*u];
  b = buf < 0 ? 0 : buf > 255 ? 255 : buf;
}

void yuv3rgb(icl32f y,icl32f u,icl32f v, icl32f &r, icl32f &g, icl32f &b){
  // y range 0..255
  // u range +-0.436
  // v range +-0.615

  icl32f u2 = 0.0034196078*u - 0.436;
  icl32f v2 = 0.0048235294*v - 0.615;

  r = y +               290.7   * v2;
  g = y - 100.47 * u2 - 148.155 * v2;
  b = y + 518.16 * u2;
} 
void rgb2yuv(icl32f r, icl32f g, icl32f b, icl32f &y, icl32f &u, icl32f &v){
  y = (0.299*r + 0.587*g + 0.114*b);  
  u = 0.56433408*(b-y) + 127.5; 
  v = 0.71326676*(r-y) + 127.5; 
}

*/
