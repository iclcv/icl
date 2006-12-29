#include "ICLCC.h"
#include "Macros.h"
#include "ICLCore.h"
#include "Img.h"
#include <string>


namespace icl{
  
  using std::string;

  /// for rgb to hls conversion
  static unsigned char s_a8u_hls_lut[257];
  void utility_init_hls_lut(){
    // {{{ open

    static bool inited = false;
    if(inited) return;
    for(int i=0 ; i<256 ; i++){
      s_a8u_hls_lut[i] = static_cast<icl8u>(acos(sqrt((float)i/256.0)) * 2 * 63 / M_PI);
    }
    s_a8u_hls_lut[256]=s_a8u_hls_lut[255];
    inited = true;
  }

  // }}}
  inline void utility_get_min_and_max(const icl8u r,const icl8u g,const icl8u b,icl8u &min,icl8u &max){
    // {{{ open
    if(r<g) {
      min=r;
      max=g;
    }
    else {
      min=g;
      max=r;	
    }
    if(b<min)
      min=b;
    else {
      max = b>max ? b : max;
    }
  }
  
  // }}}
  void utility_rgb_to_hls(const icl8u r,const icl8u g,const icl8u b, icl8u &h, icl8u &l, icl8u &s){
    // {{{ open
    static int RG,RB,nom,denom, cos2Hx256;
    static icl8u min;
    utility_get_min_and_max(r, g, b,min,l);
    s = l>0 ? 255-((255*min)/l) : 0;
  
    //Hue
    RG = r - g;
    RB = r - b;
    nom = (RG+RB)*(RG+RB);
    denom = (RG*RG+RB*(g-b))<<2;
  
    cos2Hx256 = denom>0 ? (nom<<8)/denom : 0;
    h=s_a8u_hls_lut[cos2Hx256];
    if ((RG+RB)<0) h=127-h;
    if (b>g) h=255-h;
  }

