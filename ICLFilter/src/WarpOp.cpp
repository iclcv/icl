/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/WarpOp.cpp                               **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLFilter/WarpOp.h>

namespace icl{

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
  

  template<class T, T (*interpolator)(float x, float y, const Channel<T> &src)>
  static inline void apply_warp_2(const Channel32f warpMap[2],
                                  const Channel<T> &src,
                                  Channel<T> &dst,
                                  const Size &size){

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
        apply_warp_2<T,interpolate_pixel_nn<T> >(warpMap,s,d,s.getSize());
      }else if(mode == interpolateLIN){
        apply_warp_2<T,interpolate_pixel_lin<T> >(warpMap,s,d,s.getSize());
      }else{
        ERROR_LOG("region average interpolation mode does not work here!");
        return;
      }
    }
  }

#ifdef HAVE_IPP
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
  }
  
  void WarpOp::setScaleMode(scalemode scaleMode){
    m_scaleMode = scaleMode;
  }
  void WarpOp::setWarpMap(const Img32f &warpMap){
    warpMap.deepCopy(&m_warpMap);
    prepare_warp_table_inplace(m_warpMap);
    m_scaledWarpMap = Img32f();
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



}
