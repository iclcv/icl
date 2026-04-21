// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/PaintEngine.h>
#include <QFont>

class QPainter;
class QOpenGLWidget;

namespace icl::qt {

  /// PaintEngine implementation using QPainter for GL 4.1 Core Profile overlay
  /** QPainter on QOpenGLWidget is the Qt-recommended approach for 2D overlay
      on top of OpenGL content. Create this after all GL rendering is done;
      Qt handles GL state save/restore automatically. */
  class ICLQt_API QPainterPaintEngine : public PaintEngine {
  public:
    QPainterPaintEngine(QPainter *painter, QOpenGLWidget *widget);
    ~QPainterPaintEngine() override;

    void color(float r, float g, float b, float a=255) override;
    void fill(float r, float g, float b, float a=255) override;
    void fontsize(float size) override;
    void font(std::string name, float size = -1,
              TextWeight weight = Normal, TextStyle style = StyleNormal) override;

    void linewidth(float w) override;
    void pointsize(float s) override;

    void line(const utils::Point32f &a, const utils::Point32f &b) override;
    void point(const utils::Point32f &p) override;
    void image(const utils::Rect32f &r, core::ImgBase *image,
               AlignMode mode = Justify, core::scalemode sm=core::interpolateNN) override;
    void image(const utils::Rect32f &r, const QImage &image,
               AlignMode mode = Justify, core::scalemode sm=core::interpolateNN) override;
    void rect(const utils::Rect32f &r) override;
    void triangle(const utils::Point32f &a, const utils::Point32f &b,
                  const utils::Point32f &c) override;
    void quad(const utils::Point32f &a, const utils::Point32f &b,
              const utils::Point32f &c, const utils::Point32f &d) override;
    void ellipse(const utils::Rect32f &r) override;
    void text(const utils::Rect32f &r, const std::string text,
              AlignMode mode = Centered, float angle=0) override;

    void bci(float brightness=0, float contrast=0, float intensity=0) override;
    void bciAuto() override;

    void getColor(float *piColor) override;
    void getFill(float *piColor) override;

    float getFontSize() const override;
    float getLineWidth() const override;
    float getPointSize() const override;
    utils::Size getSize() override;
    utils::Size estimateTextBounds(const std::string &text) const override;

  private:
    QPainter *m_painter;
    QOpenGLWidget *m_widget;
    float m_linecolor[4];
    float m_fillcolor[4];
    float m_linewidth;
    float m_pointsize;
    float m_bci[3];
    bool m_bciauto;
    QFont m_font;
  };

} // namespace icl::qt
