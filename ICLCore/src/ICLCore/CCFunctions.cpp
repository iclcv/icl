/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/CCFunctions.cpp                    **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Michael Goetting, Robert Haschke  **
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

#include <ICLCore/CCFunctions.h>
#include <ICLCore/Img.h>
#include <map>
#include <ICLCore/CCLUT.h>
#include <ICLUtils/SSEUtils.h>

using namespace icl::utils;

namespace icl{
  namespace core{
    
    using std::string;
  
  #define AVL ccAvailable
  #define IMU ccEmulated
  #define UAV ccUnavailable 
  #define IPS ccImpossible
  #define ADP ccAdapted

    static const unsigned int NFMTS = 7;
  #ifdef ICL_HAVE_SSE2
    static const ccimpl g_aeAvailableTable[NFMTS*NFMTS] = {
      /*                      |-------------- dst format ------------------> */ 
      /*  ___                 gray   rgb    hls    yuv    lab  chroma matrix */
      /*   |        gray  */  AVL,   AVL,   AVL,   AVL,   AVL,   IPS,  ADP,   
      /*   |        rgb   */  AVL,   AVL,   AVL,   AVL,   AVL,   AVL,  ADP,   
      /*  src-      hls   */  AVL,   AVL,   AVL,   AVL,   AVL,   IMU,  ADP, 
      /* format     yuv   */  AVL,   AVL,   AVL,   AVL,   AVL,   IMU,  ADP, 
      /*   |        lab   */  AVL,   AVL,   AVL,   AVL,   AVL,   IMU,  ADP, 
      /*   |       chroma */  IPS,   IPS,   IPS,   IPS,   IPS,   AVL,  ADP, 
      /*   V       matrix */  ADP,   ADP,   ADP,   ADP,   ADP,   ADP,  ADP
    };
  #else
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
  #endif
  
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
  #ifdef USE_RGB_TO_YUV_FULL_RANGE_UV
      /// fixed point approximation of the the rgb2yuv version :
      /** we no longer use this version due to compatibility issues with IPP based code */
      y = ( 1254097*r + 2462056*g + 478151*b ) >> 22;  
      u = ( 2366989*(b-y) + 534773760        ) >> 22;
      v = ( 2991658*(r-y) + 534773760        ) >> 22;
  #else
      /// this is the way Intel IPP does rgb-to-yuv conversion
      y = ( 1254097*r + 2462056*g + 478151*b ) >> 22;  
      u = (2063598*(b-y) >> 22) + 128;
      v = (3678405*(r-y) >> 22) + 128; 
      if(v<0) v=0;
      else if(v > 255) v = 255;
  #endif
    }
    // }}}

    inline void cc_util_rgb_to_yuv_inline(const icl32s r, const icl32s g, const icl32s b, icl32s &y, icl32s &u, icl32s &v){
      // {{{ integer open
  #ifdef USE_RGB_TO_YUV_FULL_RANGE_UV
      /// fixed point approximation of the the rgb2yuv version :
      /** we no longer use this version due to compatibility issues with IPP based code */
      y = ( 1254097*r + 2462056*g + 478151*b ) >> 22;  
      u = ( 2366989*(b-y) + 534773760        ) >> 22;
      v = ( 2991658*(r-y) + 534773760        ) >> 22;
  #else
      /// this is the way Intel IPP does rgb-to-yuv conversion
      y = ( 1254097*r + 2462056*g + 478151*b ) >> 22;  
      u = (2063598*(b-y) >> 22) + 128;
      v = (3678405*(r-y) >> 22) + 128; 
      if(v<0) v=0;
      else if(v > 255) v = 255;
  #endif
    }
    // }}}

    inline void cc_util_rgb_to_yuv(const icl32f r, const icl32f g, const icl32f b, icl32f &y, icl32f &u, icl32f &v){
      /// this is the way Intel IPP does rgb-to-yuv conversion
      y = 0.299f*r + 0.587f*g + 0.114*b;
      u = 0.492f*(b-y) + 128.0f;
      v = 0.877f*(r-y) + 128.0f; 
      if(v < 0.0f) v = 0.0f;
      else if(v > 255.0f) v = 255.0f;
    }

    void cc_util_yuv_to_rgb(const icl32s y,const icl32s u,const icl32s v, icl32s &r, icl32s &g, icl32s &b){
      // {{{ open
  #ifdef USE_RGB_TO_YUV_FULL_RANGE_UV
      icl32s u2 = 14343*u - 1828717; 
      icl32s v2 = 20231*v - 2579497; 
      
      r = y +  ( ( 290 * v2 ) >> 22 );
      g = y -  ( ( 100  * u2 + 148 * v2) >> 22 );
      b = y +  ( ( 518 * u2 ) >> 22 );
  #else
      // ipp compatible version (please note, due to the clipping process of 'v' in rgb_to_yuv,
      // this method cannot restore an original rgb value completetly. Since we lost some information
      // in v, the resulting r and g values are differ as follows: r-r' in [-32,35], and g-g' in [-17,18]
      icl32s u2 = u-128;
      icl32s v2 = v-128;
      icl32s y2 = y<<22;
      
      r = (y2 + 4781506 * v2 ) >> 22;
      g = (y2 - 1652556 * u2 - 2436891 *v2 ) >> 22;
      b = (y2 + 8522826 * u2 ) >> 22;
      
      r = utils::clip(r,0,255);
      g = utils::clip(g,0,255);
      b = utils::clip(b,0,255);
  #endif
    } 
    // }}}

    inline void cc_util_yuv_to_rgb_inline(const icl32s y,const icl32s u,const icl32s v, icl32s &r, icl32s &g, icl32s &b){
      // {{{ open
  #ifdef USE_RGB_TO_YUV_FULL_RANGE_UV
      icl32s u2 = 14343*u - 1828717; 
      icl32s v2 = 20231*v - 2579497; 
      
      r = y +  ( ( 290 * v2 ) >> 22 );
      g = y -  ( ( 100  * u2 + 148 * v2) >> 22 );
      b = y +  ( ( 518 * u2 ) >> 22 );
  #else
      // ipp compatible version (please note, due to the clipping process of 'v' in rgb_to_yuv,
      // this method cannot restore an original rgb value completetly. Since we lost some information
      // in v, the resulting r and g values are differ as follows: r-r' in [-32,35], and g-g' in [-17,18]
      icl32s u2 = u-128;
      icl32s v2 = v-128;
      icl32s y2 = y<<22;
      
      r = (y2 + 4781506 * v2 ) >> 22;
      g = (y2 - 1652556 * u2 - 2436891 *v2 ) >> 22;
      b = (y2 + 8522826 * u2 ) >> 22;
      
      r = utils::clip(r,0,255);
      g = utils::clip(g,0,255);
      b = utils::clip(b,0,255);
  #endif
    } 
    // }}}

    inline void cc_util_yuv_to_rgb(const icl32f y,const icl32f u,const icl32f v, icl32f &r, icl32f &g, icl32f &b){
      // {{{ open
      // ipp compatible version using 32f values
      icl32f u2 = u-128.0f;
      icl32f v2 = v-128.0f;
      
      r = (y + 1.140f * v2 );
      g = (y - 0.394f * u2 - 0.581f * v2 );
      b = (y + 2.032f * u2 );
      
      r = utils::clip(r,0.0f,255.0f);
      g = utils::clip(g,0.0f,255.0f);
      b = utils::clip(b,0.0f,255.0f);
    }

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

    inline void cc_util_rgb_to_hls_inline(const icl32f r255,const icl32f g255,const icl32f b255, icl32f &h, icl32f &l, icl32f &s){
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

    inline void cc_util_rgb_to_xyz_inline(const icl32f r, const icl32f g, const icl32f b, icl32f &X, icl32f &Y, icl32f &Z){
      // {{{ open
      /*
      icl32f fR = r / 255.0f;
      icl32f fG = g / 255.0f;
      icl32f fB = b / 255.0f;
      */
      // sRGB gamma correction
      // maybe 0.03928 instead of 0.04045
      /*if (fR > 0.04045) fR = pow(((fR + 0.055) / 1.055), 2.4f);
      else fR = fR / 12.92f;
      if (fG > 0.04045) fG = pow(((fG + 0.055) / 1.055), 2.4f);
      else fG = fG / 12.92f;
      if (fB > 0.04045) fB = pow(((fB + 0.055) / 1.055), 2.4f);
      else fB = fB / 12.92f;*/
      static icl32f m[3][3] = {{ 0.412453f/255.0f, 0.35758f/255.0f , 0.180423f/255.0f},
                               { 0.212671f/255.0f, 0.71516f/255.0f , 0.072169f/255.0f},
                               { 0.019334f/255.0f, 0.119193f/255.0f, 0.950227f/255.0f}};
      X = m[0][0] * r + m[0][1] * g + m[0][2] * b;
      Y = m[1][0] * r + m[1][1] * g + m[1][2] * b;
      Z = m[2][0] * r + m[2][1] * g + m[2][2] * b;
    }
    // }}}

      // cube root by W.Kahan (5bit precision)
      inline icl32f cbrt_kahan(icl32f x)
      {
	      icl32u* p = (icl32u *) &x;
	      *p = *p/3 + 709921077;
	      return x;
      }

      // Halley's method for cube roots
      // (Otis E. Lancaster in Machine Method for the Extraction of a Cube Root)
      inline icl32f cbrt_halley(const icl32f a, const icl32f x)
      {
	      const icl32f a3 = a * a * a;
        const icl32f b = a * (a3 + x + x) / (a3 + a3 + x);
	      return b;
      }

      // cube root
      inline icl32f cbrt(icl32f x) {
        return cbrt_halley(cbrt_kahan(x), x);
      }

    inline void cc_util_xyz_to_lab_inline(const icl32f X, const icl32f Y, const icl32f Z, icl32f &L, icl32f &a, icl32f &b){
      // {{{ open
      static const icl32f wX = 1/0.950455f;
      static const icl32f wY = 1/1.000;
      static const icl32f wZ = 1/1.088753f;
      
      icl32f XXn = X * wX;
      icl32f YYn = Y * wY;
      icl32f ZZn = Z * wZ;

      icl32f fX = (XXn > 0.008856f) ? cbrt(XXn) : 7.787f * XXn + (16.0f / 116.0f);
      icl32f fY = (YYn > 0.008856f) ? cbrt(YYn) : 7.787f * YYn + (16.0f / 116.0f); 
      icl32f fZ = (ZZn > 0.008856f) ? cbrt(ZZn) : 7.787f * ZZn + (16.0f / 116.0f);

      L = (116.0f * 2.55f * fY) - 16.0f * (255.0f / 100.0f);
      a = 500.0f * (fX - fY) + 128;
      b = 200.0f * (fY - fZ) + 128;
    }
    // }}}

    void cc_util_rgb_to_lab(const icl32f &r, const icl32f &g, const icl32f &b, icl32f &L, icl32f &A, icl32f &B){
      icl32f x(0),y(0),z(0);
      cc_util_rgb_to_xyz_inline(r,g,b,x,y,z);
      cc_util_xyz_to_lab_inline(x,y,z,L,A,B);
    }

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
        case 6: r = v;    g = mid1; b = m;    break; // TODO: delete line?
      }
      
