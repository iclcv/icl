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
        core::DataSegment<icl##D,N> dst((icl##D*)d, N*sizeof(icl##D), dim, mi.width); \
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
          if(!o.supports(PointCloudObjectBase::X)){                     \
            if(o.canAddFeature(PointCloudObjectBase::X)){               \
              o.addFeature(PointCloudObjectBase::X);                    \
            }                                                           \
          }                                                             \
          if(o.supports(PointCloudObjectBase::X)){                      \
            const icl8u *s = dev.sourceFor(f,nBytes);                   \
            core::DataSegment<icl##D,N> src((icl##D*)s,                 \
                                            N*sizeof(icl##D),           \
                                            dim, mi.width);             \
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



    
    void PointCloudSerializer::DefaultSerializationDevice::initializeSerialization(const MandatoryInfo &info){
      this->info = info;
    }

    icl8u *PointCloudSerializer::DefaultSerializationDevice::targetFor(const std::string &featureName, int bytes){
      data[featureName].resize(bytes);
      return data[featureName].data();
    }
    
    /// returns number of bytes when all data is concatenated
    int PointCloudSerializer::DefaultSerializationDevice::getFullSerializationSize() const{
      /* data: 
         byte[sizeof(MandatoryInfo)] mandatory info
         int num entries
         block [numEnries]:
          nameLen, entryLen, nameData, entryData

         note: each strig is stored without explicit \0 delimiter
     */
      int n = sizeof(MandatoryInfo) + 4;
      for(std::map<std::string, std::vector<icl8u> >::const_iterator it = data.begin();
          it != data.end(); ++it){
        n += 4 +  4 + it->first.length() + it->second.size();
      }

      return n;
    }
    
    static void copy_next_bytes(icl8u *&dst,const void *src, int numBytes){
      memcpy(dst, src, numBytes);
      dst += numBytes;
    }

    static void copy_next_bytes_src(void *dst, const icl8u *&src, int numBytes){
      memcpy(dst, src, numBytes);
      src += numBytes;
    }

    
    /// data needs to be at least getFullSerializationSize bytes long
    void PointCloudSerializer::DefaultSerializationDevice::copyData(icl8u *dst){
      copy_next_bytes(dst, &info, sizeof(MandatoryInfo));

      icl32s n = data.size();
      copy_next_bytes(dst, &n, sizeof(n));
      
      for(std::map<std::string, std::vector<icl8u> >::const_iterator it = data.begin();
          it != data.end(); ++it){
        icl32s nameLen = it->first.length();
        icl32s dataLen = it->second.size();

        copy_next_bytes(dst, &nameLen, sizeof(nameLen));
        copy_next_bytes(dst, &dataLen, sizeof(dataLen));
        copy_next_bytes(dst, &it->first[0], nameLen);
        copy_next_bytes(dst, &it->second[0], dataLen);
      }
    }

    void PointCloudSerializer::DefaultSerializationDevice::clear(){
      data.clear();
    }

    PointCloudSerializer::DefaultDeserializationDevice::DefaultDeserializationDevice(const icl8u *src){
      //std::map<std::string, std::vector<icl8u> > data;
      //  std::vector<std::string> features;

      features.clear();
      
      copy_next_bytes_src(&info, src, sizeof(MandatoryInfo));
      icl32s nFeatures = 0;
      copy_next_bytes_src(&nFeatures, src, sizeof(nFeatures));
      
      for(icl32s i=0;i<nFeatures;++i){
        icl32s nameLen = 0;
        icl32s dataLen = 0;
        copy_next_bytes_src(&nameLen, src, sizeof(nameLen));
        copy_next_bytes_src(&dataLen, src, sizeof(dataLen));

        std::string name(nameLen,'\0');
        copy_next_bytes_src(&name[0],src, nameLen);
        
        features.push_back(name);
        
        std::vector<icl8u> &dataVec = data[name];
        dataVec.resize(dataLen);
        copy_next_bytes_src(&dataVec[0],src, dataLen);
      }
    }

    const icl8u *PointCloudSerializer::DefaultDeserializationDevice::sourceFor(const std::string &featureName, int &bytes){
      std::map<std::string,std::vector<icl8u> >::const_iterator it = data.find(featureName);
      if(it == data.end()){
        bytes = 0;
        throw ICLException("DefaultDeserializationDevice::sourceFor: no source for feature name '" + featureName + "' found");
      }
      return it->second.data();
    }

  }
}

