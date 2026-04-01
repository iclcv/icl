// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ColorLabel.h>

namespace icl{
  namespace qt{

    /// Class wrapping ColorLabel GUI compoenent handling \ingroup HANDLES
    class ColorHandle : public GUIHandle<ColorLabel>{
      public:

      /// Create an empty handle
      ColorHandle(){}

      /// Create a new LabelHandle
      ColorHandle(ColorLabel *l, GUIWidget *w):GUIHandle<ColorLabel>(l,w){}

      /// sets new rgb color
      inline void operator=(const core::Color &rgb){
        lab()->setColor(rgb);
      }

      /// sets new rgba color (alpha is only used if alpha is enabled for the gui component)
      void operator=(const core::Color4D &rgba){
        lab()->setColor(rgba);
      }

      /// returns current rgb color
      inline core::Color getRGB() const { return lab()->getRGB(); }

      /// convenienc function that is the same as getRGBA()
      inline core::Color4D getColor() const { return lab()->getRGBA(); }

      /// returns current rgba color
      inline core::Color4D getRGBA() const { return lab()->getRGBA(); }

      /// return whether wrapped ColorLabel supports alpha
      inline bool hasAlpha() const { return lab()->hasAlpha(); }

      private:
      /// utitlity function
      ColorLabel *lab() { return **this; }

      /// utitlity function
      const ColorLabel *lab() const { return **this; }
    };




  } // namespace qt
}
