/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WarpOp.cpp                     **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
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

#include <ICLFilter/WarpOp.h>
#ifdef ICL_HAVE_OPENCL
  #include <ICLUtils/CLProgram.h>
  #include <CL/cl.hpp>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

  #ifdef ICL_HAVE_OPENCL
    struct WarpOp::CLWarp {
      CLProgram program;
      CLImage2D input;
      CLImage2D output;
      CLImage2D warpX, warpY;
      CLKernel kernel;
      Size mapSize;

      CLWarp() {
        static const char *k = (
          "__kernel void warp(const unsigned int mode,                                \n"
          "                   __read_only image2d_t warpX,                            \n"
          "                   __read_only image2d_t warpY,                            \n"
          "                   __read_only image2d_t in,                               \n"
          "                   __write_only image2d_t out) {                           \n"
          "    const int x = get_global_id(0);                                        \n"
          "    const int y = get_global_id(1);                                        \n"
          "    const int w = get_global_size(0);                                      \n"
          "    const int h = get_global_size(1);                                      \n"
          "    if(x && y && x<w-1 && y<h-1) {                                         \n"
          "      const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |              \n"
          "                                CLK_ADDRESS_CLAMP |                        \n"
          "                                CLK_FILTER_LINEAR;                         \n"
          "      const sampler_t samplerN = CLK_NORMALIZED_COORDS_FALSE |             \n"
          "                                 CLK_ADDRESS_CLAMP |                       \n"
          "                                 CLK_FILTER_NEAREST;                       \n"
          "      float4 fX = read_imagef(warpX, sampler, (int2)(x,y));                \n"
          "      float4 fY = read_imagef(warpY, sampler, (int2)(x,y));                \n"
          "      uint4 inPixel = read_imageui(in, samplerN, (float2)(fX.s0, fY.s0));  \n"
          "      write_imageui(out, (int2)(x,y), inPixel.s0);                         \n"
          "  }                                                                        \n"
          "}                                                                          \n");

        program = CLProgram("gpu", k);
        kernel = program.createKernel("warp");
      }

      void setWarpMap(const Img32f &warpMap) {
        mapSize = warpMap.getSize();
        int w = warpMap.getWidth();
        int h = warpMap.getHeight();
        warpX = program.createImage2D("r", w, h, 3, warpMap.begin(0));
        warpY = program.createImage2D("r", w, h, 3, warpMap.begin(1));
      }

      void setWarpMap(const Channel32f warpMap[2]) {
        mapSize = warpMap[0].getSize();
        int w = warpMap[0].getWidth();
        int h = warpMap[0].getHeight();
        warpX = program.createImage2D("r", w, h, 3, warpMap[0].begin());
        warpY = program.createImage2D("r", w, h, 3, warpMap[1].begin());
        std::cout << "changed" << std::endl;
      }

      void apply(const Channel32f warpMap[2], const ImgBase *src, ImgBase *dst, scalemode mode) {
        cl_filter_mode filterMode;
        int w = src->getWidth();
        int h = src->getHeight();
        
        if (mode == interpolateNN)
          filterMode = CL_FILTER_NEAREST;
        else if (mode == interpolateLIN)
          filterMode = CL_FILTER_LINEAR;
        else {
          ERROR_LOG("region average interpolation mode does not work here!");
          return;
        }

        if (warpMap[0].getSize() != mapSize)
          setWarpMap(warpMap);

        input = program.createImage2D("r", w, h, src->getDepth());
        output = program.createImage2D("w", w, h, src->getDepth());

        for (int i = 0; i < src->getChannels(); i++) {
          input.write(src->getDataPtr(i));
          kernel.setArgs(filterMode, warpX, warpY, input, output);
          kernel.apply(w, h, 0);
          output.read(dst->getDataPtr(i));
        }
      }
    };
  #endif

    template<class T>
    T interpolate_pixel_nn(float x, float y, const Channel<T> &src){
      if(x < 0) return T(0);
      return src(round(x),round(y));
    }
    
    template<class T>
    T interpolate_pixel_lin(float x, float y, const Channel<T> &src){
      float fX0 = x - floor(x), fX1 = 1.0 - fX0;
      float fY0 = y - floor(y), fY1 = 1.0 - fY0;
      int xll = (int) x;
      int yll = (int) y;
     
      const T* pLL = &src(xll,yll);
      float a = *pLL;        //  a b
      float b = *(++pLL);    //  c d
      pLL += src.getWidth();
      float d = *pLL;
      float c = *(--pLL);
  
      return fX1 * (fY1*a + fY0*c) + fX0 * (fY1*b + fY0*d);
    }
    
  
    template<class T>
    static inline void apply_warp_2(const Channel32f warpMap[2],
                                    const Channel<T> &src,
                                    Channel<T> &dst,
                                    const Size &size, T (*interpolator)(float x, float y, const Channel<T> &src)){
  
      for(int x=0;x<size.width;++x){
        for(int y=0;y<size.height;++y){
          int idx = x+size.width*y;
          dst[idx] = interpolator(warpMap[0][idx],warpMap[1][idx],src);
        }
      }
    }

    template<class T>
    static void apply_warp(const Channel32f warpMap[2], 
                           const Img<T>&src, 
                           Img<T> &dst,
                           scalemode mode){
      for(int c=0;c<src.getChannels();++c){
        const Channel<T> s = src[c];
        Channel<T> d = dst[c];
        if(mode == interpolateNN){
          apply_warp_2<T>(warpMap,s,d,s.getSize(),interpolate_pixel_nn<T>);
        }else if(mode == interpolateLIN){
          apply_warp_2<T>(warpMap,s,d,s.getSize(),interpolate_pixel_lin<T>);
        }else{
          ERROR_LOG("region average interpolation mode does not work here!");
          return;
        }
      }
    }          
  
  #ifdef ICL_HAVE_IPP
    template<>  
    void apply_warp<icl8u>(const Channel32f warpMap[2], 
                                  const Img<icl8u> &src, 
                                  Img<icl8u> &dst,
                                  scalemode mode){
      for(int c=0;c<src.getChannels();++c){
        IppStatus s = ippiRemap_8u_C1R(src.begin(c),src.getSize(),src.getLineStep(),
                                       src.getImageRect(),warpMap[0].begin(),sizeof(icl32f)*warpMap[0].getWidth(),
                                       warpMap[1].begin(),sizeof(icl32f)*warpMap[1].getWidth(),dst.begin(c),
                                       dst.getLineStep(),dst.getSize(),(int)mode);
        if(s != ippStsNoErr){
          ERROR_LOG("IPP-Error:" << ippGetStatusString(s));
          return;
        }
      }
    }
    template<>  
    void apply_warp<icl32f>(const Channel32f warpMap[2], 
                                   const Img<icl32f> &src, 
                                   Img<icl32f> &dst,
                                   scalemode mode){
      for(int c=0;c<src.getChannels();++c){
        IppStatus s = ippiRemap_32f_C1R(src.begin(c),src.getSize(),src.getLineStep(),
                                       src.getImageRect(),warpMap[0].begin(),sizeof(icl32f)*warpMap[0].getWidth(),
                                       warpMap[1].begin(),sizeof(icl32f)*warpMap[1].getWidth(),dst.begin(c),
                                       dst.getLineStep(),dst.getSize(),(int)mode);
        if(s != ippStsNoErr){
          ERROR_LOG("IPP-Error:" << ippGetStatusString(s));
          return;
        }
      }
    }
  
    
    // specialization for apply_warp<icl8u> and <icl32f>
  #endif
  
    void prepare_warp_table_inplace(Img32f &warpMap){
      const Rect r = warpMap.getImageRect();
      
      Channel32f cs[2];
      warpMap.extractChannels(cs);
      const Size size = warpMap.getSize();
  
      for(int x=0;x<size.width;++x){
        for(int y=0;y<size.height;++y){
          if(!r.contains(round(cs[0](x,y)),round(cs[1](x,y)))){
            cs[0](x,y) = cs[1](x,y) = -1;
          }
        }
      } 
    }
    
  
    WarpOp::WarpOp(const Img32f &warpMap,scalemode mode, bool allowWarpMapScaling):
      m_allowWarpMapScaling(allowWarpMapScaling),m_scaleMode(mode){
      warpMap.deepCopy(&m_warpMap);
      prepare_warp_table_inplace(m_warpMap);
  #ifdef ICL_HAVE_OPENCL
      m_clWarp = new CLWarp();
  #endif
    }

    WarpOp::~WarpOp() {
  #ifdef ICL_HAVE_OPENCL
      delete m_clWarp;
  #endif
    }
    
    void WarpOp::setScaleMode(scalemode scaleMode){
      m_scaleMode = scaleMode;
    }
    void WarpOp::setWarpMap(const Img32f &warpMap){
      warpMap.deepCopy(&m_warpMap);
      prepare_warp_table_inplace(m_warpMap);
      m_scaledWarpMap = Img32f();
  #ifdef ICL_HAVE_OPENCL
      m_clWarp->setWarpMap(m_warpMap);
  #endif
    }
    void WarpOp::setAllowWarpMapScaling(bool allow){
      m_allowWarpMapScaling = allow;
    }
    
    
    void WarpOp::apply(const ImgBase *src, ImgBase **dst){
      ICLASSERT_RETURN(src);
      ICLASSERT_RETURN(dst);
      ICLASSERT_RETURN(src != *dst);
      ICLASSERT_RETURN(m_warpMap.getSize() != Size::null);
  
      if(!src->hasFullROI()){
        ERROR_LOG("warp op does currently not support ROI");
        return;
      }
  
      if(!UnaryOp::prepare(dst,src->getDepth(),src->getSize(),
                           src->getFormat(),src->getChannels(),
                           src->getROI(),src->getTime())){
        ERROR_LOG("unable to prepare destination image (returning)");
        return;
      }
      
      Channel32f cwm[2];

      if(src->getSize() != m_warpMap.getSize()){
        if(m_allowWarpMapScaling){
          if(m_scaledWarpMap.getSize() != src->getSize()){
            m_scaledWarpMap.setSize(src->getSize());
            m_warpMap.scaledCopy(&m_scaledWarpMap);
            prepare_warp_table_inplace(m_scaledWarpMap);
          }
          m_scaledWarpMap.extractChannels(cwm);
        }else{
          ERROR_LOG("warp map size and image size are not equal\n"
                    "warp map can be scaled using\n"
                    "setAllowWarpMapScaling(true)");
          return;
        }
      }else{
        m_warpMap.extractChannels(cwm);
      }

  #ifdef ICL_HAVE_OPENCL
      // the written kernel of CLWarp supports only uint values and linear interpolation;
      // TODO: create CLSampler for diffrent interpolation methods
      if (src->getDepth() == 0 && m_scaleMode == interpolateLIN) {
        m_clWarp->apply(cwm, src, *dst, m_scaleMode);
        return;
      }
  #endif

      switch(src->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                 \
        case depth##D:                                             \
        apply_warp<icl##D>(cwm,*src->asImg<icl##D>(),              \
                           *(*dst)->asImg<icl##D>(),m_scaleMode);  \
        break;
        ICL_INSTANTIATE_ALL_DEPTHS;
        default:
          ICL_INVALID_DEPTH;
  #undef ICL_INSTANTIATE_DEPTH
  
      }
    }
  
  
  
  } // namespace filter
}
