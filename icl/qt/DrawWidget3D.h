// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/DrawWidget.h>
#include <icl/qt/GLCallback.h>
#include <vector>
#include <mutex>

namespace icl::qt {
  /// Extended ICLDrawWidget, able to draw 2D and 3D primitives using OpenGL
  /** Even though, it is possible to use the ICLDrawWidget3D for rendering
      3D stuff on top of an image directly, it is strongly recommended to use
      an instance of ICLGeom::Scene to manage camera, scene objects and lights.
      A scene instance provides a single ICLDrawWidget3D::GLCallback* that can easily
      be linked to the DrawWidget3D by using the ICLDrawWidget3D::link method.

      Most older methods got the deprecated status. They will be removed soon, since
      it turned out, that using ICLGeom's Scene class is much easier and less error-prone.
  */
  class ICLQt_API ICLDrawWidget3D : public ICLDrawWidget {
    public:

    /// Backward-compat alias — the actual class now lives at
    /// `icl::qt::GLCallback` (see `icl/qt/GLCallback.h`).  Preserves
    /// the historical `ICLDrawWidget3D::GLCallback` spelling for
    /// existing callers.
    using GLCallback = icl::qt::GLCallback;

    /// creates a new ICLDrawWidget embedded into the parent component
    ICLDrawWidget3D(QWidget *parent);

    /// overwrites the parent's method
    virtual void customPaintEvent(PaintEngine *e);

    /// add a single 3D callback, that is rendered (here, no queue swapping is neccessary)
    /** the linked callback can be removed by calling link(0). The ownership of the linked
        callback is never passed. */
    void link(GLCallback *cb);

    protected:
    /// special single callback linked to the 3D visualisation
    GLCallback *m_linkedCallback;

    /// internal mutex
    std::recursive_mutex m_linkMutex;
  };
  } // namespace icl::qt