// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIComponent.h>
#include <icl/utils/Range.h>
#include <icl/viz3d/PlotWidget3D.h>
#include <icl/viz3d/PlotHandle3D.h>

namespace icl::viz3d {

  /// GUI component for a viz3d 3D box plot (needs the icl-viz3d library).
  /** Creates a viz3d::PlotHandle3D wrapping a PlotWidget3D (its own Scene2 +
      camera + mouse interaction). Usage:
      \code
      gui << Plot3D().handle("plot") << Show();
      PlotHandle3D plot = gui["plot"];
      \endcode */
  struct Plot3D : public qt::GUIComponentT<Plot3D> {
    utils::Range32f xrange, yrange, zrange;

    Plot3D(const utils::Range32f &xrange = utils::Range32f(0, 0),
           const utils::Range32f &yrange = utils::Range32f(0, 0),
           const utils::Range32f &zrange = utils::Range32f(0, 0))
      : qt::GUIComponentT<Plot3D>("plot3D2"), xrange(xrange), yrange(yrange), zrange(zrange) {}

    /// defined in PlotWidget3D.cpp (next to Plot3DGUIWidget)
    qt::GUIWidget *createWidget(const qt::CreateContext &ctx) const override;
  };

} // namespace icl::viz3d
