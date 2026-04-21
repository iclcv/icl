// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Size32f.h>
#include <ICLUtils/Rect32f.h>
#include <ICLCore/Types.h>
#include <string>
#include <stdio.h>
#include <QImage>

namespace icl{
  /** \cond */
  namespace core{ class ImgBase; }
  /** \endcond */

  namespace qt{
    /// pure virtual Paint engine interface \ingroup UNCOMMON
    class PaintEngine{
      public:
      virtual ~PaintEngine(){}
      enum AlignMode {NoAlign, Centered, Justify};
      enum TextWeight {Light, Normal, DemiBold, Bold, Black};
      enum TextStyle {StyleNormal, StyleItalic, StyleOblique };

      virtual void color(float r, float g, float b, float a=255)=0;
      virtual void fill(float r, float g, float b, float a=255)=0;
      virtual void fontsize(float size)=0;
      virtual void font(std::string name, float size = -1, TextWeight weight = Normal, TextStyle style = StyleNormal)=0;

      virtual void linewidth(float w)=0;
      virtual void pointsize(float s)=0;
      virtual void line(const utils::Point32f &a, const utils::Point32f &b)=0;
      virtual void point(const utils::Point32f &p)=0;
      virtual void image(const utils::Rect32f &r,core::ImgBase *image, AlignMode mode = Justify,
                         core::scalemode sm=core::interpolateNN)=0;
      virtual void image(const utils::Rect32f &r,const QImage &image, AlignMode mode = Justify,
                         core::scalemode sm=core::interpolateNN)=0;
      virtual void rect(const utils::Rect32f &r)=0;
      virtual void triangle(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c)=0;
      virtual void quad(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c, const utils::Point32f &d)=0;
      virtual void ellipse(const utils::Rect32f &r)=0;
      virtual void text(const utils::Rect32f &r, const std::string text, AlignMode mode = Centered, float angle=0)=0;

      /// brightness-constrast intensity adjustment (for images only)
      virtual void bci(float brightness=0, float contrast=0, float floatensity=0)=0;
      virtual void bciAuto()=0;

      virtual void getColor(float *piColor)=0;
      virtual void getFill(float *piColor)=0;

      virtual float getFontSize() const =0;
      virtual float getLineWidth() const =0;
      virtual float getPointSize() const =0;
      virtual utils::Size getSize() =0;
      virtual utils::Size estimateTextBounds(const std::string &text) const =0;

    };
  } // namespace qt
}// namespace
