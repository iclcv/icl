// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/dispatch/AutoParse.h>

#include <functional>
#include <map>
#include <string>

namespace icl::utils {

  /// Parameter map: key → stringly-typed value, with transparent lookup.
  ///
  /// Used across the framework as a "bag of parameters" passed through
  /// generic APIs (e.g. `FiducialDetector`, `MarkerGridDetector`).
  /// Typical construction uses a braced initializer list:
  /// \code
  ///   ParamMap p{{"size", Size(30,30)}, {"border ratio", 0.3f}};
  /// \endcode
  /// Values are stored as `AutoParse<std::string>` — each entry is a
  /// stringified form that any consumer can extract as the type it
  /// needs via implicit conversion (`int w = p["size"]`).
  using ParamMap = std::map<std::string, AutoParse<std::string>, std::less<>>;

}  // namespace icl::utils
