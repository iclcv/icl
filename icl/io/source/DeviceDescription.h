// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Macros.h>
#include <string>
#include <iostream>

namespace icl::io {
  /// defines and explains an available grabber device
  struct ICLIO_API DeviceDescription{
    /// Constructor
    DeviceDescription(const std::string &deviceType, const std::string &deviceID, const std::string &description):
      type(deviceType),id(deviceID),description(description){}

    /// Empty constructor
    DeviceDescription(){}

    /// type of the device (e.g. dc, pwc, sr or dc800)
    std::string type;

    /// ID of the device (e.g. 0 or 1)
    std::string id;

    /// additional description of the device (obtained by calling getUniqueID)
    std::string description;

    std::string name() const {
      return "[" + type + "]:" +  id;
    }

    bool equals(const DeviceDescription other){
      return (type == other.type) && (id == other.id);
    }

    bool isEmpty(){
      return type.size() && id.size();
    }

  };

  /// ostream operator for ImageSource::FoundDevice instances
  inline std::ostream &operator<<(std::ostream &s, const DeviceDescription &d){
    return s << "FoundDevice(" << d.type << "," << d.id << "," << d.description << ")";
  }

  } // namespace icl::io