// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QPainterPaintEngine.h>
#include <icl/core/Img.h>
#include <icl/core/CCFunctions.h>
#include <QPainter>
#include <QOpenGLWidget>
#include <QFontMetrics>
#include <QPolygonF>
#include <algorithm>
#include <cstring>

namespace icl::qt {

  static QColor toQColor(const float *c) {
    return QColor(std::clamp((int)c[0], 0, 255),
                  std::clamp((int)c[1], 0, 255),
                  std::clamp((int)c[2], 0, 255),
                  std::clamp((int)c[3], 0, 255));
  }

  QPainterPaintEngine::QPainterPaintEngine(QPainter *painter, QOpenGLWidget *widget)
    : m_painter(painter), m_widget(widget),
      m_linewidth(1), m_pointsize(1), m_bciauto(false),
      m_font("Arial", 30) {
    std::fill(m_fillcolor, m_fillcolor + 4, 0.0f);
    std::fill(m_linecolor, m_linecolor + 4, 255.0f);
    std::fill(m_bci, m_bci + 3, 0.0f);

    m_painter->setRenderHint(QPainter::Antialiasing, true);
    m_painter->setRenderHint(QPainter::TextAntialiasing, true);
  }

  QPainterPaintEngine::~QPainterPaintEngine() {}

  void QPainterPaintEngine::color(float r, float g, float b, float a) {
    m_linecolor[0] = r; m_linecolor[1] = g; m_linecolor[2] = b; m_linecolor[3] = a;
  }

  void QPainterPaintEngine::fill(float r, float g, float b, float a) {
    m_fillcolor[0] = r; m_fillcolor[1] = g; m_fillcolor[2] = b; m_fillcolor[3] = a;
  }

  void QPainterPaintEngine::fontsize(float size) {
    m_font.setPointSizeF(size);
  }

  void QPainterPaintEngine::font(std::string name, float size,
                                  TextWeight weight, TextStyle style) {
    if (name.length()) m_font.setFamily(name.c_str());
    if (size > 0) m_font.setPointSizeF(size);
    switch (weight) {
      case Light:    m_font.setWeight(QFont::Light); break;
      case Normal:   m_font.setWeight(QFont::Normal); break;
      case DemiBold: m_font.setWeight(QFont::DemiBold); break;
      case Bold:     m_font.setWeight(QFont::Bold); break;
      case Black:    m_font.setWeight(QFont::Black); break;
    }
    switch (style) {
      case StyleNormal:  m_font.setStyle(QFont::StyleNormal); break;
      case StyleItalic:  m_font.setStyle(QFont::StyleItalic); break;
      case StyleOblique: m_font.setStyle(QFont::StyleOblique); break;
    }
  }

  void QPainterPaintEngine::linewidth(float w) { m_linewidth = w; }
  void QPainterPaintEngine::pointsize(float s) { m_pointsize = s; }

  void QPainterPaintEngine::line(const utils::Point32f &a, const utils::Point32f &b) {
    m_painter->setPen(QPen(toQColor(m_linecolor), m_linewidth));
    m_painter->drawLine(QPointF(a.x, a.y), QPointF(b.x, b.y));
  }

  void QPainterPaintEngine::point(const utils::Point32f &p) {
    m_painter->setPen(QPen(toQColor(m_linecolor), m_pointsize));
    m_painter->drawPoint(QPointF(p.x, p.y));
  }

  void QPainterPaintEngine::rect(const utils::Rect32f &r) {
    QRectF qr(r.x, r.y, r.width, r.height);
    m_painter->setBrush(toQColor(m_fillcolor));
    m_painter->setPen(QPen(toQColor(m_linecolor), m_linewidth));
    m_painter->drawRect(qr);
  }

  void QPainterPaintEngine::triangle(const utils::Point32f &a,
                                      const utils::Point32f &b,
                                      const utils::Point32f &c) {
    QPolygonF poly;
    poly << QPointF(a.x, a.y) << QPointF(b.x, b.y) << QPointF(c.x, c.y);
    m_painter->setBrush(toQColor(m_fillcolor));
    m_painter->setPen(QPen(toQColor(m_linecolor), m_linewidth));
    m_painter->drawPolygon(poly);
  }

  void QPainterPaintEngine::quad(const utils::Point32f &a, const utils::Point32f &b,
                                  const utils::Point32f &c, const utils::Point32f &d) {
    QPolygonF poly;
    poly << QPointF(a.x, a.y) << QPointF(b.x, b.y)
         << QPointF(c.x, c.y) << QPointF(d.x, d.y);
    m_painter->setBrush(toQColor(m_fillcolor));
    m_painter->setPen(QPen(toQColor(m_linecolor), m_linewidth));
    m_painter->drawPolygon(poly);
  }

