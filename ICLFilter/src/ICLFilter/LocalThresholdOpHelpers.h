/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/LocalThresholdOpHelpers.h      **
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

#pragma once

#include <stdint.h>

namespace icl{
  namespace filter{
    namespace {
      template<class TT>
      struct ThreshType{ typedef TT T; };
      
      template<> struct ThreshType<uint8_t> { typedef int T; };
      template<> struct ThreshType<int16_t> { typedef int T; };
      template<> struct ThreshType<int32_t> { typedef int T; };
      template<> struct ThreshType<float> { typedef float T; };
      template<> struct ThreshType<double> { typedef double T; };

      inline float lt_clip_float(float f) { return f > 255 ? 255 : f < 0 ? 0 : f; }

      typedef uint8_t lt_icl8u;
      typedef int16_t lt_icl16s;
      typedef int32_t lt_icl32s;
      typedef float   lt_icl32f;
      typedef double  lt_icl64f;
    }
    /// Internally used helper function 
    /** This function was outsourced to optimize the compilation times by better
        exploiting multi-threaded compilation. The actual template instantiation of
        this function is spread over 10 source-files. */
    template<class TS,  class TI, class TD, class TT, bool WITH_GAMMA>
    void fast_lt(const TS *psrc, const TI *ii, TD *pdst, int w, int h, int r, TT t, float gs, int channel);

    /// Internally used helper function 
    /** This function was outsourced to optimize the compilation times by better
        exploiting multi-threaded compilation */
    template<class TS,  class TI, class TD, class TT, bool WITH_GAMMA>
    void fast_lt_impl(const TS *psrc, const TI *ii, TD *pdst, int w, int h, int r, TT t, float gs, int channel){
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
      ((TD)lt_clip_float( gs * (psrc[x+w*y] - float(GET_RECT((rx),(ry),(rw),(rh)))/((rw)*(rh)) ) + 128))
  
  
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
        
#define STEP *d = (!WITH_GAMMA) ?                                       \
        (255 * ( (*s*dim) > (*B - *C - *D + *A + t))) :                 \
        ((TD)lt_clip_float( gs * (*s - float(*B - *C - *D + *A + t)/dim ) + 128)) \
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
  }
}

#define FAST_LT_DEFINITION                                              \
    template<class TS,  class TI, class TD, class TT, bool WITH_GAMMA>  \
    void fast_lt(const TS *psrc, const TI *iim, TD *pdst, int w, int h, \
                 int r, TT t, float gs, int channel){                   \
      fast_lt_impl<TS,TI,TD,TT,WITH_GAMMA>(psrc,iim,pdst,w,h,r,t,gs,channel); \
    }

#define INST_FAST_LT(TS,TI,TD,WITH_GAMMA)                               \
  template void fast_lt<lt_icl##TS,lt_icl##TI,lt_icl##TD,               \
    ThreshType<lt_icl##TS>::T,WITH_GAMMA>                      \
  (const lt_icl##TS*, const lt_icl##TI*, lt_icl##TD*, int, int,         \
   int, ThreshType<lt_icl##TS>::T,float,int)

#define INST_FAST_LT_FOR_SRC_TYPE(SRC,WITH_GAMMA) \
  INST_FAST_LT(SRC,32s,8u,WITH_GAMMA);            \
  INST_FAST_LT(SRC,32f,8u,WITH_GAMMA);            \
  INST_FAST_LT(SRC,64f,8u,WITH_GAMMA);            \
  INST_FAST_LT(SRC,32s,16s,WITH_GAMMA);           \
  INST_FAST_LT(SRC,32f,16s,WITH_GAMMA);           \
  INST_FAST_LT(SRC,64f,16s,WITH_GAMMA);           \
  INST_FAST_LT(SRC,32s,32s,WITH_GAMMA);           \
  INST_FAST_LT(SRC,32f,32s,WITH_GAMMA);           \
  INST_FAST_LT(SRC,64f,32s,WITH_GAMMA);           \
  INST_FAST_LT(SRC,32s,32f,WITH_GAMMA);           \
  INST_FAST_LT(SRC,32f,32f,WITH_GAMMA);           \
  INST_FAST_LT(SRC,64f,32f,WITH_GAMMA);           \
  INST_FAST_LT(SRC,32s,64f,WITH_GAMMA);           \
  INST_FAST_LT(SRC,32f,64f,WITH_GAMMA);           \
  INST_FAST_LT(SRC,64f,64f,WITH_GAMMA);       


