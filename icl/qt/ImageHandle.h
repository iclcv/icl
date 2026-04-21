// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>
#include <icl/qt/GUIHandle.h>
#include <icl/core/Image.h>

namespace icl{
  /** \cond */
  namespace core{ class ImgBase; class Image; }
  /** \endcond */

  namespace qt{

    /** \cond */
    class ICLWidget;
    /** \endcond */

    /// Handle class for image components \ingroup HANDLES
    class ICLQt_API ImageHandle : public GUIHandle<ICLWidget>{
      public:
      /// Create an empty handle
      ImageHandle(){}

      /// create a new ImageHandel
      ImageHandle(ICLWidget *w, GUIWidget *guiw):GUIHandle<ICLWidget>(w,guiw){}

      /// make the wrapped ICLWidget show a given image
      void setImage(const core::ImgBase *image);

      /// make the wrapped ICLWidget show a given image (as set Image)
      void operator=(const core::ImgBase *image) { setImage(image); }

      /// make the wrapped ICLWidget show a given image (as set Image)
      void operator=(const core::ImgBase &image) { setImage(&image); }

      /// Image-based overload
      void operator=(const core::Image &image) { setImage(image.ptr()); }

      /// calles updated internally
      void render();

      /// passes callback registration to wrapped ICLWidget instance)
      /** allowed event names are all,move,drag,press,release,enter,leave */
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
