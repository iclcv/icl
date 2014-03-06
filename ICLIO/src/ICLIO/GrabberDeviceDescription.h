/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/GrabberDeviceDescription.h             **
** Module : ICLIO                                                  **
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

#include <ICLUtils/Macros.h>
#include <string>
#include <iostream>

namespace icl{ 
  namespace io{
    
    /// defines and explains an available grabber device
    struct ICLIO_API GrabberDeviceDescription{
      /// Constructor
      GrabberDeviceDescription(const std::string &deviceType, const std::string &deviceID, const std::string &description):
        type(deviceType),id(deviceID),description(description){}
  
      /// Empty constructor
      GrabberDeviceDescription(){}
      
      /// type of the device (e.g. dc, pwc, sr or dc800)
      std::string type;
      
      /// ID of the device (e.g. 0 or 1)
      std::string id;
      
      /// additional description of the device (obtained by calling getUniqueID)
      std::string description;

      std::string name() const {
        return "[" + type + "]:" +  id;
      }

      bool equals(const GrabberDeviceDescription other){
        return (type == other.type) && (id == other.id);
      }

      bool isEmpty(){
        return type.size() && id.size();
      }

    };
  
    /// ostream operator for GenericGrabber::FoundDevice instances
    inline std::ostream &operator<<(std::ostream &s, const GrabberDeviceDescription &d){
      return s << "FoundDevice(" << d.type << "," << d.id << "," << d.description << ")";
    }
  
  } // namespace io
}



