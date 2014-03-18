/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ProtobufSerializationDevice.cpp    **
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

#include <ICLGeom/ProtoBufSerializationDevice.h>

namespace icl{
  namespace geom{
    ProtobufSerializationDevice::ProtobufSerializationDevice(io::RSBPointCloud *protoBufObject):
      protoBufObject(protoBufObject){
      
    }
    void  ProtobufSerializationDevice::initialize(const PointCloudObjectBase &o){
    
    }
    icl8u *ProtobufSerializationDevice::targetFor(const std::string &featureName, int bytes){
      return 0;
    }
    void  ProtobufSerializationDevice::prepareTarget(PointCloudObjectBase &dst){
    
    }
    std::vector<std::string> ProtobufSerializationDevice::getFeatures(){
      return std::vector<std::string>();
    }
    const icl8u * ProtobufSerializationDevice::sourceFor(const std::string &featureName, int bytes){
      return 0;
    }
  }
}
