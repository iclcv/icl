// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <icl/qt/PlotWidget.h>

namespace icl::qt {
  /// Handle class for image components \ingroup HANDLES
  class ICLQt_API PlotHandle : public GUIHandle<PlotWidget>{
    public:
    /// Create an empty handle
    PlotHandle(){}

    /// create a new ImageHandel
    PlotHandle(PlotWidget *w, GUIWidget *guiw):GUIHandle<PlotWidget>(w,guiw){}

    /// re-renders the widget
    void render();

    // todo: implement several set data method for more convenience

    /// callback registration is not supported for this compoment
    virtual void registerCallback(const GUI::Callback &cb, const std::string &events="all"){
      throw utils::ICLException("PlotHandle::registerCallback: you cannot register"
                         " Callbacks to this component");
    }

    /// complex callbacks are not allowed for image-components (this method will throw an exception)
    virtual void registerCallback(const GUI::ComplexCallback&, const std::string &){
      throw utils::ICLException("PlotHandle::registerCallback: you cannot register "
                         "GUI::ComplexCallback instances to an image GUI component");
    }
  };
  } // namespace icl::qt