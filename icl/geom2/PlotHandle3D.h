// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <icl/qt/GUIWidget.h>
#include <icl/geom2/PlotWidget3D.h>

namespace icl::geom2 {

  /// Handle for the geom2 Plot3D GUI component \ingroup HANDLES
  class ICLGeom2_API PlotHandle3D : public qt::GUIHandle<PlotWidget3D> {
  public:
    PlotHandle3D() {}
    PlotHandle3D(PlotWidget3D *w, qt::GUIWidget *guiw)
      : qt::GUIHandle<PlotWidget3D>(w, guiw) {}

    inline void render() { (***this).render(); }

    virtual void registerCallback(const qt::GUI::Callback &, const std::string & = "all") {
      throw utils::ICLException("PlotHandle3D::registerCallback: you cannot register"
                                " Callbacks to this component");
    }
    virtual void registerCallback(const qt::GUI::ComplexCallback &, const std::string &) {
      throw utils::ICLException("PlotHandle3D::registerCallback: you cannot register "
                                "GUI::ComplexCallback instances to a plot3D GUI component");
    }
  };

} // namespace icl::geom2
