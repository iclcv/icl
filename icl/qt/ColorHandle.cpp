// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// ColorHandle is otherwise header-only; this TU exists solely to host
// the runtime enrollment of its `Assign<>` pairs.  Color ↔ Color4D
// cross-conversion and string parsing that the legacy DataStore
// supported are deliberately not ported here: they belong in the core
// module (on Color/Color4D themselves) rather than leaking qt-specific
// registration into cross-class territory.  Callers that need string
// round-trip go via `utils::str(handle.as<Color4D>())` and
// `handle = utils::parse<Color4D>(...)` explicitly.

#include <icl/qt/ColorHandle.h>

#include <icl/utils/AssignRegistry.h>

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::ColorHandle;
  namespace core = icl::core;
  __attribute__((constructor))
  static void icl_register_color_handle_assignments() {
    AssignRegistry::enroll_symmetric<ColorHandle, core::Color, core::Color4D>();
    AssignRegistry::enroll_identity<ColorHandle>();
  }
}
