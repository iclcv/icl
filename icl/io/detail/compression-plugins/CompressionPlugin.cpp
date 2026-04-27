// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/compression-plugins/CompressionPlugin.h>
#include <icl/core/CoreFunctions.h>  // operator<<(depth), operator<<(format)
#include <icl/utils/StringUtils.h>
#include <algorithm>
#include <sstream>

namespace icl::io {
  using namespace icl::core;
  using utils::str;

  bool CompressionPlugin::Capabilities::accepts(depth d, int channels,
                                                format f) const {
    if (!depths.empty()
        && std::find(depths.begin(), depths.end(), d) == depths.end()) {
      return false;
    }
    if (minChannels && channels < minChannels) return false;
    if (maxChannels && channels > maxChannels) return false;
    if (!formats.empty()
        && std::find(formats.begin(), formats.end(), f) == formats.end()) {
      return false;
    }
    return true;
  }

  std::string CompressionPlugin::Capabilities::describe() const {
    if (depths.empty() && !minChannels && !maxChannels && formats.empty()) {
      return "any image";
    }
    std::ostringstream os;
    bool first = true;
    auto sep = [&]() { if (!first) os << ", "; first = false; };
    if (!depths.empty()) {
      sep();
      os << "depth ";
      for (size_t i = 0; i < depths.size(); ++i) {
        if (i) os << '/';
        os << depths[i];
      }
    }
    if (minChannels || maxChannels) {
      sep();
      if (minChannels && minChannels == maxChannels) {
        os << minChannels << "ch";
      } else if (minChannels && maxChannels) {
        os << minChannels << '-' << maxChannels << "ch";
      } else if (minChannels) {
        os << "≥" << minChannels << "ch";
      } else {
        os << "≤" << maxChannels << "ch";
      }
    }
    if (!formats.empty()) {
      sep();
      os << "format ";
      for (size_t i = 0; i < formats.size(); ++i) {
        if (i) os << '/';
        os << formats[i];
      }
    }
    return os.str();
  }
} // namespace icl::io
