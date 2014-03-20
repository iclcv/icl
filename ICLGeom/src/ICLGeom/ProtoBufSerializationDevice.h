/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ProtobufSerializationDevice.h      **
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

#pragma once

#include <ICLGeom/PointCloudSerializer.h>
#include <ICLGeom/RSBPointCloud.pb.h>

#if !defined(ICL_HAVE_RSB) || !defined(ICL_HAVE_PROTOBUF)
  #if WIN32
    #pragma WARNING("This header should only be included if ICL_HAVE_RSB and ICL_HAVE_PROTOBUF are defined and available in ICL")
  #else
    #warning "This header should only be included if ICL_HAVE_RSB and ICL_HAVE_PROTOBUF are defined and available in ICL"
  #endif
#endif

namespace icl{
  
  namespace geom{
    class ICLGeom_API ProtobufSerializationDevice : public PointCloudSerializer::SerializationDevice,  
                                                    public PointCloudSerializer::DeserializationDevice{
      protected:
      
      void null_check(const std::string &function) throw (utils::ICLException);

      RSBPointCloud *protoBufObject;
      
      public:

      ProtobufSerializationDevice(RSBPointCloud *protoBufObject);
      
      void init(RSBPointCloud *protoBufObject);
      
      bool isNull() const;
      
      // serialization
      virtual void initializeSerialization(const PointCloudSerializer::MandatoryInfo &info);
      virtual icl8u *targetFor(const std::string &featureName, int bytes);
      
      // deserialization
      virtual PointCloudSerializer::MandatoryInfo getDeserializationInfo();
      virtual std::vector<std::string> getFeatures();
      virtual const icl8u *sourceFor(const std::string &featureName, int &bytes);
    };
  }
}
