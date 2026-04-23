// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Yaml.h>

#include <string_view>

namespace icl::utils::yaml::detail {

  /// Parse `src` into `doc.root()`.  Scalar views produced by the parser
  /// either point into `src` directly (for plain scalars and quoted
  /// scalars without escapes) or into `doc`'s arena (for quoted scalars
  /// with escape sequences, and block scalars).
  ///
  /// Throws `ParseError` on malformed input.
  ICLUtils_API void parseInto(std::string_view src, Document &doc);

}  // namespace icl::utils::yaml::detail
