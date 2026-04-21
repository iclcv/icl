// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>
#include <icl/qt/GUIHandle.h>

namespace icl{
  /** \cond */
  namespace core{ class ImgBase; }
  /** \endcond */

  namespace qt{

    /** \cond */
    class ICLDrawWidget3D;
    /** \endcond */

    /// Handle class for image components \ingroup HANDLES
    class ICLQt_API DrawHandle3D : public GUIHandle<ICLDrawWidget3D>{
      public:

      /// Create an empty handle
      DrawHandle3D(){}

      /// create a new ImageHandel
      DrawHandle3D(ICLDrawWidget3D *w, GUIWidget *guiw):GUIHandle<ICLDrawWidget3D>(w,guiw){}

      /// make the wrapped ICLWidget show a given image
      void setImage(const core::ImgBase *image);

      /// make the wrapped ICLWidget show a given image (as set Image)
      void operator=(const core::ImgBase *image) { setImage(image); }

      /// Image-based overload
      void operator=(const core::Image &image) { setImage(image.ptr()); }

      /// re-renders the widget
      void render();

      /// passes callback registration to wrapped ICLWidget instance)
      virtual void registerCallback(const GUI::Callback &cb, const std::string &events="all");

      /// complex callbacks are not allowed for image-components (this method will throw an exception)
      virtual void registerCallback(const GUI::ComplexCallback&, const std::string &){
        throw utils::ICLException("ImageHandle::registerCallback: you cannot register "
                           "GUI::ComplexCallback instances to an image GUI component");
      }

      /// passes callback registration to wrapped ICLWidget instance)
      virtual void removeCallbacks();

    };

  } // namespace qt
}
