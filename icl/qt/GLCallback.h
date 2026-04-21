// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>

namespace icl::qt {

  class ICLDrawWidget3D;

  /// Draw-and-link callback for `ICLDrawWidget3D`.
  ///
  /// Historically lived as `ICLDrawWidget3D::GLCallback` (nested).
  /// Pulled out to top-level scope so lightweight headers (notably
  /// `DataStore.h`'s `Data::link(GLCallback *)`) can reference it
  /// with just a forward declaration of `ICLDrawWidget3D`, without
  /// dragging in the full `DrawWidget3D.h` / `Widget.h` /
  /// `DrawWidget.h` stack.  `DrawWidget3D.h` provides a
  /// compatibility `using GLCallback = icl::qt::GLCallback;` inside
  /// the widget class, so existing callers spelling the type as
  /// `ICLDrawWidget3D::GLCallback` keep compiling.
  class ICLQt_API GLCallback {
  public:
    virtual ~GLCallback() = default;

    /// 2nd draw pass that can access the parent widget.
    virtual void draw(ICLDrawWidget3D *widget) { static_cast<void>(widget); }

    /// Custom setup-on-link hook.
    virtual void link(ICLDrawWidget3D *widget) { static_cast<void>(widget); }

    /// Custom teardown-on-unlink hook.
    virtual void unlink(ICLDrawWidget3D *widget) { static_cast<void>(widget); }
  };

}  // namespace icl::qt
