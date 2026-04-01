// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/GUIWidget.h>
#include <ICLGeom/PlotWidget3D.h>

namespace icl{
  namespace geom{

    /// Handle class for image components \ingroup HANDLES
    class ICLGeom_API PlotHandle3D : public qt::GUIHandle<PlotWidget3D>{
      public:
      /// Create an empty handle
      PlotHandle3D(){}

      /// create a new ImageHandel
      PlotHandle3D(PlotWidget3D *w, qt::GUIWidget *guiw):qt::GUIHandle<PlotWidget3D>(w,guiw){}

      /// re-renders the widget
      inline void render(){
        (***this).render();
      }

      // todo: implement several set data method for more convenience

      /// callback registration is not supported for this compoment
      virtual void registerCallback(const qt::GUI::Callback &cb, const std::string &events="all"){
        throw utils::ICLException("PlotHandle3D::registerCallback: you cannot register"
                           " Callbacks to this component");
      }

      /// complex callbacks are not allowed for image-components (this method will throw an exception)
      virtual void registerCallback(const qt::GUI::ComplexCallback&, const std::string &){
        throw utils::ICLException("PlotHandle3D::registerCallback: you cannot register "
                           "GUI::ComplexCallback instances to a plot3D GUI component");
      }
    };
  } // namespace qt
}
