/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ProtoBufSerializationDevice.cpp    **
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

#include <ICLGeom/ProtoBufSerializationDevice.h>

namespace icl{
  using namespace utils;

  namespace geom{
    ProtoBufSerializationDevice::ProtoBufSerializationDevice(RSBPointCloud *protoBufObject):
      protoBufObject(protoBufObject){
    }

    void ProtoBufSerializationDevice::init(RSBPointCloud *protoBufObject){
      this->protoBufObject = protoBufObject;
    }
      
    bool ProtoBufSerializationDevice::isNull() const{
      return !protoBufObject;
    }
    
    void ProtoBufSerializationDevice::null_check(const std::string &function) throw (ICLException){
      if(isNull()) throw ICLException(function + ": instance is null");
    }
    

    void ProtoBufSerializationDevice::initializeSerialization(const PointCloudSerializer::MandatoryInfo &info){
      null_check(__FUNCTION__);
      protoBufObject->Clear();
      protoBufObject->set_width(info.width);
      protoBufObject->set_height(info.height);
      protoBufObject->set_organized(info.organized);
      protoBufObject->set_timestamp(info.timestamp);
    }

    PointCloudSerializer::MandatoryInfo ProtoBufSerializationDevice::getDeserializationInfo(){
      null_check(__FUNCTION__);
      PointCloudSerializer::MandatoryInfo mi = {
        protoBufObject->width(),
        protoBufObject->height(),
        protoBufObject->organized(),
        protoBufObject->timestamp()
      };
      return mi;
    }


    icl8u *ProtoBufSerializationDevice::targetFor(const std::string &featureName, int bytes){
      null_check(__FUNCTION__);
      if(featureName.length() >= 5 && featureName.substr(0,5) == "meta:"){
        RSBPointCloud_MetaDataEntry *m = protoBufObject->add_metadata();
        m->set_key(featureName.substr(5));
        m->set_value(std::string(bytes,'\0'));
        return (icl8u*) m->mutable_value()->c_str();
      }else{
        RSBPointCloud_Field *f = protoBufObject->add_fields();
        f->set_name(featureName);
        f->set_compression("none");
        f->set_data(std::string(bytes,'\0'));
        return (icl8u*) f->mutable_data()->c_str();
      }
    }
    
    std::vector<std::string> ProtoBufSerializationDevice::getFeatures(){
      null_check(__FUNCTION__);
      std::vector<std::string> fs;
      for(int i=0;i<protoBufObject->fields_size();++i){
        fs.push_back(protoBufObject->fields(i).name());
      }
      for(int i=0;i<protoBufObject->metadata_size(); ++i){
        fs.push_back("meta:"+protoBufObject->metadata(i).key());
      }      
      
      //std::cout << "--" << std::endl;
      //for(size_t i=0;i<fs.size();++i){
      //  std::cout << "received feature " << fs[i] << std::endl;
      //}
      return fs;
    }
    
    const icl8u * ProtoBufSerializationDevice::sourceFor(const std::string &featureName, int &bytes){
      //      DEBUG_LOG("what ????");
      null_check(__FUNCTION__);
      if(featureName.length() >= 5 && featureName.substr(0,5) == "meta:"){
        std::string name = featureName.substr(5);
        for(int i=0;i<protoBufObject->metadata_size();++i){
          //DEBUG_LOG("searching for meta-data entry " << name << " but found " <<  protoBufObject->metadata(i).key() );
          if(protoBufObject->metadata(i).key() == name){
            const std::string &value = protoBufObject->metadata(i).value();
            bytes = (int)value.length();
            return (icl8u*)value.c_str();
          }
        }
      }else{
        for(int i=0;i<protoBufObject->fields_size();++i){
          if(protoBufObject->fields(i).name() == featureName){
            const std::string &data = protoBufObject->fields(i).data();
            bytes = (int)data.length();
            return (icl8u*)data.c_str();
          }
        }        
      }
      throw ICLException("unable get find source for feature with name: " + featureName);
      return 0;
    }
  }
}
