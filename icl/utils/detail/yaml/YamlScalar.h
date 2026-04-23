// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Yaml.h>

#include <string_view>

namespace icl::utils::yaml::detail {

  /// Resolve a YAML scalar's inferred kind per the 1.2 core schema.
  ///
  /// Non-plain styles always resolve to String.  Plain scalars are
  /// matched against (in order): null literals, bool literals, integer
  /// syntax (decimal / 0x / 0o), float syntax (incl. .inf / .nan), and
  /// otherwise String by exclusion.  YAML 1.1 aliases (yes/no/on/off)
  /// are intentionally **not** honored — they resolve as String.
  ICLUtils_API ScalarKind resolveScalarKind(std::string_view sv, ScalarStyle style);

}  // namespace icl::utils::yaml::detail
