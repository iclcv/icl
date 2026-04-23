// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Yaml.h>

#include <string>

namespace icl::utils::yaml::detail {

  /// Serialize `n` to a YAML 1.2-subset text representation per `opts`.
  /// Mappings are always block-style; sequences are flow-style if all
  /// elements are scalars and `size() <= opts.flowThreshold`, otherwise
  /// block.  Scalar quoting is chosen to ensure a round-trip-safe parse
  /// (plain if unambiguous, single-quoted if ambiguous but printable,
  /// double-quoted with escapes if control chars are present).
  ICLUtils_API std::string emit(const Node &n, const EmitOptions &opts);

}  // namespace icl::utils::yaml::detail