  // }}}
  void utility_rgb_to_xyz(const icl32f r, const icl32f g, const icl32f b, icl32f &X, icl32f &Y, icl32f &Z){
    // {{{ open
    static icl32f m[3][3] = {{ 0.412453, 0.35758 , 0.180423},
                             { 0.212671, 0.71516 , 0.072169},
                             { 0.019334, 0.119193, 0.950227}};
    X = m[0][0] * r + m[0][1] * g + m[0][2] * b;
    Y = m[1][0] * r + m[1][1] * g + m[1][2] * b;
    Z = m[2][0] * r + m[2][1] * g + m[2][2] * b;
  }
  // }}}
  void utility_xyz_to_lab(const icl32f X, const icl32f Y, const icl32f Z, icl32f &L, icl32f &a, icl32f &b){
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
  void utility_hls_to_rgb(const icl32f h255, const icl32f l255, const icl32f sl255, icl32f &r, icl32f &g, icl32f &b){
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
  void utility_rgb_to_yuv(const icl32f r, const icl32f g, const icl32f b, icl32f &y, icl32f &u, icl32f &v){
    // {{{ open

    y = 0.299*r + 0.587*g + 0.114*b;  
    u = 0.493*(b-y);
    v = 0.877*(r-y);
  }

  // }}}
  void utility_rgb_to_chroma(const icl32f r, const icl32f g, const icl32f b, icl32f &chromaR, icl32f &chromaG){
    // {{{ open

    icl32f sum = r+g+b;
    sum+=!sum; //avoid division by zero
    chromaR=r*255/sum;
    chromaG=g*255/sum;
  }

  // }}}
  void utitlity_lab_to_xyz(const icl32f l, const icl32f a, const icl32f b, icl32f &x, icl32f &y, icl32f &z){
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
  void utility_xyz_to_rgb(const icl32f x, const icl32f y, const icl32f z, icl32f &r, icl32f &g, icl32f &b){
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
    static void convert(Img<S> *src, Img<D> *dst){
      (void)src; (void)dst;
      ERROR_LOG("no color conversion with source depth "<< translateDepth(getDepth<S>()) <<
                " and destination depth "<< translateDepth(getDepth<D>()) << " from "<< 
                translateFormat(srcFmt) << " to " << translateFormat(dstFmt) << " available!");
    }
  };

  // }}}
  template<class S, class D, format srcDstFmt> struct CCFunc<S,D,srcDstFmt,srcDstFmt>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      src->deepCopy(dst);
    }
  };

  // }}}
  template<class S, class D, format dstFmt> struct CCFunc<S,D,formatMatrix,dstFmt>{
    // {{{ open

    static void convert(Img<S> *src, Img<D> *dst){
      (void)src; (void)dst;
      ERROR_LOG("no color conversion with destination format formatMatrix allowed!");
    }
  };

  // }}}
  template<class S, class D, format srcFmt> struct CCFunc<S,D,srcFmt,formatMatrix>{
    // {{{ open

    static void convert(Img<S> *src, Img<D> *dst){
      (void)src; (void)dst;
      ERROR_LOG("no color conversion with source format formatMatrix allowed!");
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatMatrix,formatMatrix>{
    // {{{ open

    static void convert(Img<S> *src, Img<D> *dst){
      (void)src; (void)dst;
      ERROR_LOG("no color conversion with source format formatMatrix allowed!");
    }
  };

  // }}}

  /// FROM FORMAT RGB
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatGray>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(S,src,r,g,b,dim);
      D *gr = dst->getData(0);
      for(int i=0;i<dim;++i){
        gr[i] = Cast<S,D>::cast((r[i]+g[i]+b[i])/3);
      }
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatHLS>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,h,l,s);
      register icl8u reg_h, reg_l, reg_s;
      for(int i=0;i<dim;++i){
        utility_rgb_to_hls(Cast<S,icl8u>::cast(r[i]),
                           Cast<S,icl8u>::cast(g[i]),
                           Cast<S,icl8u>::cast(b[i]),
                           reg_h,reg_l,reg_s);
        h[i] = Cast<icl8u,D>::cast(reg_h);
        l[i] = Cast<icl8u,D>::cast(reg_l);
        s[i] = Cast<icl8u,D>::cast(reg_s);
      }
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatChroma>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      GET_3_CHANNEL_POINTERS_DIM(S,src,r,g,b,dim);
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
    static void convert(Img<S> *src, Img<D> *dst){
      GET_3_CHANNEL_POINTERS_DIM(S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,y,u,v);
      
      register icl32f reg_y;
      for(int i=0;i<dim;++i){ 
        reg_y = 0.299*r[i] + 0.587*g[i] + 0.114*b[i];  
        y[i] = Cast<icl32f,D>::cast(reg_y);
        u[i] = Cast<icl32f,D>::cast(0.493*(Cast<S,icl32f>::cast(b[i])-reg_y));
        v[i] = Cast<icl32f,D>::cast(0.877*(Cast<S,icl32f>::cast(r[i])-reg_y));
      }
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatRGB,formatLAB>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      GET_3_CHANNEL_POINTERS_DIM(S,src,r,g,b,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,LL,aa,bb);
      
      register icl32f reg_X,reg_Y,reg_Z,reg_L, reg_a, reg_b;
      for(int i=0;i<dim;++i){ 
        utility_rgb_to_xyz(Cast<S,icl32f>::cast(r[i]),
                           Cast<S,icl32f>::cast(g[i]),
                           Cast<S,icl32f>::cast(b[i]),
                           reg_X,reg_Y,reg_Z);
        utility_xyz_to_lab(reg_X,reg_Y,reg_Z,reg_L,reg_a,reg_b);
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
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      S *gr = src->getData(0);
      GET_3_CHANNEL_POINTERS_DIM(D,dst,r,g,b,dim);
      for(int i=0;i<dim;++i){
        r[i] = g[i] = b[i] = Cast<S,D>::cast(gr[i]);
      }
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatGray,formatHLS>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      S *gr = src->getData(0);
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
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      S *gr = src->getData(0);
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
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      S *gr = src->getData(0);
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
    static void convert(Img<S> *src, Img<D> *dst){
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
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      icl::copy(src->getData(1),src->getData(1)+src->getDim(), dst->getData(0));
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatRGB>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(S,src,h,l,s,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,r,g,b);
      register icl32f reg_r, reg_g, reg_b;
      for(int i=0;i<dim;++i){
        utility_hls_to_rgb(Cast<S,icl32f>::cast(h[i]),
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
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatYUV>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(S,src,h,l,s,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,y,u,v);
      register icl32f reg_r, reg_g, reg_b,reg_y,reg_u,reg_v;
      for(int i=0;i<dim;++i){
        utility_hls_to_rgb(Cast<S,icl32f>::cast(h[i]),
                           Cast<S,icl32f>::cast(l[i]),
                           Cast<S,icl32f>::cast(s[i]),
                           reg_r,reg_g,reg_b);
        utility_rgb_to_yuv(reg_r,reg_g,reg_b,reg_y,reg_u,reg_v);
        y[i] = Cast<icl32f,D>::cast(reg_y);
        u[i] = Cast<icl32f,D>::cast(reg_u);
        v[i] = Cast<icl32f,D>::cast(reg_v);
      }
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatLAB>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(S,src,h,l,s,dim);
      GET_3_CHANNEL_POINTERS_NODIM(D,dst,ll,aa,bb);
      register icl32f reg_r, reg_g, reg_b, reg_ll,reg_aa,reg_bb,reg_x, reg_y, reg_z;
      for(int i=0;i<dim;++i){
        utility_hls_to_rgb(Cast<S,icl32f>::cast(h[i]),
                           Cast<S,icl32f>::cast(l[i]),
                           Cast<S,icl32f>::cast(s[i]),
                           reg_r,reg_g,reg_b);
        utility_rgb_to_xyz(reg_r, reg_g, reg_b,reg_x, reg_y, reg_z);
        utility_xyz_to_lab(reg_x,reg_y,reg_z,reg_ll,reg_aa,reg_bb);
        ll[i] = Cast<icl32f,D>::cast(reg_ll);
        aa[i] = Cast<icl32f,D>::cast(reg_aa);
        bb[i] = Cast<icl32f,D>::cast(reg_bb);
      }
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatHLS,formatChroma>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      FUNCTION_LOG("");
      GET_3_CHANNEL_POINTERS_DIM(S,src,h,l,s,dim);
      GET_2_CHANNEL_POINTERS_NODIM(D,dst,chromaR,chromaG);
      register icl32f reg_r, reg_g, reg_b, reg_chromaR, reg_chromaG;
      for(int i=0;i<dim;++i){
        utility_hls_to_rgb(Cast<S,icl32f>::cast(h[i]),
                           Cast<S,icl32f>::cast(l[i]),
                           Cast<S,icl32f>::cast(s[i]),
                           reg_r,reg_g,reg_b);
        utility_rgb_to_chroma(reg_r,reg_g,reg_b,reg_chromaR, reg_chromaG);
        chromaR[i] = Cast<icl32f,D>::cast(reg_chromaR);
        chromaG[i] = Cast<icl32f,D>::cast(reg_chromaG);
      }
    }
  };

  // }}}
  
  /// FROM FORMAT CHROMA
  template<class S, class D> struct CCFunc<S,D,formatChroma,formatGray>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      (void) src; (void)dst;
      ERROR_LOG("converting from formatChroma is not possible !");
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatChroma,formatRGB>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      (void) src; (void)dst;
      ERROR_LOG("converting from formatChroma is not possible !");
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatChroma,formatHLS>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      (void) src; (void)dst;
      ERROR_LOG("converting from formatChroma is not possible !");
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatChroma,formatYUV>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      (void) src; (void)dst;
      ERROR_LOG("converting from formatChroma is not possible !");
    }
  };

  // }}}
  template<class S, class D> struct CCFunc<S,D,formatChroma,formatLAB>{
    // {{{ open
    static void convert(Img<S> *src, Img<D> *dst){
      (void) src; (void)dst;
      ERROR_LOG("converting from formatChroma is not possible !");
    }
  };

  // }}}

  
  /// FROM FORMAT LAB

  
  
  template<class S, class D> void cc_sd(Img<S> *src, Img<D> *dst){
    // {{{ open

    // {{{ definition of CASE_LABEL(XXX)

#define CASE_LABEL(XXX)                                                                     \
      case format##XXX:                                                                     \
        switch(dst->getFormat()){                                                           \
          case formatRGB: CCFunc<S,D,format##XXX,formatRGB>::convert(src,dst); break;       \
          case formatHLS: CCFunc<S,D,format##XXX,formatHLS>::convert(src,dst); break;       \
          case formatLAB: CCFunc<S,D,format##XXX,formatLAB>::convert(src,dst); break;       \
          case formatYUV: CCFunc<S,D,format##XXX,formatYUV>::convert(src,dst); break;       \
          case formatGray: CCFunc<S,D,format##XXX,formatGray>::convert(src,dst); break;     \
          case formatMatrix: CCFunc<S,D,format##XXX,formatMatrix>::convert(src,dst); break; \
          case formatChroma: CCFunc<S,D,format##XXX,formatChroma>::convert(src,dst); break; \
          default:                                                                          \
            ICL_INVALID_FORMAT;                                                             \
        }                                                                                   \
        break

    // }}}
    
    switch(src->getFormat()){
      CASE_LABEL(RGB);
      CASE_LABEL(HLS);
      CASE_LABEL(YUV);
      CASE_LABEL(Gray);
      CASE_LABEL(Matrix);
      CASE_LABEL(Chroma);
      default:
        ICL_INVALID_FORMAT;
    }
#undef CASE_LABEL                                          
  }  

  // }}}
  template<class S> void cc_s(Img<S> *src, ImgBase *dst){
    // {{{ open
#if __GNUC__ == 3
#warning "gcc 3 does not support asImg<T>()"
#else
    switch(dst->getDepth()){
      case depth8u:  cc_sd(src, dst->asImg<icl8u>()); break;
      case depth16s: cc_sd(src, dst->asImg<icl16s>()); break;
      case depth32s: cc_sd(src, dst->asImg<icl32s>()); break;
      case depth32f: cc_sd(src, dst->asImg<icl32f>()); break;
      case depth64f: cc_sd(src, dst->asImg<icl64f>()); break;
      default:
        ICL_INVALID_DEPTH;
    }
#endif
  }

  // }}}
  void cc(ImgBase *src, ImgBase *dst){
    // {{{ open

    ICLASSERT_RETURN( src );
    ICLASSERT_RETURN( dst );
    switch(src->getDepth()){
      case depth8u: cc_s(src->asImg<icl8u>(),dst); break;
      case depth16s: cc_s(src->asImg<icl16s>(),dst); break;
      case depth32s: cc_s(src->asImg<icl32s>(),dst); break;
      case depth32f: cc_s(src->asImg<icl32f>(),dst); break;
      case depth64f: cc_s(src->asImg<icl64f>(),dst); break;
      default:
        ICL_INVALID_DEPTH;
    }
  }

  // }}}

}