  void QPainterPaintEngine::ellipse(const utils::Rect32f &r) {
    QRectF qr(r.x, r.y, r.width, r.height);
    m_painter->setBrush(toQColor(m_fillcolor));
    m_painter->setPen(QPen(toQColor(m_linecolor), m_linewidth));
    m_painter->drawEllipse(qr);
  }

  void QPainterPaintEngine::text(const utils::Rect32f &r, const std::string text,
                                  AlignMode mode, float angle) {
    m_painter->setFont(m_font);
    m_painter->setPen(toQColor(m_linecolor));
    m_painter->setBrush(Qt::NoBrush);

    int flags = 0;
    switch (mode) {
      case NoAlign:  flags = Qt::AlignLeft | Qt::AlignTop; break;
      case Centered: flags = Qt::AlignHCenter | Qt::AlignVCenter; break;
      case Justify:  flags = Qt::AlignHCenter | Qt::AlignVCenter; break;
    }

    QRectF qr(r.x, r.y, r.width, r.height);
    if (std::abs(angle) > 0.01f) {
      m_painter->save();
      QPointF center = qr.center();
      m_painter->translate(center);
      m_painter->rotate(angle);
      m_painter->translate(-center);
      m_painter->drawText(qr, flags, text.c_str());
      m_painter->restore();
    } else {
      m_painter->drawText(qr, flags, text.c_str());
    }
  }

  void QPainterPaintEngine::image(const utils::Rect32f &r, core::ImgBase *image,
                                   AlignMode mode, core::scalemode sm) {
    if (!image) return;
    int w = image->getWidth(), h = image->getHeight(), ch = image->getChannels();
    if (w <= 0 || h <= 0) return;

    // Convert ICL image to QImage ARGB32
    QImage qimg(w, h, QImage::Format_ARGB32);
    if (ch >= 3) {
      const auto &img8 = *image->as8u();
      for (int y = 0; y < h; y++) {
        QRgb *line = reinterpret_cast<QRgb*>(qimg.scanLine(y));
        for (int x = 0; x < w; x++) {
          int ri = img8(x, y, 0), gi = img8(x, y, 1), bi = img8(x, y, 2);
          int ai = (ch > 3) ? img8(x, y, 3) : 255;
          line[x] = qRgba(ri, gi, bi, ai);
        }
      }
    } else {
      const auto &img8 = *image->as8u();
      for (int y = 0; y < h; y++) {
        QRgb *line = reinterpret_cast<QRgb*>(qimg.scanLine(y));
        for (int x = 0; x < w; x++) {
          int v = img8(x, y, 0);
          line[x] = qRgba(v, v, v, 255);
        }
      }
    }

    QRectF dest(r.x, r.y, r.width, r.height);
    m_painter->drawImage(dest, qimg);
  }

  void QPainterPaintEngine::image(const utils::Rect32f &r, const QImage &image,
                                   AlignMode mode, core::scalemode sm) {
    QRectF dest(r.x, r.y, r.width, r.height);
    m_painter->drawImage(dest, image);
  }

  void QPainterPaintEngine::bci(float brightness, float contrast, float intensity) {
    m_bci[0] = brightness; m_bci[1] = contrast; m_bci[2] = intensity;
    m_bciauto = false;
  }

  void QPainterPaintEngine::bciAuto() { m_bciauto = true; }

  void QPainterPaintEngine::getColor(float *piColor) {
    std::memcpy(piColor, m_linecolor, 4 * sizeof(float));
  }

  void QPainterPaintEngine::getFill(float *piColor) {
    std::memcpy(piColor, m_fillcolor, 4 * sizeof(float));
  }

  float QPainterPaintEngine::getFontSize() const { return m_font.pointSizeF(); }
  float QPainterPaintEngine::getLineWidth() const { return m_linewidth; }
  float QPainterPaintEngine::getPointSize() const { return m_pointsize; }

  utils::Size QPainterPaintEngine::getSize() {
    return utils::Size(m_widget->width(), m_widget->height());
  }

  utils::Size QPainterPaintEngine::estimateTextBounds(const std::string &text) const {
    QFontMetrics fm(m_font);
    QRect br = fm.boundingRect(text.c_str());
    return utils::Size(br.width(), br.height());
  }

} // namespace icl::qt