      r *= 255;
      g *= 255;
      b *= 255;
    }
    // }}}

    inline void cc_util_hls_to_rgb_inline(const icl32f h255, const icl32f l255, const icl32f sl255, icl32f &r, icl32f &g, icl32f &b){
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
        case 6: r = v;    g = mid1; b = m;    break;
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

    inline void cc_util_lab_to_xyz(const icl32f l, const icl32f a, const icl32f b, icl32f &x, icl32f &y, icl32f &z){
      // {{{ open
  
      static const icl32f n = 16.0f/116.0f;
  
      // white point values
      static const icl32f wX = 0.950455f;
      static const icl32f wZ = 1.088754f;
  
      icl32f fy = (l+16.0f*2.55)/(2.55f*116.0f);
      icl32f fx = fy+(a-128)/500.0f;
      icl32f fz = fy-(b-128)/200.0f;
  
      y = (fy>0.206893f) ?     pow(fy,3) :    (fy-n)/7.787f;
      x = (fx>0.206893f) ?  wX*pow(fx,3) : wX*(fx-n)/7.787f;
      z = (fz>0.206893f) ?  wZ*pow(fz,3) : wZ*(fz-n)/7.787f;
    }
  
    // }}}
    inline void cc_util_xyz_to_rgb(const icl32f x, const icl32f y, const icl32f z, icl32f &r, icl32f &g, icl32f &b){
      // {{{ open
      static icl32f m[3][3] = {{ 3.240479*255.0f, -1.53715*255.0f, -0.498535*255.0f},
                               {-0.969256*255.0f,  1.875991*255.0f, 0.041556*255.0f},
                               { 0.055648*255.0f, -0.204043*255.0f, 1.057311*255.0f}};
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
            cc_util_rgb_to_hls_inline(clipped_cast<S,icl32f>(*itR),
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
            cc_util_rgb_to_hls_inline(clipped_cast<S,icl32f>(r[i]),
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
        if (roiOnly) {
          const ImgIterator<S> itR = src->beginROI(0);
          const ImgIterator<S> itG = src->beginROI(1);
          const ImgIterator<S> itB = src->beginROI(2);
          ImgIterator<D> itCr = dst->beginROI(0);
          ImgIterator<D> itCg = dst->beginROI(1);
          const ImgIterator<S> itEnd = src->endROI(0);
          register S sum;
          for(; itR!= itEnd; ++itR,++itG,++itB,++itCr,++itCg){
            sum = *itR + *itG + *itB;
            sum += !sum; //avoid division by zero
            *itCr = clipped_cast<S,D>((*itR * 255) / sum);
            *itCg = clipped_cast<S,D>((*itG * 255) / sum);
          }
        } else {
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
      }
      
    };
  
    // }}}
    template<class S, class D> struct CCFunc<S,D,formatRGB,formatYUV>{
      // {{{ open
      static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
        if(roiOnly){
          const ImgIterator<S> itR = src->beginROI(0);
          const ImgIterator<S> itG = src->beginROI(1);
          const ImgIterator<S> itB = src->beginROI(2);
          ImgIterator<D> itY = dst->beginROI(0);
          ImgIterator<D> itU = dst->beginROI(1);
          ImgIterator<D> itV = dst->beginROI(2);
          const ImgIterator<D> itEnd = dst->endROI(0);
          register icl32s reg_y, reg_u, reg_v;
          for(;itY!= itEnd;++itR,++itG,++itB,++itY, ++itU, ++itV){
            cc_util_rgb_to_yuv_inline(clipped_cast<S,icl32s>(*itR),
                                      clipped_cast<S,icl32s>(*itG),
                                      clipped_cast<S,icl32s>(*itB),
                                      reg_y, reg_u, reg_v);
            *itY = clipped_cast<icl32s,D>(reg_y);
            *itU = clipped_cast<icl32s,D>(reg_u);
            *itV = clipped_cast<icl32s,D>(reg_v);
          }
        }else{
          GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
          GET_3_CHANNEL_POINTERS_NODIM(D,dst,y,u,v);
          register icl32s reg_y, reg_u, reg_v;
          for(int i=0;i<dim;++i){ 
            cc_util_rgb_to_yuv_inline(clipped_cast<S,icl32s>(r[i]),
                                      clipped_cast<S,icl32s>(g[i]),
                                      clipped_cast<S,icl32s>(b[i]),
                                      reg_y, reg_u, reg_v);
            y[i] = clipped_cast<icl32s,D>(reg_y);
            u[i] = clipped_cast<icl32s,D>(reg_u);
            v[i] = clipped_cast<icl32s,D>(reg_v);
          }
        }
      }
    };
  
    // }}}
    template<class S, class D> struct CCFunc<S,D,formatRGB,formatLAB>{
      // {{{ open
      static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
        register icl32f reg_X,reg_Y,reg_Z,reg_L, reg_a, reg_b;

        if(roiOnly){
          const ImgIterator<S> itR = src->beginROI(0);
          const ImgIterator<S> itG = src->beginROI(1);
          const ImgIterator<S> itB = src->beginROI(2);
          ImgIterator<D> itL = dst->beginROI(0);
          ImgIterator<D> ita = dst->beginROI(1);
          ImgIterator<D> itb = dst->beginROI(2);
          const ImgIterator<S> itEnd = src->endROI(0);

          for(;itR!= itEnd;++itR,++itG,++itB,++itL,++ita,++itb){
            cc_util_rgb_to_xyz_inline(clipped_cast<S,icl32f>(*itR),
                               clipped_cast<S,icl32f>(*itG),
                               clipped_cast<S,icl32f>(*itB),
                               reg_X,reg_Y,reg_Z);
            cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_L,reg_a,reg_b);
            *itL = clipped_cast<icl32f,D>(reg_L);
            *ita = clipped_cast<icl32f,D>(reg_a);
            *itb = clipped_cast<icl32f,D>(reg_b);
          }
        }else{
          GET_3_CHANNEL_POINTERS_DIM(const S,src,r,g,b,dim);
          GET_3_CHANNEL_POINTERS_NODIM(D,dst,LL,aa,bb);

          for(int i=0;i<dim;++i){ 
            cc_util_rgb_to_xyz_inline(clipped_cast<S,icl32f>(r[i]),
                               clipped_cast<S,icl32f>(g[i]),
                               clipped_cast<S,icl32f>(b[i]),
                               reg_X,reg_Y,reg_Z);
            cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_L,reg_a,reg_b);
            LL[i] = clipped_cast<icl32f,D>(reg_L);
            aa[i] = clipped_cast<icl32f,D>(reg_a);
            bb[i] = clipped_cast<icl32f,D>(reg_b);
          }
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
          icl::core::convert(src->getData(1),src->getData(1)+src->getDim(), dst->getData(0));
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
            cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*itH),
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
            cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(h[i]),
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
  
/* the emulated version is faster than this one
    template<class S, class D> struct CCFunc<S,D,formatHLS,formatYUV>{
      // {{{ open
      static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
        FUNCTION_LOG("");
        
        register icl32f reg_r(0), reg_g(0), reg_b(0);
        register icl32s reg_y(0), reg_u(0), reg_v(0);
        if(roiOnly){
          const ImgIterator<S> itH = src->beginROI(0);
          const ImgIterator<S> itL = src->beginROI(1);
          const ImgIterator<S> itS = src->beginROI(2);
          ImgIterator<D> itY = dst->beginROI(0);
          ImgIterator<D> itU = dst->beginROI(1);
          ImgIterator<D> itV = dst->beginROI(2);
          const ImgIterator<S> itEnd = src->endROI(0);
          for(;itH!= itEnd;++itH,++itL,++itS,++itY,++itU,++itV){
            cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*itH),
                                      clipped_cast<S,icl32f>(*itL),
                                      clipped_cast<S,icl32f>(*itS),
                                      reg_r,reg_g,reg_b);
            cc_util_rgb_to_yuv_inline(reg_r, reg_g, reg_b, reg_y, reg_u, reg_v);
            *itY = clipped_cast<icl32s,D>(reg_y);
            *itU = clipped_cast<icl32s,D>(reg_u);
            *itV = clipped_cast<icl32s,D>(reg_v);
          }
        }else{
          GET_3_CHANNEL_POINTERS_DIM(const S,src,h,l,s,dim);
          GET_3_CHANNEL_POINTERS_NODIM(D,dst,y,u,v);
          
          for(int i=0;i<dim;++i){
            cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(h[i]),
                                      clipped_cast<S,icl32f>(l[i]),
                                      clipped_cast<S,icl32f>(s[i]),
                                      reg_r,reg_g,reg_b);
            cc_util_rgb_to_yuv_inline(reg_r, reg_g, reg_b, reg_y, reg_u, reg_v);
            y[i] = clipped_cast<icl32s,D>(reg_y);
            u[i] = clipped_cast<icl32s,D>(reg_u);
            v[i] = clipped_cast<icl32s,D>(reg_v);
          }
        }
      }
    };
    // }}}
*/

    /// FROM FORMAT LAB
    template<class S, class D> struct CCFunc<S,D,formatLAB,formatGray>{
      // {{{ open
      static void convert(const Img<S> *src, Img<D> *dst, bool roiOnly){
        FUNCTION_LOG("");
        if(roiOnly){
          convertChannelROI(src,0,src->getROIOffset(),src->getROISize(), 
                            dst,0,dst->getROIOffset(),dst->getROISize());
        }else{
          icl::core::convert(src->getData(0),src->getData(0)+src->getDim(), dst->getData(0));        
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
          icl::core::convert(src->getData(0),src->getData(0)+src->getDim(), dst->getData(0));        
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
            cc_util_yuv_to_rgb_inline(clipped_cast<S,icl32s>(*itY),
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
            cc_util_yuv_to_rgb_inline(clipped_cast<S,icl32s>(y[i]),
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

//  the IPP conversion functions are replaced by a SSE implementation
/*
  #ifdef ICL_HAVE_IPP
    template<class IppFunc>
    static void convert_color_with_ipp(const Img8u *src, Img8u *dst, bool roiOnly, IppFunc ipp_func){
      if(roiOnly){
        const icl8u *s[3]={src->getROIData(0),src->getROIData(1),src->getROIData(2)};
        icl8u *d[3] = {dst->getROIData(0),dst->getROIData(1),dst->getROIData(2)};
        ipp_func(s,src->getLineStep(),d,dst->getLineStep(),src->getROISize());
      }else{
        const icl8u *s[3]={src->getData(0),src->getData(1),src->getData(2)};
        icl8u *d[3] = {dst->getData(0),dst->getData(1),dst->getData(2)};
        ipp_func(s,src->getLineStep(),d,dst->getLineStep(),src->getSize());
      }
    }
  #define USE_IPP_CONVERT(SRC_FMT,DST_FMT,FUNC)                                  \
    template<> struct CCFunc<icl8u,icl8u,format##SRC_FMT,format##DST_FMT>{       \
      static void convert(const Img<icl8u> *src, Img<icl8u> *dst, bool roiOnly){ \
        convert_color_with_ipp(src,dst,roiOnly,ippi##FUNC);                      \
      }                                                                          \
    };
  
  #define USE_IPP_CONVERT_SWAP_SRC_RG(SRC_FMT,DST_FMT,FUNC)                        \
    template<> struct CCFunc<icl8u,icl8u,format##SRC_FMT,format##DST_FMT>{         \
      static void convert(const Img<icl8u> *srcIn, Img<icl8u> *dst, bool roiOnly){ \
        static const int bgr[] = {2,1,0};                                          \
        SmartPtr<const Img8u> src = srcIn->selectChannels(std::vector<int>(bgr,bgr+3)); \
        convert_color_with_ipp(src.get(),dst,roiOnly,ippi##FUNC);                  \
      }                                                                            \
    };
  #define USE_IPP_CONVERT_SWAP_DST_RG(SRC_FMT,DST_FMT,FUNC)                        \
    template<> struct CCFunc<icl8u,icl8u,format##SRC_FMT,format##DST_FMT>{         \
      static void convert(const Img<icl8u> *src, Img<icl8u> *dst, bool roiOnly){   \
        convert_color_with_ipp(src,dst,roiOnly,ippi##FUNC);                        \
        dst->swapChannels(0,2);                                                    \
      }                                                                            \
    };
  
  

    USE_IPP_CONVERT(RGB,YUV,RGBToYUV_8u_P3R);
    USE_IPP_CONVERT(YUV,RGB,YUVToRGB_8u_P3R);
    
    USE_IPP_CONVERT_SWAP_SRC_RG(RGB,HLS,BGRToHLS_8u_P3R);
    USE_IPP_CONVERT_SWAP_DST_RG(HLS,RGB,HLSToBGR_8u_P3R);

    template<> struct CCFunc<icl8u,icl8u,formatRGB,formatLAB>{
      static void convert(const Img<icl8u> *src, Img<icl8u> *dst, bool roiOnly){
        if(roiOnly){
          const Rect roi = src->getROI();
          const int w = src->getWidth();
          const int rw = roi.width, rh= roi.height;
          const Size line(rw,1);
          std::vector<icl8u> sbuf(rw*3),dbuf(rw*3);
          const icl8u* bgrSrc[] = {src->getROIData(2),src->getROIData(1),src->getROIData(0)};
          icl8u* bgrDst[] ={dst->getROIData(0),dst->getROIData(1),dst->getROIData(2)};
          
          for(int y=0;y<rh;++y){
            ippiCopy_8u_P3C3R(bgrSrc, w, sbuf.data(), rw, line);
            ippiBGRToLab_8u_C3R(sbuf.data(),rw,dbuf.data(), rw, line);
            ippiCopy_8u_C3P3R(dbuf.data(),rw, bgrDst, w, line);
            for(int i=0;i<3;++i){
              bgrSrc[i]+=w;
              bgrDst[i]+=w;
            }
          }
        }else{
          const int w = src->getWidth(), h = src->getHeight();
          const Size line(w,1);
          std::vector<icl8u> sbuf(w*3),dbuf(w*3);
          const icl8u* bgrSrc[] = {src->getData(2),src->getData(1),src->getData(0)};
          icl8u* bgrDst[] ={dst->getData(0),dst->getData(1),dst->getData(2)};

          for(int y=0;y<h;++y){
            ippiCopy_8u_P3C3R(bgrSrc, w, sbuf.data(), w, line);
            ippiBGRToLab_8u_C3R(sbuf.data(),w,dbuf.data(), w, line);
            ippiCopy_8u_C3P3R(dbuf.data(),w, bgrDst, w, line);
            for(int i=0;i<3;++i){
              bgrSrc[i]+=w;
              bgrDst[i]+=w;
            }
          }
        }
      }
    };
  
   template<> struct CCFunc<icl8u,icl8u,formatLAB ,formatRGB>{
      static void convert(const Img<icl8u> *src, Img<icl8u> *dst, bool roiOnly){
        if(roiOnly){
          const Rect roi = src->getROI();
          const int w = src->getWidth();
          const int rw = roi.width, rh= roi.height;
          const Size line(rw,1);
          std::vector<icl8u> sbuf(rw*3),dbuf(rw*3);
          const icl8u* bgrSrc[] = {src->getROIData(0),src->getROIData(1),src->getROIData(2)};
          icl8u* bgrDst[] ={dst->getROIData(2),dst->getROIData(1),dst->getROIData(0)};
          
          for(int y=0;y<rh;++y){
            ippiCopy_8u_P3C3R(bgrSrc, w, sbuf.data(), rw, line);
            ippiLabToBGR_8u_C3R(sbuf.data(),rw,dbuf.data(), rw, line);
            ippiCopy_8u_C3P3R(dbuf.data(),rw, bgrDst, w, line);
            for(int i=0;i<3;++i){
              bgrSrc[i]+=w;
              bgrDst[i]+=w;
            }
          }
        }else{
          const int w = src->getWidth(), h = src->getHeight();
          const Size line(w,1);
          std::vector<icl8u> sbuf(w*3),dbuf(w*3);
          const icl8u* bgrSrc[] = {src->getData(0),src->getData(1),src->getData(2)};
          icl8u* bgrDst[] ={dst->getData(2),dst->getData(1),dst->getData(0)};
          
          for(int y=0;y<h;++y){
            ippiCopy_8u_P3C3R(bgrSrc, w, sbuf.data(), w, line);
            ippiLabToBGR_8u_C3R(sbuf.data(),w,dbuf.data(), w, line);
            ippiCopy_8u_C3P3R(dbuf.data(),w, bgrDst, w, line);
            for(int i=0;i<3;++i){
              bgrSrc[i]+=w;
              bgrDst[i]+=w;
            }
          }
        }
      }
    };
  
    /// lab conversion in IPP is only available for planar images
    //USE_IPP_CONVERT_SWAP_RB(RGB,LAB,BGRToLAB);
    //USE_IPP_CONVERT_SWAP_RB(LAB,BGR,LABToBGR);
  #endif
*/

  #ifdef ICL_HAVE_SSE2

    // ++ for-loops ++ // 

    template<class S, class D>
    inline void sse_for_image_roi(const Img<S> *src, Img<D> *dst,
                                  void (*subMethod)(const S*, D*, D*, D*),
                                  void (*subSSEMethod)(const S*, D*, D*, D*),
                                  long step = 16) {
      int srcW = src->getWidth();
      int dstW = dst->getWidth();

      Point pROI;
      Size sROI;
      src->getROI(pROI, sROI);

      long offset  = pROI.y * src->getWidth() + pROI.x;

      const S *src0 = src->getData(0) + offset;

      D *dst0      = dst->getData(0) + offset;
      D *dst1      = dst->getData(1) + offset;
      D *dst2      = dst->getData(2) + offset;
      D *dstEnd    = dst0 + sROI.width + (sROI.height - 1) * dstW;

      sse_for(src0, dst0, dst1, dst2, dstEnd,
              srcW, dstW, sROI.width, subMethod, subSSEMethod, step, step);
    }

    template<class S, class D>
    inline void sse_for_image_roi(const Img<S> *src, Img<D> *dst,
                                  void (*subMethod)(const S*, const S*, const S*, D*),
                                  void (*subSSEMethod)(const S*, const S*, const S*, D*),
                                  long step = 16) {
      int srcW = src->getWidth();
      int dstW = dst->getWidth();

      Point pROI;
      Size sROI;
      src->getROI(pROI, sROI);

      long offset  = pROI.y * src->getWidth() + pROI.x;

      const S *src0 = src->getData(0) + offset;
      const S *src1 = src->getData(1) + offset;
      const S *src2 = src->getData(2) + offset;

      D *dst0   = dst->getData(0) + offset;
      D *dstEnd = dst0 + sROI.width + (sROI.height - 1) * dstW;

      sse_for(src0, src1, src2, dst0, dstEnd,
              srcW, dstW, sROI.width, subMethod, subSSEMethod, step, step);
    }

    template<class S, class D>
    inline void sse_for_image_roi(const Img<S> *src, Img<D> *dst,
                                  void (*subMethod)(const S*, const S*, const S*, D*, D*),
                                  void (*subSSEMethod)(const S*, const S*, const S*, D*, D*),
                                  long step = 16) {
      int srcW = src->getWidth();
      int dstW = dst->getWidth();

      Point pROI;
      Size sROI;
      src->getROI(pROI, sROI);

      long offset  = pROI.y * src->getWidth() + pROI.x;

      const S *src0 = src->getData(0) + offset;
      const S *src1 = src->getData(1) + offset;
      const S *src2 = src->getData(2) + offset;

      D *dst0   = dst->getData(0) + offset;
      D *dst1   = dst->getData(1) + offset;
      D *dstEnd = dst0 + sROI.width + (sROI.height - 1) * dstW;

      sse_for(src0, src1, src2, dst0, dst1, dstEnd,
              srcW, dstW, sROI.width, subMethod, subSSEMethod, step, step);
    }

    template<class S, class D>
    inline void sse_for_image_roi(const Img<S> *src, Img<D> *dst,
                                  void (*subMethod)(const S*, const S*, const S*, D*, D*, D*),
                                  void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*),
                                  long step = 16) {
      int srcW = src->getWidth();
      int dstW = dst->getWidth();

      Point pROI;
      Size sROI;
      src->getROI(pROI, sROI);

      long offset  = pROI.y * src->getWidth() + pROI.x;

      const S *src0 = src->getData(0) + offset;
      const S *src1 = src->getData(1) + offset;
      const S *src2 = src->getData(2) + offset;

      D *dst0   = dst->getData(0) + offset;
      D *dst1   = dst->getData(1) + offset;
      D *dst2   = dst->getData(2) + offset;
      D *dstEnd = dst0 + sROI.width + (sROI.height - 1) * dstW;

      sse_for(src0, src1, src2, dst0, dst1, dst2, dstEnd,
              srcW, dstW, sROI.width, subMethod, subSSEMethod, step, step);
    }

    template<class S, class D>
    inline void sse_for_image(const Img<S> *src, Img<D> *dst,
                              void (*subMethod)(const S*, D*, D*, D*),
                              void (*subSSEMethod)(const S*, D*, D*, D*),
                              long step = 16) {
      const S *src0 = src->getData(0);

      D *dst0      = dst->getData(0);
      D *dst1      = dst->getData(1);
      D *dst2      = dst->getData(2);
      D *dstEnd    = dst0 + dst->getDim();

      sse_for(src0, dst0, dst1, dst2, dstEnd, subMethod, subSSEMethod, step, step);
    }

    template<class S, class D>
    inline void sse_for_image(const Img<S> *src, Img<D> *dst,
                              void (*subMethod)(const S*, const S*, const S*, D*),
                              void (*subSSEMethod)(const S*, const S*, const S*, D*),
                              long step = 16) {
      const S *src0 = src->getData(0);
      const S *src1 = src->getData(1);
      const S *src2 = src->getData(2);

      D *dst0   = dst->getData(0);
      D *dstEnd = dst0 + dst->getDim();

      sse_for(src0, src1, src2, dst0, dstEnd, subMethod, subSSEMethod, step, step);
    }

    template<class S, class D>
    inline void sse_for_image(const Img<S> *src, Img<D> *dst,
                              void (*subMethod)(const S*, const S*, const S*, D*, D*),
                              void (*subSSEMethod)(const S*, const S*, const S*, D*, D*),
                              long step = 16) {
      const S *src0 = src->getData(0);
      const S *src1 = src->getData(1);
      const S *src2 = src->getData(2);

      D *dst0      = dst->getData(0);
      D *dst1      = dst->getData(1);
      D *dstEnd    = dst0 + dst->getDim();

      sse_for(src0, src1, src2, dst0, dst1, dstEnd, subMethod, subSSEMethod, step, step);
    }

    template<class S, class D>
    inline void sse_for_image(const Img<S> *src, Img<D> *dst,
                              void (*subMethod)(const S*, const S*, const S*, D*, D*, D*),
                              void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*),
                              long step = 16) {
      const S *src0 = src->getData(0);
      const S *src1 = src->getData(1);
      const S *src2 = src->getData(2);

      D *dst0      = dst->getData(0);
      D *dst1      = dst->getData(1);
      D *dst2      = dst->getData(2);
      D *dstEnd    = dst0 + dst->getDim();

      sse_for(src0, src1, src2, dst0, dst1, dst2, dstEnd, subMethod, subSSEMethod, step, step);
    }

    // -- for-loops -- // 


    #define USE_SSE_CONVERT(SRC_TYPE,DST_TYPE,SRC_FMT,DST_FMT,FUNC,SSEFUNC,NUM_VAL)      \
      template<> struct CCFunc<SRC_TYPE,DST_TYPE,format##SRC_FMT,format##DST_FMT>{       \
        static void convert(const Img<SRC_TYPE> *src, Img<DST_TYPE> *dst, bool roiOnly){ \
          FUNCTION_LOG("");                                                              \
          if (roiOnly) {                                                                 \
            sse_for_image_roi(src, dst, FUNC, SSEFUNC, NUM_VAL);                         \
          } else {                                                                       \
            sse_for_image(src, dst, FUNC, SSEFUNC, NUM_VAL);                             \
          }                                                                              \
        }                                                                                \
      };


    // ++ Gray to RGB ++ //

    template<class S, class D>
    inline void subGraytoRGB(const S *gr, D *r, D *g, D *b) {
      *r = *g = *b = clipped_cast<S,D>(*gr);
    }

    inline void subSSEGraytoRGB(const icl8u *gr, icl8u *r, icl8u *g, icl8u *b) {
      // load gray values
      icl128i vGr = icl128i(gr);

      // store the 'calculated' values
      vGr.storeu(r);
      vGr.storeu(g);
      vGr.storeu(b);
    }

    inline void subSSEGraytoRGB(const icl8u *gr, icl32f *r, icl32f *g, icl32f *b) {
      // load gray values
      icl512 vGr = icl512(gr);

      // store the 'calculated' values
      vGr.storeu(r);
      vGr.storeu(g);
      vGr.storeu(b);
    }

    inline void subSSEGraytoRGB(const icl32f *gr, icl8u *r, icl8u *g, icl8u *b) {
      // load gray values
      icl512 vGr = icl512(gr);

      // store the 'calculated' values
      vGr.storeu(r);
      vGr.storeu(g);
      vGr.storeu(b);
    }

    inline void subSSEGraytoRGB(const icl32f *gr, icl32f *r, icl32f *g, icl32f *b) {
      // load gray values
      icl128 vGr = icl128(gr);

      // store the 'calculated' values
      vGr.storeu(r);
      vGr.storeu(g);
      vGr.storeu(b);
    }

    USE_SSE_CONVERT(icl8u,icl8u,Gray,RGB,subGraytoRGB,subSSEGraytoRGB,16)
    USE_SSE_CONVERT(icl8u,icl32f,Gray,RGB,subGraytoRGB,subSSEGraytoRGB,16)
    USE_SSE_CONVERT(icl32f,icl8u,Gray,RGB,subGraytoRGB,subSSEGraytoRGB,16)
    USE_SSE_CONVERT(icl32f,icl32f,Gray,RGB,subGraytoRGB,subSSEGraytoRGB,4)

    // -- Gray to RGB -- //


    // ++ RBG to Gray ++ ///

    template<class S, class D>
    inline void subRGBtoGray(const S *r, const S *g, const S *b, D *gr) {
      *gr = clipped_cast<S,D>((*r + *g + *b)/3.0f + 0.5f);
    }

    inline void subSSERGBtoGray(const icl8u *r, const icl8u *g, const icl8u *b, icl8u *gr) {
      icl256i vR = icl128i(r);
      icl256i vG = icl128i(g);
      icl256i vB = icl128i(b);

      vR.add16(vB);
      vR.add16(vG);

      icl512 vRes = icl512i(vR);

      vRes *= icl512(1.0f/3.0f);
      vRes.storeu(gr);
    }

    inline void subSSERGBtoGray(const icl8u *r, const icl8u *g, const icl8u *b, icl32f *gr) {
      icl256i vR = icl128i(r);
      icl256i vG = icl128i(g);
      icl256i vB = icl128i(b);

      vR.add16(vB);
      vR.add16(vG);

      icl512 vRes = icl512i(vR);

      vRes *= icl512(1.0f/3.0f);
      vRes.storeu(gr);
    }

    inline void subSSERGBtoGray(const icl32f *r, const icl32f *g, const icl32f *b, icl8u *gr) {
      icl512 vR(r);
      icl512 vG(g);
      icl512 vB(b);

      vR += vB;
      vR += vG;
      vR *= icl512(1.0f/3.0f);

      vR.storeu(gr);
    }

    inline void subSSERGBtoGray(const icl32f *r, const icl32f *g, const icl32f *b, icl32f *gr) {
      icl128 vR(r);
      icl128 vG(g);
      icl128 vB(b);

      vR += vB;
      vR += vG;
      vR *= icl128(1.0f/3.0f);

      vR.storeu(gr);
    }

    template<> struct CCFunc<icl8u,icl8u,formatRGB,formatGray>{
      static void convert(const Img<icl8u> *src, Img<icl8u> *dst, bool roiOnly){
        FUNCTION_LOG("");
        if(roiOnly){
          sse_for_image_roi(src, dst, subRGBtoGray, subSSERGBtoGray);
        }else{
          GET_3_CHANNEL_POINTERS_DIM(const icl8u,src,r,g,b,dim);
          icl8u *gr = dst->getData(0);

          // convert the channels to vector channels
          __m128i *cR  = (__m128i*)r;
          __m128i *cG  = (__m128i*)g;
          __m128i *cB  = (__m128i*)b;
          __m128i *cGr = (__m128i*)gr;

          int i = 0;

          // convert 16 values at the same time
          for(; i<dim-15; i+=16, cR++, cG++, cB++, cGr++){
            icl256i vR = icl128i(cR);
            icl256i vG = icl128i(cG);
            icl256i vB = icl128i(cB);

            vR.add16(vB);
            vR.add16(vG);

            icl512 vTmp = icl512i(vR);

            vTmp *= icl512(1.0f/3.0f);

            icl128i vRes = icl512i(vTmp).pack8u();
            vRes.storeu(cGr);
          }

          // convert the last values (one by one)
          for(; i<dim; ++i){
            gr[i] = clipped_cast<icl8u,icl8u>((r[i]+g[i]+b[i])/3);
          }
        }
      }
    };

    template<> struct CCFunc<icl8u,icl32f,formatRGB,formatGray>{
      static void convert(const Img<icl8u> *src, Img<icl32f> *dst, bool roiOnly){
        FUNCTION_LOG("");
        if(roiOnly){
          sse_for_image_roi(src, dst, subRGBtoGray, subSSERGBtoGray);
        }else{
          GET_3_CHANNEL_POINTERS_DIM(const icl8u,src,r,g,b,dim);
          icl32f *gr = dst->getData(0);

          // convert the channels to  vector channels
          __m128i *cR = (__m128i*)r;
          __m128i *cG = (__m128i*)g;
          __m128i *cB = (__m128i*)b;

          int i = 0;

          // convert 16 values at the same time
          for(; i<dim-15; i+=16, ++cR, ++cG, ++cB){
            icl256i vR = icl128i(cR);
            icl256i vG = icl128i(cG);
            icl256i vB = icl128i(cB);

            vR.add16(vB);
            vR.add16(vG);

            icl512 vRes = icl512i(vR);

            vRes *= icl512(1.0f/3.0f);
            vRes.storeu(&gr[i]);
          }

          // convert the last values (one by one)
          for(; i<dim; ++i){
            gr[i] = clipped_cast<icl8u,icl32f>((r[i]+g[i]+b[i])/3.0f);
          }
        }
      }
    };

    USE_SSE_CONVERT(icl32f,icl8u,RGB,Gray,subRGBtoGray,subSSERGBtoGray,16)
    USE_SSE_CONVERT(icl32f,icl32f,RGB,Gray,subRGBtoGray,subSSERGBtoGray,4)

    // -- RBG to Gray -- //


    // ++ RBG to HLS ++ //

    template<class S, class D>
    inline void subRGBtoHLS(const S *r, const S *g, const S *b, D *h, D *l, D *s) {
      register icl32f reg_h, reg_l, reg_s;
      cc_util_rgb_to_hls(clipped_cast<S,icl32f>(*r),
                         clipped_cast<S,icl32f>(*g),
                         clipped_cast<S,icl32f>(*b),
                         reg_h,reg_l,reg_s);
      *h = clipped_cast<icl32f,D>(reg_h);
      *l = clipped_cast<icl32f,D>(reg_l);
      *s = clipped_cast<icl32f,D>(reg_s);
    }

    template<class S>
    inline void subRGBtoHLS(const S *r, const S *g, const S *b, icl8u *h, icl8u *l, icl8u *s) {
      register icl32f reg_h, reg_l, reg_s;
      cc_util_rgb_to_hls_inline(clipped_cast<S,icl32f>(*r),
                                clipped_cast<S,icl32f>(*g),
                                clipped_cast<S,icl32f>(*b),
                                reg_h,reg_l,reg_s);
      *h = clipped_cast<icl32f,icl8u>(reg_h + 0.5f);
      *l = clipped_cast<icl32f,icl8u>(reg_l + 0.5f);
      *s = clipped_cast<icl32f,icl8u>(reg_s + 0.5f);
    }

    template<class S, class D>
    inline void subSSERGBtoHLS(const S *r, const S *g, const S *b,
                               D *h, D *l, D *s) {
      // load RGB values
      icl512 vR = icl512(r);
      icl512 vG = icl512(g);
      icl512 vB = icl512(b);

      // change the range to 0..1
      vR *= icl512(1.0f/255.0f);
      vG *= icl512(1.0f/255.0f);
      vB *= icl512(1.0f/255.0f);

      icl512 vMax = max(max(vR, vG), vB);
      icl512 vMin = min(min(vR, vG), vB);
      icl512 vMpM  = vMax + vMin;
      icl512 vMmM  = vMax - vMin;

      // calculate lightness
      icl512 vL = vMpM * icl512(0.5f);

      // calculate saturation

      icl512 vIf0 = (vL > icl512(0.5f));
      icl512 vIf1 = (vMin != vMax);

      icl512 vS = ((vMmM * (icl512(2.0f) - vMpM).rcp()) & vIf0);
      icl512 vS1 = andnot(vMmM * vMpM.rcp(), vIf0);

      vS += vS1;
      vS &= vIf1;
      vS &= (vL != icl512(0.0f));


      // calculate hue

      vMmM.rcp();

      icl512 vIf00 = (vR == vMax);
      icl512 vIf01 = andnot(vG == vMax, vIf00);
      icl512 vIf02 = andnot(andnot(vB == vMax, vIf01), vIf00);

      icl512 vH = (((vG - vB) * vMmM) & vIf00);
      vH += ((icl512(2.0f) + ((vB - vR) * vMmM)) & vIf01);
      vH += ((icl512(4.0f) + ((vR - vG) * vMmM)) & vIf02);
      vH *= icl512(60.0f);

      icl512 vIf10 = (vH < icl512(0.0f));
      vH += (icl512(360.0f) & vIf10);


      // TODO: maybe this is not important?
      vH &= (vL != icl512(0.0f));
      vH &= vIf1;

      // change the range of the calculated values to 0..255
      vH *= icl512(255.0f/360.0f);
      vL *= icl512(255.0f);
      vS *= icl512(255.0f);

      // store the calculated values
      vH.storeu(h);
      vL.storeu(l);
      vS.storeu(s);
    }

    template<>
    inline void subSSERGBtoHLS(const icl32f *r, const icl32f *g, const icl32f *b,
                               icl32f *h, icl32f *l, icl32f *s) {
      // load RGB values
      icl128 vR = icl128(r);
      icl128 vG = icl128(g);
      icl128 vB = icl128(b);

      // change the range to 0..1
      vR *= icl128(1.0f/255.0f);
      vG *= icl128(1.0f/255.0f);
      vB *= icl128(1.0f/255.0f);

      icl128 vMax = max(max(vR, vG), vB);
      icl128 vMin = min(min(vR, vG), vB);
      icl128 vMpM  = vMax + vMin;
      icl128 vMmM  = vMax - vMin;

      // calculate lightness
      icl128 vL = vMpM * icl128(0.5f);

      // calculate saturation

      icl128 vIf0 = (vL > icl128(0.5f));
      icl128 vIf1 = (vMin != vMax);

      icl128 vS = ((vMmM * (icl128(2.0f) - vMpM).rcp()) & vIf0);
      icl128 vS1 = andnot(vMmM * vMpM.rcp(), vIf0);

      vS += vS1;
      vS &= vIf1;
      vS &= (vL != icl128(0.0f));


      // calculate hue

      vMmM.rcp();

      icl128 vIf00 = (vR == vMax);
      icl128 vIf01 = andnot(vG == vMax, vIf00);
      icl128 vIf02 = andnot(andnot(vB == vMax, vIf01), vIf00);

      icl128 vH = (((vG - vB) * vMmM) & vIf00);
      vH += ((icl128(2.0f) + ((vB - vR) * vMmM)) & vIf01);
      vH += ((icl128(4.0f) + ((vR - vG) * vMmM)) & vIf02);
      vH *= icl128(60.0f);

      icl128 vIf10 = (vH < icl128(0.0f));
      vH += (icl128(360.0f) & vIf10);


      // TODO: maybe this is not important?
      vH &= (vL != icl128(0.0f));
      vH &= vIf1;

      // change the range of the calculated values to 0..255
      vH *= icl128(255.0f/360.0f);
      vL *= icl128(255.0f);
      vS *= icl128(255.0f);

      // store the calculated values
      vH.storeu(h);
      vL.storeu(l);
      vS.storeu(s);
    }

    USE_SSE_CONVERT(icl8u,icl8u,RGB,HLS,subRGBtoHLS,subSSERGBtoHLS,16)
    USE_SSE_CONVERT(icl8u,icl32f,RGB,HLS,subRGBtoHLS,subSSERGBtoHLS,16)
    USE_SSE_CONVERT(icl32f,icl8u,RGB,HLS,subRGBtoHLS,subSSERGBtoHLS,16)
    USE_SSE_CONVERT(icl32f,icl32f,RGB,HLS,subRGBtoHLS,subSSERGBtoHLS,4)

    // -- RBG to HLS -- //


    // ++ RBG to YUV ++ //

    template<class S, class D>
    inline void subRGBtoYUV(const S *r, const S *g, const S *b, D *y, D *u, D *v) {
      register icl32f reg_y, reg_u, reg_v;
      cc_util_rgb_to_yuv(clipped_cast<S,icl32f>(*r),
                         clipped_cast<S,icl32f>(*g),
                         clipped_cast<S,icl32f>(*b),
                         reg_y, reg_u, reg_v);
      *y = clipped_cast<icl32f,D>(reg_y);
      *u = clipped_cast<icl32f,D>(reg_u);
      *v = clipped_cast<icl32f,D>(reg_v);
    }

    template<class S>
    inline void subRGBtoYUV(const S *r, const S *g, const S *b, icl8u *y, icl8u *u, icl8u *v) {
      register icl32f reg_y, reg_u, reg_v;
      cc_util_rgb_to_yuv(clipped_cast<S,icl32f>(*r),
                         clipped_cast<S,icl32f>(*g),
                         clipped_cast<S,icl32f>(*b),
                         reg_y, reg_u, reg_v);
      *y = clipped_cast<icl32f,icl8u>(reg_y);
      *u = clipped_cast<icl32f,icl8u>(reg_u);
      *v = clipped_cast<icl32f,icl8u>(reg_v);
    }

    template<class S, class D>
    inline void subSSERGBtoYUV(const S *r, const S *g, const S *b,
                               D *y, D *u, D *v) {
      // load RGB values
      icl512 vR = icl512(r);
      icl512 vG = icl512(g);
      icl512 vB = icl512(b);

      // calculate Y values
      icl512 vY(icl512(0.299f) * vR);
      vY += icl512(0.587f) * vG;
      vY += icl512(0.114f) * vB;

      // calculate U values
      vB -= vY;
      icl512 vU = icl512(0.492f) * vB;
      vU += icl512(128.0f);

      // calculate V values
      vR -= vY;
      icl512 vV = icl512(0.877f) * vR;
      vV += icl512(128.0f);

      //max(vV, C_0);
      //min(vV, C_255);

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vV.storeu(v);
    }

    template<>
    inline void subSSERGBtoYUV(const icl8u *r, const icl8u *g, const icl8u *b,
                               icl32f *y, icl32f *u, icl32f *v) {
      // load RGB values
      icl512 vR = icl512(r);
      icl512 vG = icl512(g);
      icl512 vB = icl512(b);

      // calculate Y values
      icl512 vY(icl512(0.299f) * vR);
      vY += icl512(0.587f) * vG;
      vY += icl512(0.114f) * vB;

      // calculate U values
      vB -= vY;
      icl512 vU = icl512(0.492f) * vB;
      vU += icl512(128.0f);

      // calculate V values
      vR -= vY;
      icl512 vV = icl512(0.877f) * vR;
      vV += icl512(128.0f);

      // saturate values
      vU = max(vU, icl512(0.0f));
      vU = min(vU, icl512(255.0f));
      vV = max(vV, icl512(0.0f));
      vV = min(vV, icl512(255.0f));

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vV.storeu(v);
    }

    template<>
    inline void subSSERGBtoYUV(const icl32f *r, const icl32f *g, const icl32f *b,
                               icl32f *y, icl32f *u, icl32f *v) {
      // load RGB values
      icl128 vR = icl128(r);
      icl128 vG = icl128(g);
      icl128 vB = icl128(b);

      // calculate Y values
      icl128 vY(icl128(0.299f) * vR);
      vY += icl128(0.587f) * vG;
      vY += icl128(0.114f) * vB;

      // calculate U values
      vB -= vY;
      icl128 vU = icl128(0.492f) * vB;
      vU += icl128(128.0f);

      // calculate V values
      vR -= vY;
      icl128 vV = icl128(0.877f) * vR;
      vV += icl128(128.0f);

      // saturate values
      vU = max(vU, icl128(0.0f));
      vU = min(vU, icl128(255.0f));
      vV = max(vV, icl128(0.0f));
      vV = min(vV, icl128(255.0f));

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vV.storeu(v);
    }

    USE_SSE_CONVERT(icl8u,icl8u,RGB,YUV,subRGBtoYUV,subSSERGBtoYUV,16)
    USE_SSE_CONVERT(icl8u,icl32f,RGB,YUV,subRGBtoYUV,subSSERGBtoYUV,16)
    USE_SSE_CONVERT(icl32f,icl8u,RGB,YUV,subRGBtoYUV,subSSERGBtoYUV,16)
    USE_SSE_CONVERT(icl32f,icl32f,RGB,YUV,subRGBtoYUV,subSSERGBtoYUV,4)

    // -- RBG to YUV -- //


    // ++ RBG to Lab ++ //

    template<class S, class D>
    inline void subRGBtoLab(const S *r, const S *g, const S *b, D *L, D *A, D *B) {
      register icl32f reg_X,reg_Y,reg_Z,reg_L, reg_a, reg_b;
      cc_util_rgb_to_xyz_inline(clipped_cast<S,icl32f>(*r),
                         clipped_cast<S,icl32f>(*g),
                         clipped_cast<S,icl32f>(*b),
                         reg_X,reg_Y,reg_Z);
      cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_L,reg_a,reg_b);
      *L = clipped_cast<icl32f,D>(reg_L);
      *A = clipped_cast<icl32f,D>(reg_a);
      *B = clipped_cast<icl32f,D>(reg_b);
    }

    template<class S>
    inline void subRGBtoLab(const S *r, const S *g, const S *b, icl8u *L, icl8u *A, icl8u *B) {
      register icl32f reg_X,reg_Y,reg_Z,reg_L, reg_a, reg_b;
      cc_util_rgb_to_xyz_inline(clipped_cast<S,icl32f>(*r),
                         clipped_cast<S,icl32f>(*g),
                         clipped_cast<S,icl32f>(*b),
                         reg_X,reg_Y,reg_Z);
      cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_L,reg_a,reg_b);
      *L = clipped_cast<icl32f,icl8u>(reg_L);
      *A = clipped_cast<icl32f,icl8u>(reg_a);
      *B = clipped_cast<icl32f,icl8u>(reg_b);
    }

    template<class S, class D>
    inline void subSSERGBtoLab(const S *r, const S *g, const S *b,
                               D *L, D *A, D *B) {
      // load RGB values
      icl512 vR = icl512(r);
      icl512 vG = icl512(g);
      icl512 vB = icl512(b);

      // RGB to XYZ
      icl512 x = icl512(0.412453f/255.0f) * vR
               + icl512(0.35758f/255.0f)  * vG
               + icl512(0.180423f/255.0f) * vB;
      icl512 y = icl512(0.212671f/255.0f) * vR
               + icl512(0.71516f/255.0f)  * vG
               + icl512(0.072169f/255.0f) * vB;
      icl512 z = icl512(0.019334f/255.0f) * vR
               + icl512(0.119193f/255.0f) * vG
               + icl512(0.950227f/255.0f) * vB;

      x *= icl512(1.0f/0.950455f);
      z *= icl512(1.0f/1.088753f);

      icl512 fX = cbrt(x);
      icl512 fY = cbrt(y);
      icl512 fZ = cbrt(z);

      icl512 ifX = (x > icl512(0.008856f));
      icl512 ifY = (y > icl512(0.008856f));
      icl512 ifZ = (z > icl512(0.008856f));

      fX &= ifX;
      fX += andnot(icl512(7.787f) * x + icl512(16.0f/116.0f), ifX);
      fY &= ifY;
      fY += andnot(icl512(7.787f) * y + icl512(16.0f/116.0f), ifY);
      fZ &= ifZ;
      fZ += andnot(icl512(7.787f) * z + icl512(16.0f/116.0f), ifZ);

      icl512 vL = icl512(116.0f * 2.55f) * fY - icl512(16.0f * (255.0f / 100.0f));
      icl512 va = icl512(500.0f) * (fX - fY) + icl512(128.0f);
      icl512 vb = icl512(200.0f) * (fY - fZ) + icl512(128.0f);

      // store the calculated values
      vL.storeu(L);
      va.storeu(A);
      vb.storeu(B);
    }

    template<>
    inline void subSSERGBtoLab(const icl32f *r, const icl32f *g, const icl32f *b,
                               icl32f *L, icl32f *A, icl32f *B) {
      // load RGB values
      icl128 vR = icl128(r);
      icl128 vG = icl128(g);
      icl128 vB = icl128(b);

      // RGB to XYZ
      icl128 x = icl128(0.412453f/255.0f) * vR
               + icl128(0.35758f/255.0f)  * vG
               + icl128(0.180423f/255.0f) * vB;
      icl128 y = icl128(0.212671f/255.0f) * vR
               + icl128(0.71516f/255.0f)  * vG
               + icl128(0.072169f/255.0f) * vB;
      icl128 z = icl128(0.019334f/255.0f) * vR
               + icl128(0.119193f/255.0f) * vG
               + icl128(0.950227f/255.0f) * vB;

      x *= icl128(1.0f/0.950455f);
      z *= icl128(1.0f/1.088753f);

      icl128 fX = cbrt(x);
      icl128 fY = cbrt(y);
      icl128 fZ = cbrt(z);

      icl128 ifX = (x > icl128(0.008856f));
      icl128 ifY = (y > icl128(0.008856f));
      icl128 ifZ = (z > icl128(0.008856f));

      fX &= ifX;
      fX += andnot(icl128(7.787f) * x + icl128(16.0f/116.0f), ifX);
      fY &= ifY;
      fY += andnot(icl128(7.787f) * y + icl128(16.0f/116.0f), ifY);
      fZ &= ifZ;
      fZ += andnot(icl128(7.787f) * z + icl128(16.0f/116.0f), ifZ);

      icl128 vL = icl128(116.0f * 2.55f) * fY - icl128(16.0f * (255.0f / 100.0f));
      icl128 va = icl128(500.0f) * (fX - fY) + icl128(128.0f);
      icl128 vb = icl128(200.0f) * (fY - fZ) + icl128(128.0f);

      // store the calculated values
      vL.storeu(L);
      va.storeu(A);
      vb.storeu(B);
    }

    USE_SSE_CONVERT(icl8u,icl8u,RGB,LAB,subRGBtoLab,subSSERGBtoLab,16)
    USE_SSE_CONVERT(icl8u,icl32f,RGB,LAB,subRGBtoLab,subSSERGBtoLab,16)
    USE_SSE_CONVERT(icl32f,icl8u,RGB,LAB,subRGBtoLab,subSSERGBtoLab,16)
    USE_SSE_CONVERT(icl32f,icl32f,RGB,LAB,subRGBtoLab,subSSERGBtoLab,4)

    // -- RBG to Lab -- //


    // ++ RBG to Chroma ++ //

    template<class S, class D>
    inline void subRGBtoChroma(const S *r, const S *g, const S *b, D *cr, D *cg) {
          register icl32f sum = *r+*g+*b;
          sum+=!sum; //avoid division by zero
          *cr=clipped_cast<icl32f,D>((*r*255)/sum + 0.5f);
          *cg=clipped_cast<icl32f,D>((*g*255)/sum + 0.5f);
    }

    template<class S, class D>
    inline void subSSERGBtoChroma(const S *r, const S *g, const S *b, D *cr, D *cg) {
      // load RGB values
      icl512 vR = icl512(r);
      icl512 vG = icl512(g);
      icl512 vB = icl512(b);

      vB += vR;
      vB += vG;
      vB.rcp();

      vR *= icl512(255.0f);
      vR *= vB;
      vG *= icl512(255.0f);
      vG *= vB;

      // store the calculated values
      vR.storeu(cr);
      vG.storeu(cg);
    }

    template<>
    inline void subSSERGBtoChroma(const icl32f *r, const icl32f *g, const icl32f *b,
                                  icl32f *cr, icl32f *cg) {
      // load RGB values
      icl128 vR = icl128(r);
      icl128 vG = icl128(g);
      icl128 vB = icl128(b);

      vB += vR;
      vB += vG;
      vB.rcp();

      vR *= icl128(255.0f);
      vR *= vB;
      vG *= icl128(255.0f);
      vG *= vB;

      // store the calculated values
      vR.storeu(cr);
      vG.storeu(cg);
    }

    USE_SSE_CONVERT(icl8u,icl8u,RGB,Chroma,subRGBtoChroma,subSSERGBtoChroma,16)
    USE_SSE_CONVERT(icl8u,icl32f,RGB,Chroma,subRGBtoChroma,subSSERGBtoChroma,16)
    USE_SSE_CONVERT(icl32f,icl8u,RGB,Chroma,subRGBtoChroma,subSSERGBtoChroma,16)
    USE_SSE_CONVERT(icl32f,icl32f,RGB,Chroma,subRGBtoChroma,subSSERGBtoChroma,4)

    // -- RBG to Chroma -- //


    // ++ HLS to RGB ++ //

    template<class S, class D>
    inline void subHLStoRGB(const S *h, const S *l, const S *s, D *r, D *g, D *b) {
      register icl32f reg_r(0), reg_g(0), reg_b(0);
      cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*h),
                                clipped_cast<S,icl32f>(*l),
                                clipped_cast<S,icl32f>(*s),
                                reg_r,reg_g,reg_b);
      *r = clipped_cast<icl32f,D>(reg_r);
      *g = clipped_cast<icl32f,D>(reg_g);
      *b = clipped_cast<icl32f,D>(reg_b);
    }

    template<class S>
    inline void subHLStoRGB(const S *h, const S *l, const S *s, icl8u *r, icl8u *g, icl8u *b) {
      register icl32f reg_r(0), reg_g(0), reg_b(0);
      cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*h),
                                clipped_cast<S,icl32f>(*l),
                                clipped_cast<S,icl32f>(*s),
                                reg_r,reg_g,reg_b);
      *r = clipped_cast<icl32f,icl8u>(reg_r + 0.5f);
      *g = clipped_cast<icl32f,icl8u>(reg_g + 0.5f);
      *b = clipped_cast<icl32f,icl8u>(reg_b + 0.5f);
    }

    template<class S, class D>
    inline void subSSEHLStoRGB(const S *h, const S *l, const S *s, D *r, D *g, D *b) {
      // load HLS values
      icl512 vH = icl512(h);
      icl512 vL = icl512(l);
      icl512 vS = icl512(s);

      vH *= icl512(6.0f/255.0f);
      vL *= icl512(1.0f/255.0f);
      vS *= icl512(1.0f/255.0f);

      icl512 vSL  = vL * vS;
      icl512 vLSL = vL + vS - vSL;
      icl512 vV = sse_ifelse(vL <= icl512(0.5f), vL + vSL, vLSL);

      icl512 vM = vL + vL - vV;
      icl512 vSV = (vV - vM) * icl512(vV).rcp();
      icl512 vST = icl512i(_mm_cvttps_epi32(vH.v0), _mm_cvttps_epi32(vH.v1),
                           _mm_cvttps_epi32(vH.v2), _mm_cvttps_epi32(vH.v3));
      icl512 vF = vH - vST;
      icl512 vVSF = vV * vSV * vF;
      icl512 vMid1 = vM + vVSF;
      icl512 vMid2 = vV - vVSF;

      icl512 vIf0 = (vV  > icl512(0.0f));
      icl512 vIf1 = (vST < icl512(1.0f));
      icl512 vIf2 = (vST < icl512(2.0f));
      icl512 vIf3 = (vST < icl512(3.0f));
      icl512 vIf4 = (vST < icl512(4.0f));
      icl512 vIf5 = (vST < icl512(5.0f));
      icl512 vIf6 = (vST < icl512(6.0f));

      icl512 vR = vV;
      vR = sse_ifelse(vIf1, vR, vMid2);
      vR = sse_ifelse(vIf2, vR, vM);
      vR = sse_ifelse(vIf4, vR, vMid1);
      vR = sse_ifelse(vIf5, vR, vV);
      vR = sse_if(vIf0, vR);

      icl512 vG = vMid1;
      vG = sse_ifelse(vIf1, vG, vV);
      vG = sse_ifelse(vIf3, vG, vMid2);
      vG = sse_ifelse(vIf4, vG, vM);
      vG = sse_ifelse(vIf6, vG, vMid1);
      vG = sse_if(vIf0, vG);

      icl512 vB = vM;
      vB = sse_ifelse(vIf2, vB, vMid1);
      vB = sse_ifelse(vIf3, vB, vV);
      vB = sse_ifelse(vIf5, vB, vMid2);
      vB = sse_ifelse(vIf6, vB, vM);
      vB = sse_if(vIf0, vB);

      vR *= icl512(255.0f);
      vG *= icl512(255.0f);
      vB *= icl512(255.0f);

      // store the calculated values
      vR.storeu(r);
      vG.storeu(g);
      vB.storeu(b);
    }

    template<>
    inline void subSSEHLStoRGB(const icl32f *h, const icl32f *l, const icl32f *s,
                               icl32f *r, icl32f *g, icl32f *b) {
      // load HLS values
      icl128 vH = icl128(h);
      icl128 vL = icl128(l);
      icl128 vS = icl128(s);

      vH *= icl128(6.0f/255.0f);
      vL *= icl128(1.0f/255.0f);
      vS *= icl128(1.0f/255.0f);

      icl128 vSL  = vL * vS;
      icl128 vLSL = vL + vS - vSL;
      icl128 vV = sse_ifelse(vL <= icl128(0.5f), vL + vSL, vLSL);

      icl128 vM = vL + vL - vV;
      icl128 vSV = (vV - vM) * icl128(vV).rcp();
      icl128 vST = icl128i(_mm_cvttps_epi32(vH.v0));
      icl128 vF = vH - vST;
      icl128 vVSF = vV * vSV * vF;
      icl128 vMid1 = vM + vVSF;
      icl128 vMid2 = vV - vVSF;

      icl128 vIf0 = (vV  > icl128(0.0f));
      icl128 vIf1 = (vST < icl128(1.0f));
      icl128 vIf2 = (vST < icl128(2.0f));
      icl128 vIf3 = (vST < icl128(3.0f));
      icl128 vIf4 = (vST < icl128(4.0f));
      icl128 vIf5 = (vST < icl128(5.0f));
      icl128 vIf6 = (vST < icl128(6.0f));

      icl128 vR = vV;
      vR = sse_ifelse(vIf1, vR, vMid2);
      vR = sse_ifelse(vIf2, vR, vM);
      vR = sse_ifelse(vIf4, vR, vMid1);
      vR = sse_ifelse(vIf5, vR, vV);
      vR = sse_if(vIf0, vR);

      icl128 vG = vMid1;
      vG = sse_ifelse(vIf1, vG, vV);
      vG = sse_ifelse(vIf3, vG, vMid2);
      vG = sse_ifelse(vIf4, vG, vM);
      vG = sse_ifelse(vIf6, vG, vMid1);
      vG = sse_if(vIf0, vG);

      icl128 vB = vM;
      vB = sse_ifelse(vIf2, vB, vMid1);
      vB = sse_ifelse(vIf3, vB, vV);
      vB = sse_ifelse(vIf5, vB, vMid2);
      vB = sse_ifelse(vIf6, vB, vM);
      vB = sse_if(vIf0, vB);

      vR *= icl128(255.0f);
      vG *= icl128(255.0f);
      vB *= icl128(255.0f);

      // store the calculated values
      vR.storeu(r);
      vG.storeu(g);
      vB.storeu(b);
    }

    USE_SSE_CONVERT(icl8u,icl8u,HLS,RGB,subHLStoRGB,subSSEHLStoRGB,16)
    USE_SSE_CONVERT(icl8u,icl32f,HLS,RGB,subHLStoRGB,subSSEHLStoRGB,16)
    USE_SSE_CONVERT(icl32f,icl8u,HLS,RGB,subHLStoRGB,subSSEHLStoRGB,16)
    USE_SSE_CONVERT(icl32f,icl32f,HLS,RGB,subHLStoRGB,subSSEHLStoRGB,4)

    // -- HLS to RGB -- //


    // ++ HLS to YUV ++ //

    template<class S, class D>
    inline void subHLStoYUV(const S *h, const S *l, const S *s, D *y, D *u, D *v) {
      register icl32f reg_r(0), reg_g(0), reg_b(0);
      register icl32f reg_y, reg_u, reg_v;
      cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*h),
                                clipped_cast<S,icl32f>(*l),
                                clipped_cast<S,icl32f>(*s),
                                reg_r,reg_g,reg_b);
      cc_util_rgb_to_yuv(reg_r, reg_g, reg_b,
                         reg_y, reg_u, reg_v);
      *y = clipped_cast<icl32f,D>(reg_y);
      *u = clipped_cast<icl32f,D>(reg_u);
      *v = clipped_cast<icl32f,D>(reg_v);
    }

    template<class S>
    inline void subHLStoYUV(const S *h, const S *l, const S *s, icl8u *y, icl8u *u, icl8u *v) {
      register icl32f reg_r(0), reg_g(0), reg_b(0);
      register icl32f reg_y, reg_u, reg_v;
      cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*h),
                                clipped_cast<S,icl32f>(*l),
                                clipped_cast<S,icl32f>(*s),
                                reg_r,reg_g,reg_b);
      cc_util_rgb_to_yuv(reg_r, reg_g, reg_b,
                         reg_y, reg_u, reg_v);
      *y = clipped_cast<icl32f,icl8u>(reg_y + 0.5f);
      *u = clipped_cast<icl32f,icl8u>(reg_u + 0.5f);
      *v = clipped_cast<icl32f,icl8u>(reg_v + 0.5f);
    }

    template<class S, class D>
    inline void subSSEHLStoYUV(const S *h, const S *l, const S *s, D *y, D *u, D *v) {
      // load HLS values
      icl512 vH = icl512(h);
      icl512 vL = icl512(l);
      icl512 vS = icl512(s);

      vH *= icl512(6.0f/255.0f);
      vL *= icl512(1.0f/255.0f);
      vS *= icl512(1.0f/255.0f);

      icl512 vSL  = vL * vS;
      icl512 vLSL = vL + vS - vSL;
      icl512 vV = sse_ifelse(vL <= icl512(0.5f), vL + vSL, vLSL);

      icl512 vM = vL + vL - vV;
      icl512 vSV = (vV - vM) * icl512(vV).rcp();
      icl512 vST = icl512i(_mm_cvttps_epi32(vH.v0), _mm_cvttps_epi32(vH.v1),
                           _mm_cvttps_epi32(vH.v2), _mm_cvttps_epi32(vH.v3));
      icl512 vF = vH - vST;
      icl512 vVSF = vV * vSV * vF;
      icl512 vMid1 = vM + vVSF;
      icl512 vMid2 = vV - vVSF;

      icl512 vIf0 = (vV  > icl512(0.0f));
      icl512 vIf1 = (vST < icl512(1.0f));
      icl512 vIf2 = (vST < icl512(2.0f));
      icl512 vIf3 = (vST < icl512(3.0f));
      icl512 vIf4 = (vST < icl512(4.0f));
      icl512 vIf5 = (vST < icl512(5.0f));
      icl512 vIf6 = (vST < icl512(6.0f));

      icl512 vR = vV;
      vR = sse_ifelse(vIf1, vR, vMid2);
      vR = sse_ifelse(vIf2, vR, vM);
      vR = sse_ifelse(vIf4, vR, vMid1);
      vR = sse_ifelse(vIf5, vR, vV);
      vR = sse_if(vIf0, vR);

      icl512 vG = vMid1;
      vG = sse_ifelse(vIf1, vG, vV);
      vG = sse_ifelse(vIf3, vG, vMid2);
      vG = sse_ifelse(vIf4, vG, vM);
      vG = sse_ifelse(vIf6, vG, vMid1);
      vG = sse_if(vIf0, vG);

      icl512 vB = vM;
      vB = sse_ifelse(vIf2, vB, vMid1);
      vB = sse_ifelse(vIf3, vB, vV);
      vB = sse_ifelse(vIf5, vB, vMid2);
      vB = sse_ifelse(vIf6, vB, vM);
      vB = sse_if(vIf0, vB);

      vR *= icl512(255.0f);
      vG *= icl512(255.0f);
      vB *= icl512(255.0f);

      // calculate Y values
      icl512 vY(icl512(0.299f) * vR);
      vY += icl512(0.587f) * vG;
      vY += icl512(0.114f) * vB;

      // calculate U values
      vB -= vY;
      icl512 vU = icl512(0.492f) * vB;
      vU += icl512(128.0f);

      // calculate V values
      vR -= vY;
      icl512 vVV = icl512(0.877f) * vR;
      vVV += icl512(128.0f);

      // saturate values
      vU = max(vU, icl512(0.0f));
      vU = min(vU, icl512(255.0f));
      vVV = max(vVV, icl512(0.0f));
      vVV = min(vVV, icl512(255.0f));

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vVV.storeu(v);
    }

    template<class S>
    inline void subSSEHLStoYUV(const S *h, const S *l, const S *s, icl8u *y, icl8u *u, icl8u *v) {
      // load HLS values
      icl512 vH = icl512(h);
      icl512 vL = icl512(l);
      icl512 vS = icl512(s);

      vH *= icl512(6.0f/255.0f);
      vL *= icl512(1.0f/255.0f);
      vS *= icl512(1.0f/255.0f);

      icl512 vSL  = vL * vS;
      icl512 vLSL = vL + vS - vSL;
      icl512 vV = sse_ifelse(vL <= icl512(0.5f), vL + vSL, vLSL);

      icl512 vM = vL + vL - vV;
      icl512 vSV = (vV - vM) * icl512(vV).rcp();
      icl512 vST = icl512i(_mm_cvttps_epi32(vH.v0), _mm_cvttps_epi32(vH.v1),
                           _mm_cvttps_epi32(vH.v2), _mm_cvttps_epi32(vH.v3));
      icl512 vF = vH - vST;
      icl512 vVSF = vV * vSV * vF;
      icl512 vMid1 = vM + vVSF;
      icl512 vMid2 = vV - vVSF;

      icl512 vIf0 = (vV  > icl512(0.0f));
      icl512 vIf1 = (vST < icl512(1.0f));
      icl512 vIf2 = (vST < icl512(2.0f));
      icl512 vIf3 = (vST < icl512(3.0f));
      icl512 vIf4 = (vST < icl512(4.0f));
      icl512 vIf5 = (vST < icl512(5.0f));
      icl512 vIf6 = (vST < icl512(6.0f));

      icl512 vR = vV;
      vR = sse_ifelse(vIf1, vR, vMid2);
      vR = sse_ifelse(vIf2, vR, vM);
      vR = sse_ifelse(vIf4, vR, vMid1);
      vR = sse_ifelse(vIf5, vR, vV);
      vR = sse_if(vIf0, vR);

      icl512 vG = vMid1;
      vG = sse_ifelse(vIf1, vG, vV);
      vG = sse_ifelse(vIf3, vG, vMid2);
      vG = sse_ifelse(vIf4, vG, vM);
      vG = sse_ifelse(vIf6, vG, vMid1);
      vG = sse_if(vIf0, vG);

      icl512 vB = vM;
      vB = sse_ifelse(vIf2, vB, vMid1);
      vB = sse_ifelse(vIf3, vB, vV);
      vB = sse_ifelse(vIf5, vB, vMid2);
      vB = sse_ifelse(vIf6, vB, vM);
      vB = sse_if(vIf0, vB);

      vR *= icl512(255.0f);
      vG *= icl512(255.0f);
      vB *= icl512(255.0f);

      // calculate Y values
      icl512 vY(icl512(0.299f) * vR);
      vY += icl512(0.587f) * vG;
      vY += icl512(0.114f) * vB;

      // calculate U values
      vB -= vY;
      icl512 vU = icl512(0.492f) * vB;
      vU += icl512(128.0f);

      // calculate V values
      vR -= vY;
      icl512 vVV = icl512(0.877f) * vR;
      vVV += icl512(128.0f);

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vVV.storeu(v);
    }

    template<>
    inline void subSSEHLStoYUV(const icl32f *h, const icl32f *l, const icl32f *s,
                               icl32f *y, icl32f *u, icl32f *v) {
      // load HLS values
      icl128 vH = icl128(h);
      icl128 vL = icl128(l);
      icl128 vS = icl128(s);

      vH *= icl128(6.0f/255.0f);
      vL *= icl128(1.0f/255.0f);
      vS *= icl128(1.0f/255.0f);

      icl128 vSL  = vL * vS;
      icl128 vLSL = vL + vS - vSL;
      icl128 vV = sse_ifelse(vL <= icl128(0.5f), vL + vSL, vLSL);

      icl128 vM = vL + vL - vV;
      icl128 vSV = (vV - vM) * icl128(vV).rcp();
      icl128 vST = icl128i(_mm_cvttps_epi32(vH.v0));
      icl128 vF = vH - vST;
      icl128 vVSF = vV * vSV * vF;
      icl128 vMid1 = vM + vVSF;
      icl128 vMid2 = vV - vVSF;

      icl128 vIf0 = (vV  > icl128(0.0f));
      icl128 vIf1 = (vST < icl128(1.0f));
      icl128 vIf2 = (vST < icl128(2.0f));
      icl128 vIf3 = (vST < icl128(3.0f));
      icl128 vIf4 = (vST < icl128(4.0f));
      icl128 vIf5 = (vST < icl128(5.0f));
      icl128 vIf6 = (vST < icl128(6.0f));

      icl128 vR = vV;
      vR = sse_ifelse(vIf1, vR, vMid2);
      vR = sse_ifelse(vIf2, vR, vM);
      vR = sse_ifelse(vIf4, vR, vMid1);
      vR = sse_ifelse(vIf5, vR, vV);
      vR = sse_if(vIf0, vR);

      icl128 vG = vMid1;
      vG = sse_ifelse(vIf1, vG, vV);
      vG = sse_ifelse(vIf3, vG, vMid2);
      vG = sse_ifelse(vIf4, vG, vM);
      vG = sse_ifelse(vIf6, vG, vMid1);
      vG = sse_if(vIf0, vG);

      icl128 vB = vM;
      vB = sse_ifelse(vIf2, vB, vMid1);
      vB = sse_ifelse(vIf3, vB, vV);
      vB = sse_ifelse(vIf5, vB, vMid2);
      vB = sse_ifelse(vIf6, vB, vM);
      vB = sse_if(vIf0, vB);

      vR *= icl128(255.0f);
      vG *= icl128(255.0f);
      vB *= icl128(255.0f);

      // calculate Y values
      icl128 vY(icl128(0.299f) * vR);
      vY += icl128(0.587f) * vG;
      vY += icl128(0.114f) * vB;

      // calculate U values
      vB -= vY;
      icl128 vU = icl128(0.492f) * vB;
      vU += icl128(128.0f);

      // calculate V values
      vR -= vY;
      icl128 vVV = icl128(0.877f) * vR;
      vVV += icl128(128.0f);

      // saturate values
      vU = max(vU, icl128(0.0f));
      vU = min(vU, icl128(255.0f));
      vVV = max(vVV, icl128(0.0f));
      vVV = min(vVV, icl128(255.0f));

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vVV.storeu(v);
    }

    USE_SSE_CONVERT(icl8u,icl8u,HLS,YUV,subHLStoYUV,subSSEHLStoYUV,16)
    USE_SSE_CONVERT(icl8u,icl32f,HLS,YUV,subHLStoYUV,subSSEHLStoYUV,16)
    USE_SSE_CONVERT(icl32f,icl8u,HLS,YUV,subHLStoYUV,subSSEHLStoYUV,16)
    USE_SSE_CONVERT(icl32f,icl32f,HLS,YUV,subHLStoYUV,subSSEHLStoYUV,4)

    // -- HLS to YUV -- //


    // ++ HLS to Lab ++ //

    template<class S, class D>
    inline void subHLStoLab(const S *h, const S *l, const S *s, D *L, D *a, D *b) {
      register icl32f reg_r(0), reg_g(0), reg_b(0), reg_X, reg_Y, reg_Z;
      cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*h),
                                clipped_cast<S,icl32f>(*l),
                                clipped_cast<S,icl32f>(*s),
                                reg_r,reg_g,reg_b);
      cc_util_rgb_to_xyz_inline(reg_r, reg_g, reg_b,
                         reg_X, reg_Y, reg_Z);
      cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_r,reg_g,reg_b);
      *L = clipped_cast<icl32f,D>(reg_r);
      *a = clipped_cast<icl32f,D>(reg_g);
      *b = clipped_cast<icl32f,D>(reg_b);
    }

    template<class S>
    inline void subHLStoLab(const S *h, const S *l, const S *s, icl8u *L, icl8u *a, icl8u *b) {
      register icl32f reg_r(0), reg_g(0), reg_b(0), reg_X, reg_Y, reg_Z;
      cc_util_hls_to_rgb_inline(clipped_cast<S,icl32f>(*h),
                                clipped_cast<S,icl32f>(*l),
                                clipped_cast<S,icl32f>(*s),
                                reg_r,reg_g,reg_b);
      cc_util_rgb_to_xyz_inline(reg_r, reg_g, reg_b,
                         reg_X, reg_Y, reg_Z);
      cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_r,reg_g,reg_b);
      *L = clipped_cast<icl32f,icl8u>(reg_r + 0.5f);
      *a = clipped_cast<icl32f,icl8u>(reg_g + 0.5f);
      *b = clipped_cast<icl32f,icl8u>(reg_b + 0.5f);
    }

    template<class S, class D>
    inline void subSSEHLStoLab(const S *h, const S *l, const S *s, D *L, D *a, D *b) {
      // load HLS values
      icl512 vH = icl512(h);
      icl512 vL = icl512(l);
      icl512 vS = icl512(s);

      vH *= icl512(6.0f/255.0f);
      vL *= icl512(1.0f/255.0f);
      vS *= icl512(1.0f/255.0f);

      icl512 vSL  = vL * vS;
      icl512 vLSL = vL + vS - vSL;
      icl512 vV = sse_ifelse(vL <= icl512(0.5f), vL + vSL, vLSL);

      icl512 vM = vL + vL - vV;
      icl512 vSV = (vV - vM) * icl512(vV).rcp();
      icl512 vST = icl512i(_mm_cvttps_epi32(vH.v0), _mm_cvttps_epi32(vH.v1),
                           _mm_cvttps_epi32(vH.v2), _mm_cvttps_epi32(vH.v3));
      icl512 vF = vH - vST;
      icl512 vVSF = vV * vSV * vF;
      icl512 vMid1 = vM + vVSF;
      icl512 vMid2 = vV - vVSF;

      icl512 vIf0 = (vV  > icl512(0.0f));
      icl512 vIf1 = (vST < icl512(1.0f));
      icl512 vIf2 = (vST < icl512(2.0f));
      icl512 vIf3 = (vST < icl512(3.0f));
      icl512 vIf4 = (vST < icl512(4.0f));
      icl512 vIf5 = (vST < icl512(5.0f));
      icl512 vIf6 = (vST < icl512(6.0f));

      icl512 vR = vV;
      vR = sse_ifelse(vIf1, vR, vMid2);
      vR = sse_ifelse(vIf2, vR, vM);
      vR = sse_ifelse(vIf4, vR, vMid1);
      vR = sse_ifelse(vIf5, vR, vV);
      vR = sse_if(vIf0, vR);

      icl512 vG = vMid1;
      vG = sse_ifelse(vIf1, vG, vV);
      vG = sse_ifelse(vIf3, vG, vMid2);
      vG = sse_ifelse(vIf4, vG, vM);
      vG = sse_ifelse(vIf6, vG, vMid1);
      vG = sse_if(vIf0, vG);

      icl512 vB = vM;
      vB = sse_ifelse(vIf2, vB, vMid1);
      vB = sse_ifelse(vIf3, vB, vV);
      vB = sse_ifelse(vIf5, vB, vMid2);
      vB = sse_ifelse(vIf6, vB, vM);
      vB = sse_if(vIf0, vB);

      // RGB to XYZ
      icl512 x = icl512(0.412453f) * vR
               + icl512(0.35758f)  * vG
               + icl512(0.180423f) * vB;
      icl512 y = icl512(0.212671f) * vR
               + icl512(0.71516f)  * vG
               + icl512(0.072169f) * vB;
      icl512 z = icl512(0.019334f) * vR
               + icl512(0.119193f) * vG
               + icl512(0.950227f) * vB;

      x *= icl512(1.0f/0.950455f);
      z *= icl512(1.0f/1.088753f);

      icl512 fX = cbrt(x);
      icl512 fY = cbrt(y);
      icl512 fZ = cbrt(z);

      icl512 ifX = (x > icl512(0.008856f));
      icl512 ifY = (y > icl512(0.008856f));
      icl512 ifZ = (z > icl512(0.008856f));

      fX &= ifX;
      fX += andnot(icl512(7.787f) * x + icl512(16.0f/116.0f), ifX);
      fY &= ifY;
      fY += andnot(icl512(7.787f) * y + icl512(16.0f/116.0f), ifY);
      fZ &= ifZ;
      fZ += andnot(icl512(7.787f) * z + icl512(16.0f/116.0f), ifZ);

      icl512 vl = icl512(116.0f * 2.55f) * fY - icl512(16.0f * (255.0f / 100.0f));
      icl512 va = icl512(500.0f) * (fX - fY) + icl512(128.0f);
      icl512 vb = icl512(200.0f) * (fY - fZ) + icl512(128.0f);

      // store the calculated values
      vl.storeu(L);
      va.storeu(a);
      vb.storeu(b);
    }

    template<>
    inline void subSSEHLStoLab(const icl32f *h, const icl32f *l, const icl32f *s,
                               icl32f *L, icl32f *a, icl32f *b) {
      // load HLS values
      icl128 vH = icl128(h);
      icl128 vL = icl128(l);
      icl128 vS = icl128(s);

      vH *= icl128(6.0f/255.0f);
      vL *= icl128(1.0f/255.0f);
      vS *= icl128(1.0f/255.0f);

      icl128 vSL  = vL * vS;
      icl128 vLSL = vL + vS - vSL;
      icl128 vV = sse_ifelse(vL <= icl128(0.5f), vL + vSL, vLSL);

      icl128 vM = vL + vL - vV;
      icl128 vSV = (vV - vM) * icl128(vV).rcp();
      icl128 vST = icl128i(_mm_cvttps_epi32(vH.v0));
      icl128 vF = vH - vST;
      icl128 vVSF = vV * vSV * vF;
      icl128 vMid1 = vM + vVSF;
      icl128 vMid2 = vV - vVSF;

      icl128 vIf0 = (vV  > icl128(0.0f));
      icl128 vIf1 = (vST < icl128(1.0f));
      icl128 vIf2 = (vST < icl128(2.0f));
      icl128 vIf3 = (vST < icl128(3.0f));
      icl128 vIf4 = (vST < icl128(4.0f));
      icl128 vIf5 = (vST < icl128(5.0f));
      icl128 vIf6 = (vST < icl128(6.0f));

      icl128 vR = vV;
      vR = sse_ifelse(vIf1, vR, vMid2);
      vR = sse_ifelse(vIf2, vR, vM);
      vR = sse_ifelse(vIf4, vR, vMid1);
      vR = sse_ifelse(vIf5, vR, vV);
      vR = sse_if(vIf0, vR);

      icl128 vG = vMid1;
      vG = sse_ifelse(vIf1, vG, vV);
      vG = sse_ifelse(vIf3, vG, vMid2);
      vG = sse_ifelse(vIf4, vG, vM);
      vG = sse_ifelse(vIf6, vG, vMid1);
      vG = sse_if(vIf0, vG);

      icl128 vB = vM;
      vB = sse_ifelse(vIf2, vB, vMid1);
      vB = sse_ifelse(vIf3, vB, vV);
      vB = sse_ifelse(vIf5, vB, vMid2);
      vB = sse_ifelse(vIf6, vB, vM);
      vB = sse_if(vIf0, vB);

      // RGB to XYZ
      icl128 x = icl128(0.412453f) * vR
               + icl128(0.35758f)  * vG
               + icl128(0.180423f) * vB;
      icl128 y = icl128(0.212671f) * vR
               + icl128(0.71516f)  * vG
               + icl128(0.072169f) * vB;
      icl128 z = icl128(0.019334f) * vR
               + icl128(0.119193f) * vG
               + icl128(0.950227f) * vB;

      x *= icl128(1.0f/0.950455f);
      z *= icl128(1.0f/1.088753f);

      icl128 fX = cbrt(x);
      icl128 fY = cbrt(y);
      icl128 fZ = cbrt(z);

      icl128 ifX = (x > icl128(0.008856f));
      icl128 ifY = (y > icl128(0.008856f));
      icl128 ifZ = (z > icl128(0.008856f));

      fX &= ifX;
      fX += andnot(icl128(7.787f) * x + icl128(16.0f/116.0f), ifX);
      fY &= ifY;
      fY += andnot(icl128(7.787f) * y + icl128(16.0f/116.0f), ifY);
      fZ &= ifZ;
      fZ += andnot(icl128(7.787f) * z + icl128(16.0f/116.0f), ifZ);

      icl128 vl = icl128(116.0f * 2.55f) * fY - icl128(16.0f * (255.0f / 100.0f));
      icl128 va = icl128(500.0f) * (fX - fY) + icl128(128.0f);
      icl128 vb = icl128(200.0f) * (fY - fZ) + icl128(128.0f);

      // store the calculated values
      vl.storeu(L);
      va.storeu(a);
      vb.storeu(b);
    }

    USE_SSE_CONVERT(icl8u,icl8u,HLS,LAB,subHLStoLab,subSSEHLStoLab,16)
    USE_SSE_CONVERT(icl8u,icl32f,HLS,LAB,subHLStoLab,subSSEHLStoLab,16)
    USE_SSE_CONVERT(icl32f,icl8u,HLS,LAB,subHLStoLab,subSSEHLStoLab,16)
    USE_SSE_CONVERT(icl32f,icl32f,HLS,LAB,subHLStoLab,subSSEHLStoLab,4)

    // -- HLS to Lab -- //


    // ++ YUV to RGB ++ //

    template<class S, class D>
    inline void subYUVtoRGB(const S *y, const S *u, const S *v, D *r, D *g, D *b) {
      register icl32f reg_r, reg_g, reg_b;
      cc_util_yuv_to_rgb(clipped_cast<S,icl32f>(*y),
                         clipped_cast<S,icl32f>(*u),
                         clipped_cast<S,icl32f>(*v),
                         reg_r, reg_g, reg_b);
      *r = clipped_cast<icl32f,D>(reg_r);
      *g = clipped_cast<icl32f,D>(reg_g);
      *b = clipped_cast<icl32f,D>(reg_b);
    }

    template<class S>
    inline void subYUVtoRGB(const S *y, const S *u, const S *v, icl8u *r, icl8u *g, icl8u *b) {
      register icl32f reg_r, reg_g, reg_b;
      cc_util_yuv_to_rgb(clipped_cast<S,icl32f>(*y),
                         clipped_cast<S,icl32f>(*u),
                         clipped_cast<S,icl32f>(*v),
                         reg_r, reg_g, reg_b);
      *r = clipped_cast<icl32f,icl8u>(reg_r + 0.5f);
      *g = clipped_cast<icl32f,icl8u>(reg_g + 0.5f);
      *b = clipped_cast<icl32f,icl8u>(reg_b + 0.5f);
    }

    template<class S, class D>
    inline void subSSEYUVtoRGB(const S *y, const S *u, const S *v, D *r, D *g, D *b) {
      // load YUV values
      icl512 vY = icl512(y);
      icl512 vU = icl512(u);
      icl512 vV = icl512(v);

      vU -= icl512(128.0f);
      vV -= icl512(128.0f);

      icl512 vR = vY + icl512(1.140f) * vV;
      icl512 vG = vY - icl512(0.394f) * vU - icl512(0.581f) * vV;
      icl512 vB = vY + icl512(2.032f) * vU;

      // clip values
      vR = min(vR, icl512(255.0f));
      vR = max(vR, icl512(0.0f));
      vG = min(vG, icl512(255.0f));
      vG = max(vG, icl512(0.0f));
      vB = min(vB, icl512(255.0f));
      vB = max(vB, icl512(0.0f));

      // store the calculated values
      vR.storeu(r);
      vG.storeu(g);
      vB.storeu(b);
    }

    template<class S>
    inline void subSSEYUVtoRGB(const S *y, const S *u, const S *v,
                               icl8u *r, icl8u *g, icl8u *b) {
      // load YUV values
      icl512 vY = icl512(y);
      icl512 vU = icl512(u);
      icl512 vV = icl512(v);

      vU -= icl512(128.0f);
      vV -= icl512(128.0f);

      icl512 vR = vY + icl512(1.140f) * vV;
      icl512 vG = vY - icl512(0.394f) * vU - icl512(0.581f) * vV;
      icl512 vB = vY + icl512(2.032f) * vU;

      // store the calculated values
      vR.storeu(r);
      vG.storeu(g);
      vB.storeu(b);
    }

    template<>
    inline void subSSEYUVtoRGB(const icl32f *y, const icl32f *u, const icl32f *v,
                               icl32f *r, icl32f *g, icl32f *b) {
      // load YUV values
      icl128 vY = icl128(y);
      icl128 vU = icl128(u);
      icl128 vV = icl128(v);

      vU -= icl128(128.0f);
      vV -= icl128(128.0f);

      icl128 vR = vY + icl128(1.140f) * vV;
      icl128 vG = vY - icl128(0.394f) * vU - icl128(0.581f) * vV;
      icl128 vB = vY + icl128(2.032f) * vU;

      // clip values
      vR = min(vR, icl128(255.0f));
      vR = max(vR, icl128(0.0f));
      vG = min(vG, icl128(255.0f));
      vG = max(vG, icl128(0.0f));
      vB = min(vB, icl128(255.0f));
      vB = max(vB, icl128(0.0f));

      // store the calculated values
      vR.storeu(r);
      vG.storeu(g);
      vB.storeu(b);
    }

    USE_SSE_CONVERT(icl8u,icl8u,YUV,RGB,subYUVtoRGB,subSSEYUVtoRGB,16)
    USE_SSE_CONVERT(icl8u,icl32f,YUV,RGB,subYUVtoRGB,subSSEYUVtoRGB,16)
    USE_SSE_CONVERT(icl32f,icl8u,YUV,RGB,subYUVtoRGB,subSSEYUVtoRGB,16)
    USE_SSE_CONVERT(icl32f,icl32f,YUV,RGB,subYUVtoRGB,subSSEYUVtoRGB,4)

    // -- YUV to RGB -- //


    // ++ YUV to HLS ++ //

    template<class S, class D>
    inline void subYUVtoHLS(const S *y, const S *u, const S *v, D *h, D *l, D *s) {
      register icl32f reg_r, reg_g, reg_b, reg_h, reg_l, reg_s;
      cc_util_yuv_to_rgb(clipped_cast<S,icl32f>(*y),
                         clipped_cast<S,icl32f>(*u),
                         clipped_cast<S,icl32f>(*v),
                         reg_r, reg_g, reg_b);
      cc_util_rgb_to_hls(reg_r, reg_g, reg_b, reg_h, reg_l, reg_s);
      *h = clipped_cast<icl32f,D>(reg_h);
      *l = clipped_cast<icl32f,D>(reg_l);
      *s = clipped_cast<icl32f,D>(reg_s);
    }

    template<class S>
    inline void subYUVtoHLS(const S *y, const S *u, const S *v, icl8u *h, icl8u *l, icl8u *s) {
      register icl32f reg_r, reg_g, reg_b, reg_h, reg_l, reg_s;
      cc_util_yuv_to_rgb(clipped_cast<S,icl32f>(*y),
                         clipped_cast<S,icl32f>(*u),
                         clipped_cast<S,icl32f>(*v),
                         reg_r, reg_g, reg_b);
      cc_util_rgb_to_hls(reg_r, reg_g, reg_b, reg_h, reg_l, reg_s);
      *h = clipped_cast<icl32f,icl8u>(reg_h + 0.5f);
      *l = clipped_cast<icl32f,icl8u>(reg_l + 0.5f);
      *s = clipped_cast<icl32f,icl8u>(reg_s + 0.5f);
    }

    template<class S, class D>
    inline void subSSEYUVtoHLS(const S *y, const S *u, const S *v, D *h, D *l, D *s) {
      // load YUV values
      icl512 vY = icl512(y);
      icl512 vU = icl512(u);
      icl512 vV = icl512(v);

      vU -= icl512(128.0f);
      vV -= icl512(128.0f);

      icl512 vR = vY + icl512(1.140f) * vV;
      icl512 vG = vY - icl512(0.394f) * vU - icl512(0.581f) * vV;
      icl512 vB = vY + icl512(2.032f) * vU;

      // clip values
      vR = min(vR, icl512(255.0f));
      vR = max(vR, icl512(0.0f));
      vG = min(vG, icl512(255.0f));
      vG = max(vG, icl512(0.0f));
      vB = min(vB, icl512(255.0f));
      vB = max(vB, icl512(0.0f));

      // change the range to 0..1
      vR *= icl512(1.0f/255.0f);
      vG *= icl512(1.0f/255.0f);
      vB *= icl512(1.0f/255.0f);

      icl512 vMax = max(max(vR, vG), vB);
      icl512 vMin = min(min(vR, vG), vB);
      icl512 vMpM  = vMax + vMin;
      icl512 vMmM  = vMax - vMin;

      // calculate lightness
      icl512 vL = vMpM * icl512(0.5f);

      // calculate saturation

      icl512 vIf0 = (vL > icl512(0.5f));
      icl512 vIf1 = (vMin != vMax);

      icl512 vS = ((vMmM * (icl512(2.0f) - vMpM).rcp()) & vIf0);
      icl512 vS1 = andnot(vMmM * vMpM.rcp(), vIf0);

      vS += vS1;
      vS &= vIf1;
      vS &= (vL != icl512(0.0f));


      // calculate hue

      vMmM.rcp();

      icl512 vIf00 = (vR == vMax);
      icl512 vIf01 = andnot(vG == vMax, vIf00);
      icl512 vIf02 = andnot(andnot(vB == vMax, vIf01), vIf00);

      icl512 vH = (((vG - vB) * vMmM) & vIf00);
      vH += ((icl512(2.0f) + ((vB - vR) * vMmM)) & vIf01);
      vH += ((icl512(4.0f) + ((vR - vG) * vMmM)) & vIf02);
      vH *= icl512(60.0f);

      icl512 vIf10 = (vH < icl512(0.0f));
      vH += (icl512(360.0f) & vIf10);


      // TODO: maybe this is not important?
      vH &= (vL != icl512(0.0f));
      vH &= vIf1;

      // change the range of the calculated values to 0..255
      vH *= icl512(255.0f/360.0f);
      vL *= icl512(255.0f);
      vS *= icl512(255.0f);

      // store the calculated values
      vH.storeu(h);
      vL.storeu(l);
      vS.storeu(s);
    }

    template<>
    inline void subSSEYUVtoHLS(const icl32f *y, const icl32f *u, const icl32f *v,
                               icl32f *h, icl32f *l, icl32f *s) {
      // load YUV values
      icl128 vY = icl128(y);
      icl128 vU = icl128(u);
      icl128 vV = icl128(v);

      vU -= icl128(128.0f);
      vV -= icl128(128.0f);

      icl128 vR = vY + icl128(1.140f) * vV;
      icl128 vG = vY - icl128(0.394f) * vU - icl128(0.581f) * vV;
      icl128 vB = vY + icl128(2.032f) * vU;

      // clip values
      vR = min(vR, icl128(255.0f));
      vR = max(vR, icl128(0.0f));
      vG = min(vG, icl128(255.0f));
      vG = max(vG, icl128(0.0f));
      vB = min(vB, icl128(255.0f));
      vB = max(vB, icl128(0.0f));

      // change the range to 0..1
      vR *= icl128(1.0f/255.0f);
      vG *= icl128(1.0f/255.0f);
      vB *= icl128(1.0f/255.0f);

      icl128 vMax = max(max(vR, vG), vB);
      icl128 vMin = min(min(vR, vG), vB);
      icl128 vMpM  = vMax + vMin;
      icl128 vMmM  = vMax - vMin;

      // calculate lightness
      icl128 vL = vMpM * icl128(0.5f);

      // calculate saturation

      icl128 vIf0 = (vL > icl128(0.5f));
      icl128 vIf1 = (vMin != vMax);

      icl128 vS = ((vMmM * (icl128(2.0f) - vMpM).rcp()) & vIf0);
      icl128 vS1 = andnot(vMmM * vMpM.rcp(), vIf0);

      vS += vS1;
      vS &= vIf1;
      vS &= (vL != icl128(0.0f));


      // calculate hue

      vMmM.rcp();

      icl128 vIf00 = (vR == vMax);
      icl128 vIf01 = andnot(vG == vMax, vIf00);
      icl128 vIf02 = andnot(andnot(vB == vMax, vIf01), vIf00);

      icl128 vH = (((vG - vB) * vMmM) & vIf00);
      vH += ((icl128(2.0f) + ((vB - vR) * vMmM)) & vIf01);
      vH += ((icl128(4.0f) + ((vR - vG) * vMmM)) & vIf02);
      vH *= icl128(60.0f);

      icl128 vIf10 = (vH < icl128(0.0f));
      vH += (icl128(360.0f) & vIf10);


      // TODO: maybe this is not important?
      vH &= (vL != icl128(0.0f));
      vH &= vIf1;

      // change the range of the calculated values to 0..255
      vH *= icl128(255.0f/360.0f);
      vL *= icl128(255.0f);
      vS *= icl128(255.0f);

      // store the calculated values
      vH.storeu(h);
      vL.storeu(l);
      vS.storeu(s);
    }

    USE_SSE_CONVERT(icl8u,icl8u,YUV,HLS,subYUVtoHLS,subSSEYUVtoHLS,16)
    USE_SSE_CONVERT(icl8u,icl32f,YUV,HLS,subYUVtoHLS,subSSEYUVtoHLS,16)
    USE_SSE_CONVERT(icl32f,icl8u,YUV,HLS,subYUVtoHLS,subSSEYUVtoHLS,16)
    USE_SSE_CONVERT(icl32f,icl32f,YUV,HLS,subYUVtoHLS,subSSEYUVtoHLS,4)

    // -- YUV to HLS -- //


    // ++ YUV to Lab ++ //

    template<class S, class D>
    inline void subYUVtoLab(const S *y, const S *u, const S *v, D *l, D *a, D *b) {
      register icl32f reg_r, reg_g, reg_b, reg_X, reg_Y, reg_Z;
      cc_util_yuv_to_rgb(clipped_cast<S,icl32f>(*y),
                         clipped_cast<S,icl32f>(*u),
                         clipped_cast<S,icl32f>(*v),
                         reg_r, reg_g, reg_b);
      cc_util_rgb_to_xyz_inline(reg_r, reg_g, reg_b,
                         reg_X, reg_Y, reg_Z);
      cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_r,reg_g,reg_b);
      *l = clipped_cast<icl32f,D>(reg_r);
      *a = clipped_cast<icl32f,D>(reg_g);
      *b = clipped_cast<icl32f,D>(reg_b);
    }

    template<class S>
    inline void subYUVtoLab(const S *y, const S *u, const S *v, icl8u *l, icl8u *a, icl8u *b) {
      register icl32f reg_r, reg_g, reg_b, reg_X, reg_Y, reg_Z;
      cc_util_yuv_to_rgb(clipped_cast<S,icl32f>(*y),
                         clipped_cast<S,icl32f>(*u),
                         clipped_cast<S,icl32f>(*v),
                         reg_r, reg_g, reg_b);
      cc_util_rgb_to_xyz_inline(reg_r, reg_g, reg_b,
                         reg_X, reg_Y, reg_Z);
      cc_util_xyz_to_lab_inline(reg_X,reg_Y,reg_Z,reg_r,reg_g,reg_b);
      *l = clipped_cast<icl32f,icl8u>(reg_r + 0.5f);
      *a = clipped_cast<icl32f,icl8u>(reg_g + 0.5f);
      *b = clipped_cast<icl32f,icl8u>(reg_b + 0.5f);
    }

    template<class S, class D>
    inline void subSSEYUVtoLab(const S *Y, const S *U, const S *V, D *L, D *A, D *B) {
      // load YUV values
      icl512 vY = icl512(Y);
      icl512 vU = icl512(U);
      icl512 vV = icl512(V);

      vU -= icl512(128.0f);
      vV -= icl512(128.0f);

      icl512 vR = vY + icl512(1.140f) * vV;
      icl512 vG = vY - icl512(0.394f) * vU - icl512(0.581f) * vV;
      icl512 vB = vY + icl512(2.032f) * vU;

      // clip values
      vR = min(vR, icl512(255.0f));
      vR = max(vR, icl512(0.0f));
      vG = min(vG, icl512(255.0f));
      vG = max(vG, icl512(0.0f));
      vB = min(vB, icl512(255.0f));
      vB = max(vB, icl512(0.0f));

      // RGB to XYZ
      icl512 x = icl512(0.412453f/255.0f) * vR
               + icl512(0.35758f/255.0f)  * vG
               + icl512(0.180423f/255.0f) * vB;
      icl512 y = icl512(0.212671f/255.0f) * vR
               + icl512(0.71516f/255.0f)  * vG
               + icl512(0.072169f/255.0f) * vB;
      icl512 z = icl512(0.019334f/255.0f) * vR
               + icl512(0.119193f/255.0f) * vG
               + icl512(0.950227f/255.0f) * vB;

      x *= icl512(1.0f/0.950455f);
      z *= icl512(1.0f/1.088753f);

      icl512 fX = cbrt(x);
      icl512 fY = cbrt(y);
      icl512 fZ = cbrt(z);

      icl512 ifX = (x > icl512(0.008856f));
      icl512 ifY = (y > icl512(0.008856f));
      icl512 ifZ = (z > icl512(0.008856f));

      fX &= ifX;
      fX += andnot(icl512(7.787f) * x + icl512(16.0f/116.0f), ifX);
      fY &= ifY;
      fY += andnot(icl512(7.787f) * y + icl512(16.0f/116.0f), ifY);
      fZ &= ifZ;
      fZ += andnot(icl512(7.787f) * z + icl512(16.0f/116.0f), ifZ);

      icl512 vL = icl512(116.0f * 2.55f) * fY - icl512(16.0f * (255.0f / 100.0f));
      icl512 va = icl512(500.0f) * (fX - fY) + icl512(128.0f);
      icl512 vb = icl512(200.0f) * (fY - fZ) + icl512(128.0f);

      // store the calculated values
      vL.storeu(L);
      va.storeu(A);
      vb.storeu(B);
    }

    template<>
    inline void subSSEYUVtoLab(const icl32f *Y, const icl32f *U, const icl32f *V,
                               icl32f *L, icl32f *A, icl32f *B) {
      // load YUV values
      icl128 vY = icl128(Y);
      icl128 vU = icl128(U);
      icl128 vV = icl128(V);

      vU -= icl128(128.0f);
      vV -= icl128(128.0f);

      icl128 vR = vY + icl128(1.140f) * vV;
      icl128 vG = vY - icl128(0.394f) * vU - icl128(0.581f) * vV;
      icl128 vB = vY + icl128(2.032f) * vU;

      // clip values
      vR = min(vR, icl128(255.0f));
      vR = max(vR, icl128(0.0f));
      vG = min(vG, icl128(255.0f));
      vG = max(vG, icl128(0.0f));
      vB = min(vB, icl128(255.0f));
      vB = max(vB, icl128(0.0f));

      // RGB to XYZ
      icl128 x = icl128(0.412453f/255.0f) * vR
               + icl128(0.35758f/255.0f)  * vG
               + icl128(0.180423f/255.0f) * vB;
      icl128 y = icl128(0.212671f/255.0f) * vR
               + icl128(0.71516f/255.0f)  * vG
               + icl128(0.072169f/255.0f) * vB;
      icl128 z = icl128(0.019334f/255.0f) * vR
               + icl128(0.119193f/255.0f) * vG
               + icl128(0.950227f/255.0f) * vB;

      x *= icl128(1.0f/0.950455f);
      z *= icl128(1.0f/1.088753f);

      icl128 fX = cbrt(x);
      icl128 fY = cbrt(y);
      icl128 fZ = cbrt(z);

      icl128 ifX = (x > icl128(0.008856f));
      icl128 ifY = (y > icl128(0.008856f));
      icl128 ifZ = (z > icl128(0.008856f));

      fX &= ifX;
      fX += andnot(icl128(7.787f) * x + icl128(16.0f/116.0f), ifX);
      fY &= ifY;
      fY += andnot(icl128(7.787f) * y + icl128(16.0f/116.0f), ifY);
      fZ &= ifZ;
      fZ += andnot(icl128(7.787f) * z + icl128(16.0f/116.0f), ifZ);

      icl128 vL = icl128(116.0f * 2.55f) * fY - icl128(16.0f * (255.0f / 100.0f));
      icl128 va = icl128(500.0f) * (fX - fY) + icl128(128.0f);
      icl128 vb = icl128(200.0f) * (fY - fZ) + icl128(128.0f);

      // store the calculated values
      vL.storeu(L);
      va.storeu(A);
      vb.storeu(B);
    }

    USE_SSE_CONVERT(icl8u,icl8u,YUV,LAB,subYUVtoLab,subSSEYUVtoLab,16)
    USE_SSE_CONVERT(icl8u,icl32f,YUV,LAB,subYUVtoLab,subSSEYUVtoLab,16)
    USE_SSE_CONVERT(icl32f,icl8u,YUV,LAB,subYUVtoLab,subSSEYUVtoLab,16)
    USE_SSE_CONVERT(icl32f,icl32f,YUV,LAB,subYUVtoLab,subSSEYUVtoLab,4)

    // -- YUV to Lab -- //


    // ++ Lab to RGB ++ //

    template<class S, class D>
    inline void subLabtoRGB(const S *L, const S *A, const S *B, D *r, D *g, D *b) {
        register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
        cc_util_lab_to_xyz(clipped_cast<S,icl32f>(*L),
                           clipped_cast<S,icl32f>(*A),
                           clipped_cast<S,icl32f>(*B),
                           reg_x,reg_y,reg_z);
        cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
        *r = clipped_cast<icl32f,D>(reg_r);
        *g = clipped_cast<icl32f,D>(reg_g);
        *b = clipped_cast<icl32f,D>(reg_b);
    }

    template<class S>
    inline void subLabtoRGB(const S *L, const S *A, const S *B, icl8u *r, icl8u *g, icl8u *b) {
        register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
        cc_util_lab_to_xyz(clipped_cast<S,icl32f>(*L),
                           clipped_cast<S,icl32f>(*A),
                           clipped_cast<S,icl32f>(*B),
                           reg_x,reg_y,reg_z);
        cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
        *r = clipped_cast<icl32f,icl8u>(reg_r + 0.5f);
        *g = clipped_cast<icl32f,icl8u>(reg_g + 0.5f);
        *b = clipped_cast<icl32f,icl8u>(reg_b + 0.5f);
    }

    template<class S, class D>
    inline void subSSELabtoRGB(const S *L, const S *A, const S *B, D *r, D *g, D *b) {
      // load Lab values
      icl512 vL = icl512(L);
      icl512 vA = icl512(A);
      icl512 vB = icl512(B);

      icl512 vFy = (vL + icl512(16.0f*2.55f)) * icl512(1.0f/(2.55f*116.0f));
      icl512 vFx = vFy + (vA - icl512(128.0f)) * icl512(1/500.0f);
      icl512 vFz = vFy - (vB - icl512(128.0f)) * icl512(1/200.0f);

      icl512 vY0 = vFy * vFy * vFy;
      icl512 vX0 = vFx * vFx * vFx;
      icl512 vZ0 = vFz * vFz * vFz;
      icl512 vY1 = (vFy - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vX1 = (vFx - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vZ1 = (vFz - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      vY0 = sse_ifelse(vFy > icl512(0.206893f), vY0, vY1);
      vX0 = sse_ifelse(vFx > icl512(0.206893f), vX0, vX1);
      vZ0 = sse_ifelse(vFz > icl512(0.206893f), vZ0, vZ1);

      // white point correction
      vX0 *= icl512(0.950455f);
      vZ0 *= icl512(1.088754f);

      // XYZ to RGB
      icl512 vRr = icl512(3.240479*255.0f) * vX0
                 + icl512(-1.53715*255.0f)  * vY0
                 + icl512(-0.498535*255.0f) * vZ0;
      icl512 vRg = icl512(-0.969256*255.0f) * vX0
                 + icl512(1.875991*255.0f)  * vY0
                 + icl512(0.041556*255.0f) * vZ0;
      icl512 vRb = icl512(0.055648*255.0f) * vX0
                 + icl512(-0.204043*255.0f) * vY0
                 + icl512(1.057311*255.0f) * vZ0;

      // store the calculated values
      vRr.storeu(r);
      vRg.storeu(g);
      vRb.storeu(b);
    }

    template<>
    inline void subSSELabtoRGB(const icl32f *L, const icl32f *A, const icl32f *B,
                               icl32f *r, icl32f *g, icl32f *b) {
      // load Lab values
      icl128 vL = icl128(L);
      icl128 vA = icl128(A);
      icl128 vB = icl128(B);

      icl128 vFy = (vL + icl128(16.0f*2.55f)) * icl128(1.0f/(2.55f*116.0f));
      icl128 vFx = vFy + (vA - icl128(128.0f)) * icl128(1/500.0f);
      icl128 vFz = vFy - (vB - icl128(128.0f)) * icl128(1/200.0f);

      icl128 vY0 = vFy * vFy * vFy;
      icl128 vX0 = vFx * vFx * vFx;
      icl128 vZ0 = vFz * vFz * vFz;
      icl128 vY1 = (vFy - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      icl128 vX1 = (vFx - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      icl128 vZ1 = (vFz - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      vY0 = sse_ifelse(vFy > icl128(0.206893f), vY0, vY1);
      vX0 = sse_ifelse(vFx > icl128(0.206893f), vX0, vX1);
      vZ0 = sse_ifelse(vFz > icl128(0.206893f), vZ0, vZ1);

      // white point correction
      vX0 *= icl128(0.950455f);
      vZ0 *= icl128(1.088754f);

      // XYZ to RGB
      icl128 vRr = icl128(3.240479*255.0f) * vX0
                 + icl128(-1.53715*255.0f)  * vY0
                 + icl128(-0.498535*255.0f) * vZ0;
      icl128 vRg = icl128(-0.969256*255.0f) * vX0
                 + icl128(1.875991*255.0f)  * vY0
                 + icl128(0.041556*255.0f) * vZ0;
      icl128 vRb = icl128(0.055648*255.0f) * vX0
                 + icl128(-0.204043*255.0f) * vY0
                 + icl128(1.057311*255.0f) * vZ0;

      // store the calculated values
      vRr.storeu(r);
      vRg.storeu(g);
      vRb.storeu(b);
    }

    USE_SSE_CONVERT(icl8u,icl8u,LAB,RGB,subLabtoRGB,subSSELabtoRGB,16)
    USE_SSE_CONVERT(icl8u,icl32f,LAB,RGB,subLabtoRGB,subSSELabtoRGB,16)
    USE_SSE_CONVERT(icl32f,icl8u,LAB,RGB,subLabtoRGB,subSSELabtoRGB,16)
    USE_SSE_CONVERT(icl32f,icl32f,LAB,RGB,subLabtoRGB,subSSELabtoRGB,4)

    // -- Lab to RGB -- //


    // ++ Lab to HLS ++ //

    template<class S, class D>
    inline void subLabtoHLS(const S *L, const S *A, const S *B, D *h, D *l, D *s) {
      register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
      cc_util_lab_to_xyz(clipped_cast<S,icl32f>(*L),
                         clipped_cast<S,icl32f>(*A),
                         clipped_cast<S,icl32f>(*B),
                         reg_x,reg_y,reg_z);
      cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
      cc_util_rgb_to_hls(reg_r, reg_g, reg_b,
                         reg_x,reg_y,reg_z);
      *h = clipped_cast<icl32f,D>(reg_x);
      *l = clipped_cast<icl32f,D>(reg_y);
      *s = clipped_cast<icl32f,D>(reg_z);
    }

    template<class S>
    inline void subLabtoHLS(const S *L, const S *A, const S *B, icl8u *h, icl8u *l, icl8u *s) {
      register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
      cc_util_lab_to_xyz(clipped_cast<S,icl32f>(*L),
                         clipped_cast<S,icl32f>(*A),
                         clipped_cast<S,icl32f>(*B),
                         reg_x,reg_y,reg_z);
      cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
      cc_util_rgb_to_hls(reg_r, reg_g, reg_b,
                         reg_x,reg_y,reg_z);
      *h = clipped_cast<icl32f,icl8u>(reg_x + 0.5f);
      *l = clipped_cast<icl32f,icl8u>(reg_y + 0.5f);
      *s = clipped_cast<icl32f,icl8u>(reg_z + 0.5f);
    }

    template<class S, class D>
    inline void subSSELabtoHLS(const S *L, const S *A, const S *B, D *h, D *l, D *s) {
      // load Lab values
      icl512 vL = icl512(L);
      icl512 vA = icl512(A);
      icl512 vB = icl512(B);

      icl512 vFy = (vL + icl512(16.0f*2.55f)) * icl512(1.0f/(2.55f*116.0f));
      icl512 vFx = vFy + (vA - icl512(128.0f)) * icl512(1/500.0f);
      icl512 vFz = vFy - (vB - icl512(128.0f)) * icl512(1/200.0f);

      icl512 vY0 = vFy * vFy * vFy;
      icl512 vX0 = vFx * vFx * vFx;
      icl512 vZ0 = vFz * vFz * vFz;
      icl512 vY1 = (vFy - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vX1 = (vFx - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vZ1 = (vFz - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      vY0 = sse_ifelse(vFy > icl512(0.206893f), vY0, vY1);
      vX0 = sse_ifelse(vFx > icl512(0.206893f), vX0, vX1);
      vZ0 = sse_ifelse(vFz > icl512(0.206893f), vZ0, vZ1);

      // white point correction
      vX0 *= icl512(0.950455f);
      vZ0 *= icl512(1.088754f);

      // XYZ to RGB
      icl512 vRr = icl512(3.240479) * vX0
                 + icl512(-1.53715)  * vY0
                 + icl512(-0.498535) * vZ0;
      icl512 vRg = icl512(-0.969256) * vX0
                 + icl512(1.875991)  * vY0
                 + icl512(0.041556) * vZ0;
      icl512 vRb = icl512(0.055648) * vX0
                 + icl512(-0.204043) * vY0
                 + icl512(1.057311) * vZ0;

      icl512 vMax = max(max(vRr, vRg), vRb);
      icl512 vMin = min(min(vRr, vRg), vRb);
      icl512 vMpM  = vMax + vMin;
      icl512 vMmM  = vMax - vMin;

      // calculate lightness
      icl512 vLL = vMpM * icl512(0.5f);

      // calculate saturation

      icl512 vIf0 = (vLL > icl512(0.5f));
      icl512 vIf1 = (vMin != vMax);

      icl512 vS = ((vMmM * (icl512(2.0f) - vMpM).rcp()) & vIf0);
      icl512 vS1 = andnot(vMmM * vMpM.rcp(), vIf0);

      vS += vS1;
      vS &= vIf1;
      vS &= (vLL != icl512(0.0f));


      // calculate hue

      vMmM.rcp();

      icl512 vIf00 = (vRr == vMax);
      icl512 vIf01 = andnot(vRg == vMax, vIf00);
      icl512 vIf02 = andnot(andnot(vRb == vMax, vIf01), vIf00);

      icl512 vH = (((vRg - vRb) * vMmM) & vIf00);
      vH += ((icl512(2.0f) + ((vRb - vRr) * vMmM)) & vIf01);
      vH += ((icl512(4.0f) + ((vRr - vRg) * vMmM)) & vIf02);
      vH *= icl512(60.0f);

      icl512 vIf10 = (vH < icl512(0.0f));
      vH += (icl512(360.0f) & vIf10);


      // TODO: maybe this is not important?
      vH &= (vLL != icl512(0.0f));
      vH &= vIf1;

      // change the range of the calculated values to 0..255
      vH *= icl512(255.0f/360.0f);
      vLL*= icl512(255.0f);
      vS *= icl512(255.0f);

      // store the calculated values
      vH.storeu(h);
      vLL.storeu(l);
      vS.storeu(s);
    }

    template<>
    inline void subSSELabtoHLS(const icl32f *L, const icl32f *A, const icl32f *B,
                               icl32f *h, icl32f *l, icl32f *s) {
      // load Lab values
      icl128 vL = icl128(L);
      icl128 vA = icl128(A);
      icl128 vB = icl128(B);

      icl128 vFy = (vL + icl128(16.0f*2.55f)) * icl128(1.0f/(2.55f*116.0f));
      icl128 vFx = vFy + (vA - icl128(128.0f)) * icl128(1/500.0f);
      icl128 vFz = vFy - (vB - icl128(128.0f)) * icl128(1/200.0f);

      icl128 vY0 = vFy * vFy * vFy;
      icl128 vX0 = vFx * vFx * vFx;
      icl128 vZ0 = vFz * vFz * vFz;
      icl128 vY1 = (vFy - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      icl128 vX1 = (vFx - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      icl128 vZ1 = (vFz - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      vY0 = sse_ifelse(vFy > icl128(0.206893f), vY0, vY1);
      vX0 = sse_ifelse(vFx > icl128(0.206893f), vX0, vX1);
      vZ0 = sse_ifelse(vFz > icl128(0.206893f), vZ0, vZ1);

      // white point correction
      vX0 *= icl128(0.950455f);
      vZ0 *= icl128(1.088754f);

      // XYZ to RGB
      icl128 vRr = icl128(3.240479) * vX0
                 + icl128(-1.53715)  * vY0
                 + icl128(-0.498535) * vZ0;
      icl128 vRg = icl128(-0.969256) * vX0
                 + icl128(1.875991)  * vY0
                 + icl128(0.041556) * vZ0;
      icl128 vRb = icl128(0.055648) * vX0
                 + icl128(-0.204043) * vY0
                 + icl128(1.057311) * vZ0;

      icl128 vMax = max(max(vRr, vRg), vRb);
      icl128 vMin = min(min(vRr, vRg), vRb);
      icl128 vMpM  = vMax + vMin;
      icl128 vMmM  = vMax - vMin;

      // calculate lightness
      icl128 vLL = vMpM * icl128(0.5f);

      // calculate saturation

      icl128 vIf0 = (vLL > icl128(0.5f));
      icl128 vIf1 = (vMin != vMax);

      icl128 vS = ((vMmM * (icl128(2.0f) - vMpM).rcp()) & vIf0);
      icl128 vS1 = andnot(vMmM * vMpM.rcp(), vIf0);

      vS += vS1;
      vS &= vIf1;
      vS &= (vLL != icl128(0.0f));


      // calculate hue

      vMmM.rcp();

      icl128 vIf00 = (vRr == vMax);
      icl128 vIf01 = andnot(vRg == vMax, vIf00);
      icl128 vIf02 = andnot(andnot(vRb == vMax, vIf01), vIf00);

      icl128 vH = (((vRg - vRb) * vMmM) & vIf00);
      vH += ((icl128(2.0f) + ((vRb - vRr) * vMmM)) & vIf01);
      vH += ((icl128(4.0f) + ((vRr - vRg) * vMmM)) & vIf02);
      vH *= icl128(60.0f);

      icl128 vIf10 = (vH < icl128(0.0f));
      vH += (icl128(360.0f) & vIf10);


      // TODO: maybe this is not important?
      vH &= (vLL != icl128(0.0f));
      vH &= vIf1;

      // change the range of the calculated values to 0..255
      vH *= icl128(255.0f/360.0f);
      vLL *= icl128(255.0f);
      vS *= icl128(255.0f);

      // store the calculated values
      vH.storeu(h);
      vLL.storeu(l);
      vS.storeu(s);
    }

    USE_SSE_CONVERT(icl8u,icl8u,LAB,HLS,subLabtoHLS,subSSELabtoHLS,16)
    USE_SSE_CONVERT(icl8u,icl32f,LAB,HLS,subLabtoHLS,subSSELabtoHLS,16)
    USE_SSE_CONVERT(icl32f,icl8u,LAB,HLS,subLabtoHLS,subSSELabtoHLS,16)
    USE_SSE_CONVERT(icl32f,icl32f,LAB,HLS,subLabtoHLS,subSSELabtoHLS,4)

    // -- Lab to HLS -- //


    // ++ Lab to YUV ++ //

    template<class S, class D>
    inline void subLabtoYUV(const S *L, const S *A, const S *B, D *y, D *u, D *v) {
      register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
      cc_util_lab_to_xyz(clipped_cast<S,icl32f>(*L),
                         clipped_cast<S,icl32f>(*A),
                         clipped_cast<S,icl32f>(*B),
                         reg_x,reg_y,reg_z);
      cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
      cc_util_rgb_to_yuv(reg_r, reg_g, reg_b,
                         reg_x, reg_y, reg_z);
      *y = clipped_cast<icl32f,D>(reg_x);
      *u = clipped_cast<icl32f,D>(reg_y);
      *v = clipped_cast<icl32f,D>(reg_z);
    }

    template<class S>
    inline void subLabtoYUV(const S *L, const S *A, const S *B, icl8u *y, icl8u *u, icl8u *v) {
      register icl32f reg_x, reg_y, reg_z, reg_r, reg_g, reg_b;
      cc_util_lab_to_xyz(clipped_cast<S,icl32f>(*L),
                         clipped_cast<S,icl32f>(*A),
                         clipped_cast<S,icl32f>(*B),
                         reg_x,reg_y,reg_z);
      cc_util_xyz_to_rgb(reg_x,reg_y,reg_z,reg_r,reg_g,reg_b);
      cc_util_rgb_to_yuv(reg_r, reg_g, reg_b,
                         reg_x, reg_y, reg_z);
      *y = clipped_cast<icl32f,icl8u>(reg_x + 0.5f);
      *u = clipped_cast<icl32f,icl8u>(reg_y + 0.5f);
      *v = clipped_cast<icl32f,icl8u>(reg_z + 0.5f);
    }

    template<class S, class D>
    inline void subSSELabtoYUV(const S *L, const S *A, const S *B, D *y, D *u, D *v) {
      // load Lab values
      icl512 vL = icl512(L);
      icl512 vA = icl512(A);
      icl512 vB = icl512(B);

      icl512 vFy = (vL + icl512(16.0f*2.55f)) * icl512(1.0f/(2.55f*116.0f));
      icl512 vFx = vFy + (vA - icl512(128.0f)) * icl512(1/500.0f);
      icl512 vFz = vFy - (vB - icl512(128.0f)) * icl512(1/200.0f);

      icl512 vY0 = vFy * vFy * vFy;
      icl512 vX0 = vFx * vFx * vFx;
      icl512 vZ0 = vFz * vFz * vFz;
      icl512 vY1 = (vFy - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vX1 = (vFx - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vZ1 = (vFz - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      vY0 = sse_ifelse(vFy > icl512(0.206893f), vY0, vY1);
      vX0 = sse_ifelse(vFx > icl512(0.206893f), vX0, vX1);
      vZ0 = sse_ifelse(vFz > icl512(0.206893f), vZ0, vZ1);

      // white point correction
      vX0 *= icl512(0.950455f);
      vZ0 *= icl512(1.088754f);

      // XYZ to RGB
      icl512 vRr = icl512(3.240479*255.0f) * vX0
                 + icl512(-1.53715*255.0f)  * vY0
                 + icl512(-0.498535*255.0f) * vZ0;
      icl512 vRg = icl512(-0.969256*255.0f) * vX0
                 + icl512(1.875991*255.0f)  * vY0
                 + icl512(0.041556*255.0f) * vZ0;
      icl512 vRb = icl512(0.055648*255.0f) * vX0
                 + icl512(-0.204043*255.0f) * vY0
                 + icl512(1.057311*255.0f) * vZ0;

      // calculate Y values
      icl512 vY(icl512(0.299f) * vRr);
      vY += icl512(0.587f) * vRg;
      vY += icl512(0.114f) * vRb;

      // calculate U values
      vRb -= vY;
      icl512 vU = icl512(0.492f) * vRb;
      vU += icl512(128.0f);

      // calculate V values
      vRr -= vY;
      icl512 vV = icl512(0.877f) * vRr;
      vV += icl512(128.0f);

      // saturate values
      vU = max(vU, icl512(0.0f));
      vU = min(vU, icl512(255.0f));
      vV = max(vV, icl512(0.0f));
      vV = min(vV, icl512(255.0f));

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vV.storeu(v);
    }

    template<class S>
    inline void subSSELabtoYUV(const S *L, const S *A, const S *B, icl8u *y, icl8u *u, icl8u *v) {
      // load Lab values
      icl512 vL = icl512(L);
      icl512 vA = icl512(A);
      icl512 vB = icl512(B);

      icl512 vFy = (vL + icl512(16.0f*2.55f)) * icl512(1.0f/(2.55f*116.0f));
      icl512 vFx = vFy + (vA - icl512(128.0f)) * icl512(1/500.0f);
      icl512 vFz = vFy - (vB - icl512(128.0f)) * icl512(1/200.0f);

      icl512 vY0 = vFy * vFy * vFy;
      icl512 vX0 = vFx * vFx * vFx;
      icl512 vZ0 = vFz * vFz * vFz;
      icl512 vY1 = (vFy - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vX1 = (vFx - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      icl512 vZ1 = (vFz - icl512(16.0f/116.0f)) * icl512(1.0f/7.787f);
      vY0 = sse_ifelse(vFy > icl512(0.206893f), vY0, vY1);
      vX0 = sse_ifelse(vFx > icl512(0.206893f), vX0, vX1);
      vZ0 = sse_ifelse(vFz > icl512(0.206893f), vZ0, vZ1);

      // white point correction
      vX0 *= icl512(0.950455f);
      vZ0 *= icl512(1.088754f);

      // XYZ to RGB
      icl512 vRr = icl512(3.240479*255.0f) * vX0
                 + icl512(-1.53715*255.0f)  * vY0
                 + icl512(-0.498535*255.0f) * vZ0;
      icl512 vRg = icl512(-0.969256*255.0f) * vX0
                 + icl512(1.875991*255.0f)  * vY0
                 + icl512(0.041556*255.0f) * vZ0;
      icl512 vRb = icl512(0.055648*255.0f) * vX0
                 + icl512(-0.204043*255.0f) * vY0
                 + icl512(1.057311*255.0f) * vZ0;

      // calculate Y values
      icl512 vY(icl512(0.299f) * vRr);
      vY += icl512(0.587f) * vRg;
      vY += icl512(0.114f) * vRb;

      // calculate U values
      vRb -= vY;
      icl512 vU = icl512(0.492f) * vRb;
      vU += icl512(128.0f);

      // calculate V values
      vRr -= vY;
      icl512 vV = icl512(0.877f) * vRr;
      vV += icl512(128.0f);

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vV.storeu(v);
    }

    template<>
    inline void subSSELabtoYUV(const icl32f *L, const icl32f *A, const icl32f *B,
                               icl32f *y, icl32f *u, icl32f *v) {
      // load Lab values
      icl128 vL = icl128(L);
      icl128 vA = icl128(A);
      icl128 vB = icl128(B);

      icl128 vFy = (vL + icl128(16.0f*2.55f)) * icl128(1.0f/(2.55f*116.0f));
      icl128 vFx = vFy + (vA - icl128(128.0f)) * icl128(1/500.0f);
      icl128 vFz = vFy - (vB - icl128(128.0f)) * icl128(1/200.0f);

      icl128 vY0 = vFy * vFy * vFy;
      icl128 vX0 = vFx * vFx * vFx;
      icl128 vZ0 = vFz * vFz * vFz;
      icl128 vY1 = (vFy - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      icl128 vX1 = (vFx - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      icl128 vZ1 = (vFz - icl128(16.0f/116.0f)) * icl128(1.0f/7.787f);
      vY0 = sse_ifelse(vFy > icl128(0.206893f), vY0, vY1);
      vX0 = sse_ifelse(vFx > icl128(0.206893f), vX0, vX1);
      vZ0 = sse_ifelse(vFz > icl128(0.206893f), vZ0, vZ1);

      // white point correction
      vX0 *= icl128(0.950455f);
      vZ0 *= icl128(1.088754f);

      // XYZ to RGB
      icl128 vRr = icl128(3.240479*255.0f) * vX0
                 + icl128(-1.53715*255.0f)  * vY0
                 + icl128(-0.498535*255.0f) * vZ0;
      icl128 vRg = icl128(-0.969256*255.0f) * vX0
                 + icl128(1.875991*255.0f)  * vY0
                 + icl128(0.041556*255.0f) * vZ0;
      icl128 vRb = icl128(0.055648*255.0f) * vX0
                 + icl128(-0.204043*255.0f) * vY0
                 + icl128(1.057311*255.0f) * vZ0;

      // calculate Y values
      icl128 vY(icl128(0.299f) * vRr);
      vY += icl128(0.587f) * vRg;
      vY += icl128(0.114f) * vRb;

      // calculate U values
      vRb -= vY;
      icl128 vU = icl128(0.492f) * vRb;
      vU += icl128(128.0f);

      // calculate V values
      vRr -= vY;
      icl128 vV = icl128(0.877f) * vRr;
      vV += icl128(128.0f);

      // saturate values
      vU = max(vU, icl128(0.0f));
      vU = min(vU, icl128(255.0f));
      vV = max(vV, icl128(0.0f));
      vV = min(vV, icl128(255.0f));

      // store the calculated values
      vY.storeu(y);
      vU.storeu(u);
      vV.storeu(v);
    }

    USE_SSE_CONVERT(icl8u,icl8u,LAB,YUV,subLabtoYUV,subSSELabtoYUV,16)
    USE_SSE_CONVERT(icl8u,icl32f,LAB,YUV,subLabtoYUV,subSSELabtoYUV,16)
    USE_SSE_CONVERT(icl32f,icl8u,LAB,YUV,subLabtoYUV,subSSELabtoYUV,16)
    USE_SSE_CONVERT(icl32f,icl32f,LAB,YUV,subLabtoYUV,subSSELabtoYUV,4)

    // -- Lab to YUV -- //

  #endif
  
  
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

      if(src->getFormat() == dst->getFormat()){
        if(roiOnly){
          src->convertROI(dst);
        }else{
          src->convert(dst);
        }
        return;
      }

      dst->setTime(src->getTime());
  
      if(roiOnly){
        /// check for equal roi sizes
        ICLASSERT_RETURN(src->getROISize() == dst->getROISize());
      }else{
        /// adapt the size of the destination image
        dst->setSize(src->getSize());
        dst->setROI(src->getROI());
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
      dst->setMetaData(src->getMetaData());
    }
  
    // }}}

    template<class S, class D>
    void for_copy_p2c2(const S *s0, const S *s1, D *dst, const D *dstEnd) {
      while (dst<dstEnd){
        *dst++ = clipped_cast<S,D>(*s0++);
        *dst++ = clipped_cast<S,D>(*s1++);
      }
    }

    template<class S, class D>
    void for_copy_p3c3(const S *s0, const S *s1, const S *s2, D *dst, const D *dstEnd) {
      while (dst<dstEnd){
        *dst++ = clipped_cast<S,D>(*s0++);
        *dst++ = clipped_cast<S,D>(*s1++);
        *dst++ = clipped_cast<S,D>(*s2++);
      }
    }

    template<class S, class D>
    void for_copy_p4c4(const S *s0, const S *s1, const S *s2, const S *s3, D *dst, const D *dstEnd) {
      while (dst<dstEnd){
        *dst++ = clipped_cast<S,D>(*s0++);
        *dst++ = clipped_cast<S,D>(*s1++);
        *dst++ = clipped_cast<S,D>(*s2++);
        *dst++ = clipped_cast<S,D>(*s3++);
      }
    }

  #ifdef ICL_HAVE_SSSE3

      template<class S, class D>
      inline void copy_p3c3(const S *src0, const S *src1, const S *src2, D *dst) {
        *dst     = clipped_cast<S,D>(*src0);
        *(dst+1) = clipped_cast<S,D>(*src1);
        *(dst+2) = clipped_cast<S,D>(*src2);
      }

      inline void sse_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, icl8u *dst) {
        const icl128i mask = icl128i(0x04010200, 0x0A080506, 0x0D0E0C09, 0x80808080);

        icl128i v0 = icl128i(src0);
        icl128i v1 = icl128i(src1);
        icl128i v2 = icl128i(src2);
        icl128i v3 = icl128i(0);

        __m128i vl0 = _mm_unpacklo_epi8(v0.v0, v2.v0);
        __m128i vh0 = _mm_unpackhi_epi8(v0.v0, v2.v0);
        __m128i vl1 = _mm_unpacklo_epi8(v1.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi8(v1.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi16(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi16(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi16(vh0, vh1);
        v3.v0 = _mm_unpackhi_epi16(vh0, vh1);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        v0.storeu(dst);
        v1.storeu(dst+12);
        v2.storeu(dst+24);
        v3.storeu(dst+36);

        /* this function can be improved with SSE4;
           mask_0..mask_5 have to be defined

        icl128i vR0 = icl128i(_mm_blendv_epi8(v0.v0, v1.v0, mask_0));
        vR0 = icl128i(_mm_blendv_epi8(vR0.v0, v2.v0, mask_1));
        icl128i vR1 = icl128i(_mm_blendv_epi8(v0.v0, v1.v0, mask_2));
        vR0 = icl128i(_mm_blendv_epi8(vR1.v0, v2.v0, mask_3));
        icl128i vR2 = icl128i(_mm_blendv_epi8(v0.v0, v1.v0, mask_4));
        vR0 = icl128i(_mm_blendv_epi8(vR2.v0, v2.v0, mask_5));

        vR0.storeu(dst);
        vR1.storeu(dst+16);
        vR2.storeu(dst+32);
        */
      }

      inline void sse_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, icl16s *dst) {
        const icl128i mask = icl128i(0x04010200, 0x0A080506, 0x0D0E0C09, 0x80808080);
        const __m128i vk0 = _mm_setzero_si128();

        icl128i v0 = icl128i(src0);
        icl128i v1 = icl128i(src1);
        icl128i v2 = icl128i(src2);
        icl128i v3 = icl128i(0);

        __m128i vl0 = _mm_unpacklo_epi8(v0.v0, v2.v0);
        __m128i vh0 = _mm_unpackhi_epi8(v0.v0, v2.v0);
        __m128i vl1 = _mm_unpacklo_epi8(v1.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi8(v1.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi16(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi16(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi16(vh0, vh1);
        v3.v0 = _mm_unpackhi_epi16(vh0, vh1);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vt1, vt3;

        vt1 = _mm_unpacklo_epi8(v0.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v0.v0, vk0);

        _mm_storeu_si128((__m128i*)dst,      vt1);
        _mm_storeu_si128((__m128i*)(dst+8),  vt3);

        vt1 = _mm_unpacklo_epi8(v1.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v1.v0, vk0);

        _mm_storeu_si128((__m128i*)(dst+12), vt1);
        _mm_storeu_si128((__m128i*)(dst+20), vt3);

        vt1 = _mm_unpacklo_epi8(v2.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v2.v0, vk0);

        _mm_storeu_si128((__m128i*)(dst+24), vt1);
        _mm_storeu_si128((__m128i*)(dst+32), vt3);

        vt1 = _mm_unpacklo_epi8(v3.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v3.v0, vk0);

        _mm_storeu_si128((__m128i*)(dst+36), vt1);
        _mm_storeu_si128((__m128i*)(dst+44), vt3);
      }

      inline void sse_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, __m128i *dst) {
        const icl128i mask = icl128i(0x04010200, 0x0A080506, 0x0D0E0C09, 0x80808080);
        const __m128i vk0 = _mm_setzero_si128();

        icl128i v0 = icl128i(src0);
        icl128i v1 = icl128i(src1);
        icl128i v2 = icl128i(src2);
        icl128i v3 = icl128i(0);

        __m128i vl0 = _mm_unpacklo_epi8(v0.v0, v2.v0);
        __m128i vh0 = _mm_unpackhi_epi8(v0.v0, v2.v0);
        __m128i vl1 = _mm_unpacklo_epi8(v1.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi8(v1.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi16(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi16(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi16(vh0, vh1);
        v3.v0 = _mm_unpackhi_epi16(vh0, vh1);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vt0, vt1, vt2, vt3;

        vt1 = _mm_unpacklo_epi8(v0.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v0.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        _mm_storeu_si128(dst++, vt0);
        _mm_storeu_si128(dst++, vt1);
        _mm_storeu_si128(dst++, vt2);

        vt1 = _mm_unpacklo_epi8(v1.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v1.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        _mm_storeu_si128(dst++, vt0);
        _mm_storeu_si128(dst++, vt1);
        _mm_storeu_si128(dst++, vt2);

        vt1 = _mm_unpacklo_epi8(v2.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v2.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        _mm_storeu_si128(dst++, vt0);
        _mm_storeu_si128(dst++, vt1);
        _mm_storeu_si128(dst++, vt2);

        vt1 = _mm_unpacklo_epi8(v3.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v3.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        _mm_storeu_si128(dst++, vt0);
        _mm_storeu_si128(dst++, vt1);
        _mm_storeu_si128(dst++, vt2);
      }

      inline void sse_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, icl32f *dst) {
        const icl128i mask = icl128i(0x04010200, 0x0A080506, 0x0D0E0C09, 0x80808080);
        const __m128i vk0 = _mm_setzero_si128();

        icl128i v0 = icl128i(src0);
        icl128i v1 = icl128i(src1);
        icl128i v2 = icl128i(src2);
        icl128i v3 = icl128i(0);

        __m128i vl0 = _mm_unpacklo_epi8(v0.v0, v2.v0);
        __m128i vh0 = _mm_unpackhi_epi8(v0.v0, v2.v0);
        __m128i vl1 = _mm_unpacklo_epi8(v1.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi8(v1.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi16(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi16(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi16(vh0, vh1);
        v3.v0 = _mm_unpackhi_epi16(vh0, vh1);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vt0, vt1, vt2, vt3;
        __m128 vf0, vf1, vf2;

        vt1 = _mm_unpacklo_epi8(v0.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v0.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        vf0 = _mm_cvtepi32_ps(vt0);
        vf1 = _mm_cvtepi32_ps(vt1);
        vf2 = _mm_cvtepi32_ps(vt2);

        _mm_storeu_ps(dst,    vf0);
        _mm_storeu_ps(dst+4,  vf1);
        _mm_storeu_ps(dst+8,  vf2);

        vt1 = _mm_unpacklo_epi8(v1.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v1.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        vf0 = _mm_cvtepi32_ps(vt0);
        vf1 = _mm_cvtepi32_ps(vt1);
        vf2 = _mm_cvtepi32_ps(vt2);

        _mm_storeu_ps(dst+12, vf0);
        _mm_storeu_ps(dst+16, vf1);
        _mm_storeu_ps(dst+20, vf2);

        vt1 = _mm_unpacklo_epi8(v2.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v2.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        vf0 = _mm_cvtepi32_ps(vt0);
        vf1 = _mm_cvtepi32_ps(vt1);
        vf2 = _mm_cvtepi32_ps(vt2);

        _mm_storeu_ps(dst+24, vf0);
        _mm_storeu_ps(dst+28, vf1);
        _mm_storeu_ps(dst+32, vf2);

        vt1 = _mm_unpacklo_epi8(v3.v0, vk0);
        vt3 = _mm_unpackhi_epi8(v3.v0, vk0);

        vt0 = _mm_unpacklo_epi16(vt1, vk0);
        vt1 = _mm_unpackhi_epi16(vt1, vk0);
        vt2 = _mm_unpacklo_epi16(vt3, vk0);

        vf0 = _mm_cvtepi32_ps(vt0);
        vf1 = _mm_cvtepi32_ps(vt1);
        vf2 = _mm_cvtepi32_ps(vt2);

        _mm_storeu_ps(dst+36, vf0);
        _mm_storeu_ps(dst+40, vf1);
        _mm_storeu_ps(dst+44, vf2);
      }

    void for_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, icl8u *dst, icl8u *dstEnd) {
      icl8u *dstSSEEnd = dstEnd - 55;

      for (; dst<dstSSEEnd;) {
          // convert 'rvalues' values at the same time
          sse_copy_p3c3(src0, src1, src2, dst);

          // increment pointers to the next values
          src0 += 16;
          src1 += 16;
          src2 += 16;
          dst += 48;
      }

      for (; dst<dstEnd; ++src0, ++src1, ++src2, dst += 3) {
        // convert 1 value
        copy_p3c3(src0, src1, src2, dst);
      }
    }

    void for_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, icl16s *dst, icl16s *dstEnd) {
      icl16s *dstSSEEnd = dstEnd - 55;

      for (; dst<dstSSEEnd;) {
          // convert 'rvalues' values at the same time
          sse_copy_p3c3(src0, src1, src2, dst);

          // increment pointers to the next values
          src0 += 16;
          src1 += 16;
          src2 += 16;
          dst += 48;
      }

      for (; dst<dstEnd; ++src0, ++src1, ++src2, dst += 3) {
        // convert 1 value
        copy_p3c3(src0, src1, src2, dst);
      }
    }

    void for_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, icl32s *dst, icl32s *dstEnd) {
      icl32s *dstSSEEnd = dstEnd - 49;

      for (; dst<dstSSEEnd;) {
          // convert 'rvalues' values at the same time
          sse_copy_p3c3(src0, src1, src2, (__m128i*)dst);

          // increment pointers to the next values
          src0 += 16;
          src1 += 16;
          src2 += 16;
          dst += 48;
      }

      for (; dst<dstEnd; ++src0, ++src1, ++src2, dst += 3) {
        // convert 1 value
        copy_p3c3(src0, src1, src2, dst);
      }
    }

    void for_copy_p3c3(const icl8u *src0, const icl8u *src1, const icl8u *src2, icl32f *dst, icl32f *dstEnd) {
      icl32f *dstSSEEnd = dstEnd - 49;

      for (; dst<dstSSEEnd;) {
          // convert 'rvalues' values at the same time
          sse_copy_p3c3(src0, src1, src2, dst);

          // increment pointers to the next values
          src0 += 16;
          src1 += 16;
          src2 += 16;
          dst += 48;
      }

      for (; dst<dstEnd; ++src0, ++src1, ++src2, dst += 3) {
        // convert 1 value
        copy_p3c3(src0, src1, src2, dst);
      }
    }

      template<class S, class D>
      inline void copy_p4c4(const S *src0, const S *src1, const S *src2, const S *src3, D *dst) {
        *dst     = clipped_cast<S,D>(*src0);
        *(dst+1) = clipped_cast<S,D>(*src1);
        *(dst+2) = clipped_cast<S,D>(*src2);
        *(dst+3) = clipped_cast<S,D>(*src3);
      }

      inline void sse_copy_p4c4(const icl8u *src0, const icl8u *src1, const icl8u *src2, const icl8u *src3, icl8u *dst) {
        icl128i v0 = icl128i(src0);
        icl128i v1 = icl128i(src1);
        icl128i v2 = icl128i(src2);
        icl128i v3 = icl128i(src3);

        __m128i vl0 = _mm_unpacklo_epi8(v0.v0, v1.v0);
        __m128i vh0 = _mm_unpackhi_epi8(v0.v0, v1.v0);
        __m128i vl1 = _mm_unpacklo_epi8(v2.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi8(v2.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi16(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi16(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi16(vh0, vh1);
        v3.v0 = _mm_unpackhi_epi16(vh0, vh1);

        v0.storeu(dst);
        v1.storeu(dst+16);
        v2.storeu(dst+32);
        v3.storeu(dst+48);
      }

    void for_copy_p4c4(const icl8u *s0, const icl8u *s1, const icl8u *s2, const icl8u *s3, icl8u *dst, icl8u *dstEnd) {
      sse_for(s0, s1, s2, s3, dst, dstEnd, copy_p4c4, sse_copy_p4c4, 16, 64);
    }

  #endif

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
          for_copy_p2c2(src[0], src[1], dst, dst + channels*len);
          break;
        }case 3:{
          for_copy_p3c3(src[0], src[1], src[2], dst, dst + channels*len);
          break;
        }case 4:{
          for_copy_p4c4(src[0], src[1], src[2], src[3], dst, dst + channels*len);
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
        planarToInterleaved_Generic_WITH_ROI(src,dst,dstLineStep < 0 ? src->getLineStep()*src->getChannels()*sizeof(D)/sizeof(S) : dstLineStep);
      }
    }
    
    // }}}

    template<class S, class D>
    void for_copy_c2p2(const S *src, D *d0, D *d1, const D *dstEnd) {
      while (d0 < dstEnd) {
        *d0++ = clipped_cast<S,D>(*src++);
        *d1++ = clipped_cast<S,D>(*src++);
      }
    }

    template<class S, class D>
    void for_copy_c3p3(const S *src, D *d0, D *d1, D *d2, const D *dstEnd) {
      while (d0 < dstEnd) {
        *d0++ = clipped_cast<S,D>(*src++);
        *d1++ = clipped_cast<S,D>(*src++);
        *d2++ = clipped_cast<S,D>(*src++);
      }
    }

    template<class S, class D>
    void for_copy_c4p4(const S *src, D *d0, D *d1, D *d2, D *d3, const D *dstEnd) {
      while (d0 < dstEnd) {
        *d0++ = clipped_cast<S,D>(*src++);
        *d1++ = clipped_cast<S,D>(*src++);
        *d2++ = clipped_cast<S,D>(*src++);
        *d3++ = clipped_cast<S,D>(*src++);
      }
    }

  #ifdef ICL_HAVE_SSSE3

      template<class S, class D>
      inline void copy_c3p3(const S *src, D *dst0, D *dst1, D *dst2) {
        *dst0 = clipped_cast<S,D>(*src);
        *dst1 = clipped_cast<S,D>(*(src+1));
        *dst2 = clipped_cast<S,D>(*(src+2));
      }

      inline void sse_copy_c3p3(const icl8u *src, icl8u *dst0, icl8u *dst1, icl8u *dst2) {
        // this function can be improved with SSE4 using _mm_blendv_epi8;

        const icl128i mask = icl128i(0x09060300, 0x0A070401, 0x0B080502, 0x80808080);

        icl128i v0 = icl128i(src);
        icl128i v1 = icl128i(src+12);
        icl128i v2 = icl128i(src+24);
        icl128i v3 = icl128i(src+36);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vl0 = _mm_unpacklo_epi32(v0.v0, v1.v0);
        __m128i vh0 = _mm_unpackhi_epi32(v0.v0, v1.v0);
        __m128i vl1 = _mm_unpacklo_epi32(v2.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi32(v2.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi64(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi64(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi64(vh0, vh1);

        v0.storeu(dst0);
        v1.storeu(dst1);
        v2.storeu(dst2);
      }

      inline void sse_copy_c3p3(const icl8u *src, icl16s *dst0, icl16s *dst1, icl16s *dst2) {
        // this function can be improved with SSE4 using _mm_blendv_epi8;

        const icl128i mask = icl128i(0x09060300, 0x0A070401, 0x0B080502, 0x80808080);

        icl128i v0 = icl128i(src);
        icl128i v1 = icl128i(src+12);
        icl128i v2 = icl128i(src+24);
        icl128i v3 = icl128i(src+36);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vl0 = _mm_unpacklo_epi32(v0.v0, v1.v0);
        __m128i vh0 = _mm_unpackhi_epi32(v0.v0, v1.v0);
        __m128i vl1 = _mm_unpacklo_epi32(v2.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi32(v2.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi64(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi64(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi64(vh0, vh1);

        icl256i(v0).storeu(dst0);
        icl256i(v1).storeu(dst1);
        icl256i(v2).storeu(dst2);
      }

      inline void sse_copy_c3p3(const icl8u *src, icl32s *dst0, icl32s *dst1, icl32s *dst2) {
        // this function can be improved with SSE4 using _mm_blendv_epi8;

        const icl128i mask = icl128i(0x09060300, 0x0A070401, 0x0B080502, 0x80808080);

        icl128i v0 = icl128i(src);
        icl128i v1 = icl128i(src+12);
        icl128i v2 = icl128i(src+24);
        icl128i v3 = icl128i(src+36);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vl0 = _mm_unpacklo_epi32(v0.v0, v1.v0);
        __m128i vh0 = _mm_unpackhi_epi32(v0.v0, v1.v0);
        __m128i vl1 = _mm_unpacklo_epi32(v2.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi32(v2.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi64(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi64(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi64(vh0, vh1);

        icl512i(icl256i(v0)).storeu(dst0);
        icl512i(icl256i(v1)).storeu(dst1);
        icl512i(icl256i(v2)).storeu(dst2);
      }

      inline void sse_copy_c3p3(const icl8u *src, icl32f *dst0, icl32f *dst1, icl32f *dst2) {
        // this function can be improved with SSE4 using _mm_blendv_epi8;

        const icl128i mask = icl128i(0x09060300, 0x0A070401, 0x0B080502, 0x80808080);

        icl128i v0 = icl128i(src);
        icl128i v1 = icl128i(src+12);
        icl128i v2 = icl128i(src+24);
        icl128i v3 = icl128i(src+36);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vl0 = _mm_unpacklo_epi32(v0.v0, v1.v0);
        __m128i vh0 = _mm_unpackhi_epi32(v0.v0, v1.v0);
        __m128i vl1 = _mm_unpacklo_epi32(v2.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi32(v2.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi64(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi64(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi64(vh0, vh1);

        icl512(icl512i(icl256i(v0))).storeu(dst0);
        icl512(icl512i(icl256i(v1))).storeu(dst1);
        icl512(icl512i(icl256i(v2))).storeu(dst2);
      }

      inline void sse_copy_c3p3(const icl32f *src, icl8u *dst0, icl8u *dst1, icl8u *dst2) {
        icl128 v0 = icl128(src);
        icl128 v1 = icl128(src+3);
        icl128 v2 = icl128(src+6);
        icl128 v3 = icl128(src+9);

        __m128 vl0 = _mm_unpacklo_ps(v0.v0, v2.v0);
        __m128 vh0 = _mm_unpackhi_ps(v0.v0, v2.v0);
        __m128 vl1 = _mm_unpacklo_ps(v1.v0, v3.v0);
        __m128 vh1 = _mm_unpackhi_ps(v1.v0, v3.v0);

        icl512 vR0, vR1, vR2;
        vR0.v0 = _mm_unpacklo_ps(vl0, vl1);
        vR1.v0 = _mm_unpackhi_ps(vl0, vl1);
        vR2.v0 = _mm_unpacklo_ps(vh0, vh1);

        v0 = icl128(src+12);
        v1 = icl128(src+15);
        v2 = icl128(src+18);
        v3 = icl128(src+21);

        vl0 = _mm_unpacklo_ps(v0.v0, v2.v0);
        vh0 = _mm_unpackhi_ps(v0.v0, v2.v0);
        vl1 = _mm_unpacklo_ps(v1.v0, v3.v0);
        vh1 = _mm_unpackhi_ps(v1.v0, v3.v0);

        vR0.v1 = _mm_unpacklo_ps(vl0, vl1);
        vR1.v1 = _mm_unpackhi_ps(vl0, vl1);
        vR2.v1 = _mm_unpacklo_ps(vh0, vh1);

        v0 = icl128(src+24);
        v1 = icl128(src+27);
        v2 = icl128(src+30);
        v3 = icl128(src+33);

        vl0 = _mm_unpacklo_ps(v0.v0, v2.v0);
        vh0 = _mm_unpackhi_ps(v0.v0, v2.v0);
        vl1 = _mm_unpacklo_ps(v1.v0, v3.v0);
        vh1 = _mm_unpackhi_ps(v1.v0, v3.v0);

        vR0.v2 = _mm_unpacklo_ps(vl0, vl1);
        vR1.v2 = _mm_unpackhi_ps(vl0, vl1);
        vR2.v2 = _mm_unpacklo_ps(vh0, vh1);

        v0 = icl128(src+36);
        v1 = icl128(src+39);
        v2 = icl128(src+42);
        v3 = icl128(src+45);

        vl0 = _mm_unpacklo_ps(v0.v0, v2.v0);
        vh0 = _mm_unpackhi_ps(v0.v0, v2.v0);
        vl1 = _mm_unpacklo_ps(v1.v0, v3.v0);
        vh1 = _mm_unpackhi_ps(v1.v0, v3.v0);

        vR0.v3 = _mm_unpacklo_ps(vl0, vl1);
        vR1.v3 = _mm_unpackhi_ps(vl0, vl1);
        vR2.v3 = _mm_unpacklo_ps(vh0, vh1);

        vR0.storeu(dst0);
        vR1.storeu(dst1);
        vR2.storeu(dst2);
      }

    void for_copy_c3p3(const icl8u *src, icl8u *d0, icl8u *d1, icl8u *d2, icl8u *dstEnd) {
      sse_for(src, d0, d1, d2, dstEnd, copy_c3p3, sse_copy_c3p3, 48, 16);
    }

    void for_copy_c3p3(const icl8u *src, icl16s *d0, icl16s *d1, icl16s *d2, icl16s *dstEnd) {
      sse_for(src, d0, d1, d2, dstEnd, copy_c3p3, sse_copy_c3p3, 48, 16);
    }

    void for_copy_c3p3(const icl8u *src, icl32s *d0, icl32s *d1, icl32s *d2, icl32s *dstEnd) {
      sse_for(src, d0, d1, d2, dstEnd, copy_c3p3, sse_copy_c3p3, 48, 16);
    }

    void for_copy_c3p3(const icl8u *src, icl32f *d0, icl32f *d1, icl32f *d2, icl32f *dstEnd) {
      sse_for(src, d0, d1, d2, dstEnd, copy_c3p3, sse_copy_c3p3, 48, 16);
    }

    void for_copy_c3p3(const icl32f *src, icl8u *dst0, icl8u *dst1, icl8u *dst2, icl8u *dstEnd) {
      icl8u *dstSSEEnd = dstEnd - 20;

      for (; dst0<dstSSEEnd;) {
          // convert 'rvalues' values at the same time
          sse_copy_c3p3(src, dst0, dst1, dst2);

          // increment pointers to the next values
          src  += 48;
          dst0 += 16;
          dst1 += 16;
          dst2 += 16;
      }

      for (; dst0<dstEnd; src += 3, ++dst0, ++dst1, ++dst2) {
        // convert 1 value
        copy_c3p3(src, dst0, dst1, dst2);
      }
    }

      inline void copy_c4p4(const icl8u *src, icl8u *dst0, icl8u *dst1, icl8u *dst2, icl8u *dst3) {
        *dst0 = *src;
        *dst1 = *(src+1);
        *dst2 = *(src+2);
        *dst3 = *(src+3);
      }

      inline void sse_copy_c4p4(const icl8u *src, icl8u *dst0, icl8u *dst1, icl8u *dst2, icl8u *dst3) {
        // this function can be improved with SSE4 using _mm_blendv_epi8;

        const icl128i mask = icl128i(0x0C080400, 0x0D090501, 0x0E0A0602, 0x0F0B0703);

        icl128i v0 = icl128i(src);
        icl128i v1 = icl128i(src+16);
        icl128i v2 = icl128i(src+32);
        icl128i v3 = icl128i(src+48);

        v0.v0 = _mm_shuffle_epi8(v0.v0, mask.v0);
        v1.v0 = _mm_shuffle_epi8(v1.v0, mask.v0);
        v2.v0 = _mm_shuffle_epi8(v2.v0, mask.v0);
        v3.v0 = _mm_shuffle_epi8(v3.v0, mask.v0);

        __m128i vl0 = _mm_unpacklo_epi32(v0.v0, v1.v0);
        __m128i vh0 = _mm_unpackhi_epi32(v0.v0, v1.v0);
        __m128i vl1 = _mm_unpacklo_epi32(v2.v0, v3.v0);
        __m128i vh1 = _mm_unpackhi_epi32(v2.v0, v3.v0);

        v0.v0 = _mm_unpacklo_epi64(vl0, vl1);
        v1.v0 = _mm_unpackhi_epi64(vl0, vl1);
        v2.v0 = _mm_unpacklo_epi64(vh0, vh1);
        v3.v0 = _mm_unpackhi_epi64(vh0, vh1);

        v0.storeu(dst0);
        v1.storeu(dst1);
        v2.storeu(dst2);
        v3.storeu(dst3);
      }

    void for_copy_c4p4(const icl8u *src, icl8u *d0, icl8u *d1, icl8u *d2, icl8u *d3, icl8u *dstEnd) {
      sse_for(src, d0, d1, d2, d3, dstEnd, copy_c4p4, sse_copy_c4p4, 64, 16);
    }

  #endif

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
          for_copy_c2p2(src, dst[0], dst[1], dst[0] + len);
          break;
        }case 3:{
          for_copy_c3p3(src, dst[0], dst[1], dst[2], dst[0] + len);
          break;
        }
        case 4:{
          for_copy_c4p4(src, dst[0], dst[1], dst[2], dst[3], dst[0] + len);
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
    
  #ifdef ICL_HAVE_IPP
  
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
    template ICLCore_API void planarToInterleaved<TYPEB, TYPEA>(const Img<TYPEB>*, TYPEA*, int);             \
    template ICLCore_API void interleavedToPlanar<TYPEA, TYPEB>(const TYPEA*, Img<TYPEB>*, int)
  
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(signed char,icl8u);
    
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl8u);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl16s,icl8u);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32s,icl8u);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32f,icl8u);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl64f,icl8u);
  
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(signed char,icl16s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl16s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl16s,icl16s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32s,icl16s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32f,icl16s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl64f,icl16s);
  
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(signed char,icl32s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl32s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl16s,icl32s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32s,icl32s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32f,icl32s);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl64f,icl32s);
  
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(signed char,icl32f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl32f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl16s,icl32f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32s,icl32f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32f,icl32f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl64f,icl32f);
  
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(signed char,icl64f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl8u,icl64f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl16s,icl64f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32s,icl64f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl32f,icl64f);
    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(icl64f,icl64f);
  
  #undef EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION
  
    // }}}

  #ifdef ICL_HAVE_SSE2
      inline void subSSEYUV420toRGB(const icl8u *y, const icl8u *u, const icl8u *v,
                                    icl8u *r, icl8u *g, icl8u *b) {
        // load YUV values
        icl512 vY = icl512(y);
        icl128i viU = icl128i(u);
        viU.v0 = _mm_unpacklo_epi8(viU.v0, viU.v0);
        icl128i viV = icl128i(v);
        viV.v0 = _mm_unpacklo_epi8(viV.v0, viV.v0);
        icl512 vU = icl512(icl512i(icl256i(viU)));
        icl512 vV = icl512(icl512i(icl256i(viV)));

        vU -= icl512(128.0f);
        vV -= icl512(128.0f);

        icl512 vR = vY + icl512(1.140f) * vV;
        icl512 vG = vY - icl512(0.394f) * vU - icl512(0.581f) * vV;
        icl512 vB = vY + icl512(2.032f) * vU;

        // store the calculated values
        vR.storeu(r);
        vG.storeu(g);
        vB.storeu(b);
      }
  #endif

    void convertYUV420ToRGB8(const unsigned char *pucSrc,const Size &s, Img8u *poDst){
      // {{{ open
  #ifdef ICL_HAVE_SSE2
    const int w = s.width;
    const int w2 = w/2;
    const int h = s.height;

    const icl8u *cY  = pucSrc;
    const icl8u *cU  = pucSrc + w*h;
    const icl8u *cV  = pucSrc + w*h + (w*h)/4;

    icl8u *cR = poDst->getData(0);
    icl8u *cG = poDst->getData(1);
    icl8u *cB = poDst->getData(2);

    if (poDst->getDim() < w*h) {
      ERROR_LOG("the given size does not match size of the destination image"); 
    }
    const icl8u *end = cU;
    const icl8u *lEnd = cY + w;
    const icl8u *sEnd = lEnd - 15;

    int flag = 1;

    for (; cY < end-2*w;) {
      if (cY < sEnd) {
        subSSEYUV420toRGB(cY, cU, cV, cR, cG, cB);
        cY += 16;
        cU += 8;
        cV += 8;
        cR += 16;
        cG += 16;
        cB += 16;
      } else {
        for (;cY < lEnd; ++cY, ++cU, ++cV, ++cR, ++cG, ++cB) {
          subYUVtoRGB(cY++, cU, cV, cR++, cG++, cB++);
          subYUVtoRGB(cY, cU, cV, cR, cG, cB);
        }
        lEnd += w;
        sEnd += w;
        if (flag++) {
          cU -= w2;
          cV -= w2;
          flag = 0;
        }
      }
    }
    sEnd -= 16;
    for (; cY < end;) {
      if (cY < sEnd) {
        subSSEYUV420toRGB(cY, cU, cV, cR, cG, cB);
        cY += 16;
        cU += 8;
        cV += 8;
        cR += 16;
        cG += 16;
        cB += 16;
      } else {
        for (;cY < lEnd; ++cY, ++cU, ++cV, ++cR, ++cG, ++cB) {
          subYUVtoRGB(cY++, cU, cV, cR++, cG++, cB++);
          subYUVtoRGB(cY, cU, cV, cR, cG, cB);
        }
        lEnd += w;
        sEnd += w;
        if (flag++) {
          cU -= w2;
          cV -= w2;
          flag = 0;
        }
      }
    }
// IPP conversion is replaced by a SSE implementation
/*
  #ifdef ICL_HAVE_IPP
      const icl8u *apucSrc[] = {pucSrc,pucSrc+s.getDim(), pucSrc+s.getDim()+s.getDim()/4};
      icl8u *apucDst[] = {poDst->getData(0),poDst->getData(1),poDst->getData(2)};
      ippiYUV420ToRGB_8u_P3(apucSrc,apucDst,s); 
  #endif
*/
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
  
  } // namespace core
}
