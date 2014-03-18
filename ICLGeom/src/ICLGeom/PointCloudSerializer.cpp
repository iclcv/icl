/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudSerializer.cpp           **
** Module : ICLGeom                                                **
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

#include <ICLGeom/PointCloudSerializer.h>

namespace icl{
  using namespace utils;
  namespace geom{
    
    void PointCloudSerializer::serialize(const PointCloudObjectBase &o, SerializationDevice &dev){
      MandatoryInfo mi = { 0, 0, 
                           o.isOrganized(), 
                           o.getTime().toMicroSeconds() };
      if(mi.organized){
        Size s = o.getSize();
        mi.width = s.width;
        mi.height = s.height;
      }else{
        mi.width = o.getDim();
        mi.height = 1;
      }
      dev.initializeSerialization(mi);
      
      const int dim = o.getDim();
      
#define CPY(X,D,N)                                                      \
      {                                                                 \
        icl8u *d  = dev.targetFor(#X,dim*sizeof(icl##D)*N);             \
        DataSegment<icl##D,N> dst((icl##D*)d, N*sizeof(icl##D), dim, o.getSize().width); \
        o.select##X().deepCopy(dst);                                    \
      }

#define CPY_IF(X,T,N) if(o.supports(PointCloudObjectBase::X)) CPY(X,T,N)

      if(o.supports(PointCloudObjectBase::XYZH)){
        CPY(XYZH,32f,4);
      }else if(o.supports(PointCloudObjectBase::XYZ)){
        CPY(XYZ,32f,3);
      }
      
      CPY_IF(Intensity,32f,1);
      CPY_IF(Label,32s,1);
      CPY_IF(Normal,32f,4);
      CPY_IF(RGBA32f,32f,4);
      if(o.supports(PointCloudObjectBase::BGRA32s)){
        CPY(BGRA32s,32s,1);
      }else if(o.supports(PointCloudObjectBase::BGRA)){
        CPY(BGRA,8u,4);
      }else if(o.supports(PointCloudObjectBase::BGR)){
        CPY(BGR,8u,3);
      }
#undef CPY
#undef CPY_IF

      const std::map<std::string,std::string> &m = o.getMetaData();
      for(std::map<std::string,std::string>::const_iterator it = m.begin(); it != m.end(); ++it){
        const std::string &key = it->first;
        const std::string &value = it->second;
        icl8u *d = dev.targetFor("meta:"+key, value.length());
        std::copy(value.begin(),value.end(), d);        
      }      
    }
    
    void PointCloudSerializer::deserialize(PointCloudObjectBase &o, DeserializationDevice &dev){
      MandatoryInfo mi = dev.getDeserializationInfo();
      
      if(mi.organized){
        o.setSize(Size(mi.width,mi.height));
      }else{
        o.setDim(mi.width);
      }
      o.setTime(utils::Time(mi.timestamp));
      
      std::vector<std::string> fs = dev.getFeatures();
      const int dim = o.getDim();
      o.clearAllMetaData();
      int nBytes = 0;
      for(size_t i=0;i<fs.size();++i){
        const std::string &f = fs[i];
        
        if(f.length()>=5 && f.substr(0,5) == "meta:"){
          const icl8u *s = dev.sourceFor(f.substr(5), nBytes);
          std::string m(nBytes,'\0');
          std::copy(s,s+nBytes,m.begin());
          o.setMetaData(f.substr(5),m);
          continue;
        }
#define CPY(X,D,N)                                                      \
        if(f == #X){                                                    \
          if(o.supports(PointCloudObjectBase::X)){                      \
            const icl8u *s = dev.sourceFor(f,nBytes);                   \
            DataSegment<icl##D,N> src((icl##D*)s,                       \
                                      N*sizeof(icl##D),                 \
                                      dim, o.getSize().width);          \
            src.deepCopy(o.select##X());                                \
          }                                                             \
          continue;                                                     \
        }

        CPY(XYZH,32f,4);
        CPY(XYZ,32f,3);
        CPY(Intensity,32f,1);
        CPY(Label,32s,1);
        CPY(Normal,32f,4);
        CPY(RGBA32f,32f,4);
        CPY(BGRA32s,32s,1);
        CPY(BGRA,8u,4);
        CPY(BGR,8u,3);
#undef CPY
      }      
      
    }    
  }
}

