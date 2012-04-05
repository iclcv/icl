/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ColorDistanceOp.h                        **
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

#include <ICLFilter/ColorDistanceOp.h>
#include <ICLCore/Img.h>

namespace icl{
  template<class R, class T>
  inline R rgb_dist_sqr(T r0, T r1, T r2, T a, T b, T c){
    return (R)(sqr((R)r0-(R)a) + sqr((R)r1-(R)b) + sqr((R)r2-(R)c));
  }
  
  template<class S, class D, class T, bool useThresh>
  void apply_color_distance_map(const Img<S> &src, const Img<D> &dst,const S ref[3], const T thresh){
    const Channel<S> r = src[0], g = src[1], b=src[2];
    Channel<D> d = dst[0];
    const int dim = src.getDim();
    if(useThresh){
      const int Tsquared = sqr(thresh);
      for(int i=0;i<dim;++i){
        d[i] = 255 * ( rgb_dist_sqr<T,S>(ref[0],ref[1],ref[2],r[i],g[i],b[i]) < Tsquared);
      }
    }else{
      for(int i=0;i<dim;++i){
        d[i] = ::sqrt(rgb_dist_sqr<T,D>(ref[0],ref[1],ref[2],r[i],g[i],b[i]));
      }
    }
  }

  void ColorDistanceOp::apply(const ImgBase *src, ImgBase **dst){
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(src->getChannels() == 3);
    ICLASSERT_RETURN(m_refColor.size() == 3);

    const icl8u ref8u [3] = { m_refColor[0], m_refColor[1], m_refColor[2] };
    const icl16s ref16s[3] = { m_refColor[0], m_refColor[1], m_refColor[2] };
    const icl32s ref32s[3] = { m_refColor[0], m_refColor[1], m_refColor[2] };
    const icl32f ref32f[3] = { m_refColor[0], m_refColor[1], m_refColor[2] };
    const icl64f ref64f[3] = { m_refColor[0], m_refColor[1], m_refColor[2] };

    if(m_threshold == -1){
      if (!UnaryOp::prepare (dst, src->getDepth() == depth64f ? depth64f : depth32f, src->getSize(), formatMatrix, 1, src->getImageRect(), src->getTime())) return;
      
      switch(src->getDepth()){
        case depth8u:
          apply_color_distance_map<icl8u,icl32f,icl32f,false>(*src->as8u(),*(*dst)->as32f(), ref8u, m_threshold);
          break;
        case depth16s:
          apply_color_distance_map<icl16s,icl32f,icl32f,false>(*src->as16s(),*(*dst)->as32f(), ref16s, m_threshold);
          break;
        case depth32s:
          apply_color_distance_map<icl32s,icl32f,icl32f,false>(*src->as32s(),*(*dst)->as32f(), ref32s, m_threshold);
          break;
        case depth32f:
          apply_color_distance_map<icl32f,icl32f,icl32f,false>(*src->as32f(),*(*dst)->as32f(), ref32f, m_threshold);
          break;
        case depth64f:
          apply_color_distance_map<icl64f,icl64f,icl64f,false>(*src->as64f(),*(*dst)->as64f(), ref64f, m_threshold);
          break;
      }
    }else{
      if (!UnaryOp::prepare (dst,depth8u, src->getSize(), formatMatrix, 1, src->getImageRect(), src->getTime())) return;
      switch(src->getDepth()){
        case depth8u:
          apply_color_distance_map<icl8u,icl8u,icl32s,true>(*src->as8u(),*(*dst)->as8u(), ref8u, m_threshold);
          break;
        case depth16s:
          apply_color_distance_map<icl16s,icl8u,icl32s,true>(*src->as16s(),*(*dst)->as8u(), ref16s, m_threshold);
          break;
        case depth32s:
          apply_color_distance_map<icl32s,icl8u,icl32s,true>(*src->as32s(),*(*dst)->as8u(), ref32s, m_threshold);
          break;
        case depth32f:
          apply_color_distance_map<icl32f,icl8u,icl32f,true>(*src->as32f(),*(*dst)->as8u(), ref32f, m_threshold);
          break;
        case depth64f:
          apply_color_distance_map<icl64f,icl8u,icl64f,true>(*src->as64f(),*(*dst)->as8u(), ref64f, m_threshold);
          break;
      }
    }
  }
}
