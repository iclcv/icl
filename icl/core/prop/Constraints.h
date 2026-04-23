// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Color.h>
#include <icl/core/Image.h>

/// Property-constraint types that depend on `core/` types and therefore
/// can't live under `icl/utils/prop/` (wrong dependency direction).
/// Consumed the same way as the utils-side constraints —
/// `addProperty("bg", core::prop::Color{}, core::Color(0,0,0))` etc.
namespace icl::core::prop {

  /// RGB color picker (legacy XML tag `"color"`).  Storage is
  /// `core::Color` (three `icl8u` channels).  Current ICL call sites
  /// are RGB-only; an RGBA variant can be added when a caller actually
  /// needs alpha.
  struct Color {
    using value_type = core::Color;
  };

  /// Image readback (legacy XML tag `"image"`).  The property's value
  /// is a `core::Image` — typically a cached preview / output.  The
  /// GUI refreshes through qt::Prop's callback push channel whenever
  /// typed_value is written.
  ///
  /// Images don't round-trip through XML, so the adapter's `toString` /
  /// `fromString` are intentionally lossy no-ops (see
  /// `core/prop/Adapter.cpp`).  `Configurable::saveProperties` is
  /// expected to skip image-typed properties (matches today's behaviour
  /// for `"info"` / `"command"`).
  struct ImageView {
    using value_type = core::Image;
  };

} // namespace icl::core::prop
