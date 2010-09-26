/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/GrabberDeviceDescription.h               **
** Module : ICLIO                                                  **
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

#ifndef ICL_GRABBER_DEVICE_DESCRIPTION_H
#define ICL_GRABBER_DEVICE_DESCRIPTION_H

#include <string>
#include <iostream>

namespace icl{ 
  
  /// defines and explains an available grabber device
  struct GrabberDeviceDescription{
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
  };

  /// ostream operator for GenericGrabber::FoundDevice instances
  inline std::ostream &operator<<(std::ostream &s, const GrabberDeviceDescription &d){
    return s << "FoundDevice(" << d.type << "," << d.id << "," << d.description << ")";
  }

}



#endif
