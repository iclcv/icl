// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickDraw.h>
#include <icl/qt/QuickContext.h>
#include <icl/qt/QuickFilter.h>
#include <icl/core/Img.h>
#include <icl/core/LineSampler.h>
#include <icl/utils/Point32f.h>

#ifdef ICL_HAVE_QT
#include <icl/qt/QImageConverter.h>
#include <QPainter>
#include <QImage>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#endif

#include <cmath>
#include <algorithm>

using namespace icl::core;
using namespace icl::utils;

namespace icl::qt {

  // ---- templated pixel helpers ----

  namespace {
    template<class T>
    inline void blendPixel(Img<T> &img, int x, int y, const float *color, float alpha) {
      if(x < 0 || y < 0 || x >= img.getWidth() || y >= img.getHeight()) return;
      float A = alpha / 255.0f;
      int nc = std::min(img.getChannels(), 3);
      for(int c = 0; c < nc; ++c) {
        T &v = img(x, y, c);
        v = static_cast<T>((1.0f - A) * v + A * color[c]);
      }
    }

    template<class T>
    void drawHLine(Img<T> &img, int x1, int x2, int y, const float *color, float alpha) {
      if(y < 0 || y >= img.getHeight()) return;
      if(x1 > x2) std::swap(x1, x2);
      float A = alpha / 255.0f;
      int nc = std::min(img.getChannels(), 3);
      int xEnd = std::min(x2, img.getWidth() - 1);
      for(int x = std::max(x1, 0); x <= xEnd; ++x) {
        for(int c = 0; c < nc; ++c) {
          T &v = img(x, y, c);
          v = static_cast<T>((1.0f - A) * v + A * color[c]);
        }
      }
    }

    template<class T>
    void drawVLine(Img<T> &img, int x, int y1, int y2, const float *color, float alpha) {
      if(x < 0 || x >= img.getWidth()) return;
      if(y1 > y2) std::swap(y1, y2);
      float A = alpha / 255.0f;
      int nc = std::min(img.getChannels(), 3);
      int yEnd = std::min(y2, img.getHeight() - 1);
      for(int y = std::max(y1, 0); y <= yEnd; ++y) {
        for(int c = 0; c < nc; ++c) {
          T &v = img(x, y, c);
          v = static_cast<T>((1.0f - A) * v + A * color[c]);
        }
      }
    }

    template<class T>
    void drawLine(Img<T> &img, int x1, int y1, int x2, int y2, const float *color, float alpha) {
      if(x1 == x2) { drawVLine(img, x1, y1, y2, color, alpha); return; }
      if(y1 == y2) { drawHLine(img, x1, x2, y1, color, alpha); return; }

      LineSampler ls(img.getImageRect());
      LineSampler::Result xys = ls.sample(Point(x1, y1), Point(x2, y2));
      float A = alpha / 255.0f;
      int nc = std::min(img.getChannels(), 3);
      for(int c = 0; c < nc; ++c) {
        auto chan = img[c];
        for(int i = 0; i < xys.n; ++i) {
          T &v = chan(xys[i].x, xys[i].y);
          v = static_cast<T>((1.0f - A) * v + A * color[c]);
        }
      }
    }

    template<class T>
    void drawCircleFill(Img<T> &img, int cx, int cy, int radius,
                        const float *fillCol, float fillAlpha,
                        const float *drawCol, float drawAlpha) {
      float rr = float(radius) * radius;
      int h = img.getHeight(), w = img.getWidth();
      int nc = std::min(img.getChannels(), 3);

      // Fill
      if(fillAlpha > 0) {
        float A = fillAlpha / 255.0f;
        int ystart = std::max(-radius, -cy);
        int yend = std::min(radius, h - cy - 1);
        for(int dy = ystart; dy <= yend; ++dy) {
          int y = dy + cy;
          int dx = static_cast<int>(std::round(std::sqrt(rr - dy * dy)));
          int xend = std::min(cx + dx, w - 1);
          for(int x = std::max(0, cx - dx); x <= xend; ++x) {
            for(int c = 0; c < nc; ++c) {
              T &v = img(x, y, c);
              v = static_cast<T>((1.0f - A) * v + A * fillCol[c]);
            }
          }
        }
      }

      // Outline
      if(drawAlpha > 0) {
        float A = drawAlpha / 255.0f;
        float outline = 2.0f * M_PI * radius;
        int maxx = w - 1, maxy = h - 1;
        for(float f = 0; f < 2 * M_PI; f += 1.0f / outline) {
          int x = static_cast<int>(std::round(cx + std::cos(f) * radius));
          if(x < 0 || x > maxx) continue;
          int y = static_cast<int>(std::round(cy + std::sin(f) * radius));
          if(y < 0 || y > maxy) continue;
          for(int c = 0; c < nc; ++c) {
            T &v = img(x, y, c);
            v = static_cast<T>((1.0f - A) * v + A * drawCol[c]);
          }
        }
      }
    }

    template<class T>
    void drawTriangleFill(Img<T> &img, int x1, int y1, int x2, int y2, int x3, int y3,
                          const float *fillCol, float fillAlpha,
                          const float *drawCol, float drawAlpha) {
      if(fillAlpha > 0) {
        // Sort by y
        Point pts[3] = {{x1,y1},{x2,y2},{x3,y3}};
        std::sort(pts, pts + 3, [](const Point &a, const Point &b){ return a.y < b.y; });
        Point A = pts[0], B = pts[1], C = pts[2];

        auto hlinef = [&](float xa, float xb, float y) {
          drawHLine(img, static_cast<int>(std::round(xa)),
                    static_cast<int>(std::round(xb)),
                    static_cast<int>(std::round(y)), fillCol, fillAlpha);
        };

        float dx1, dx2, dx3;
        dx1 = (B.y - A.y > 0) ? float(B.x - A.x) / (B.y - A.y) : float(B.x - A.x);
        dx2 = (C.y - A.y > 0) ? float(C.x - A.x) / (C.y - A.y) : 0;
        dx3 = (C.y - B.y > 0) ? float(C.x - B.x) / (C.y - B.y) : 0;

        Point32f S(A.x, A.y), E(A.x, A.y);
        if(dx1 > dx2) {
          for(; S.y <= B.y; S.y++, E.y++, S.x += dx2, E.x += dx1) hlinef(S.x, E.x, S.y);
          E = Point32f(B.x, B.y);
          for(; S.y <= C.y; S.y++, E.y++, S.x += dx2, E.x += dx3) hlinef(S.x, E.x, S.y);
        } else {
          for(; S.y <= B.y; S.y++, E.y++, S.x += dx1, E.x += dx2) hlinef(S.x, E.x, S.y);
          S = Point32f(B.x, B.y);
          for(; S.y <= C.y; S.y++, E.y++, S.x += dx3, E.x += dx2) hlinef(S.x, E.x, S.y);
        }
      }
      if(drawAlpha > 0) {
        drawLine(img, x1, y1, x2, y2, drawCol, drawAlpha);
        drawLine(img, x1, y1, x3, y3, drawCol, drawAlpha);
        drawLine(img, x2, y2, x3, y3, drawCol, drawAlpha);
      }
    }
  } // anon namespace

  // ---- public drawing functions ----

  void pix(Image &image, int x, int y) {
    auto &ctx = activeContext();
    image.visit([&](auto &img) {
      blendPixel(img, x, y, ctx.drawColor, ctx.drawColor[3]);
    });
  }

  void pix(Image &image, const std::vector<Point> &pts) {
    auto &ctx = activeContext();
    image.visit([&](auto &img) {
      for(auto &p : pts) blendPixel(img, p.x, p.y, ctx.drawColor, ctx.drawColor[3]);
    });
  }

  void pix(Image &image, const std::vector<std::vector<Point>> &pts) {
    for(auto &v : pts) pix(image, v);
  }

  void line(Image &image, int x1, int y1, int x2, int y2) {
    auto &ctx = activeContext();
    image.visit([&](auto &img) {
      drawLine(img, x1, y1, x2, y2, ctx.drawColor, ctx.drawColor[3]);
    });
  }

  void cross(Image &image, int x, int y) {
    static const int S = 3;
    line(image, x - S, y - S, x + S, y + S);
    line(image, x - S, y + S, x + S, y - S);
  }

  void linestrip(Image &image, const std::vector<Point> &pts, bool closeLoop) {
    if(pts.empty()) return;
    for(size_t i = 0; i + 1 < pts.size(); ++i) line(image, pts[i], pts[i + 1]);
    if(closeLoop) line(image, pts.front(), pts.back());
  }

  void rect(Image &image, int x, int y, int w, int h, int rounding) {
    auto &ctx = activeContext();
    if(!rounding) {
      // Outline
      line(image, x, y, x + w - 1, y);
      line(image, x, y, x, y + h - 1);
      line(image, x + w - 1, y + h - 1, x + w - 1, y);
      line(image, x + w - 1, y + h - 1, x, y + h - 1);
      // Fill
      if(ctx.fillColor[3] > 0) {
        image.visit([&](auto &img) {
          for(int i = x + 1; i < x + w - 1; ++i)
            for(int j = y + 1; j < y + h - 1; ++j)
              blendPixel(img, i, j, ctx.fillColor, ctx.fillColor[3]);
        });
      }
    } else {
      // Rounded rect: fill center + corners with arcs
      // Simplified: use fill color for the body, draw color for outline
      int r = std::min(rounding, std::min(w / 2, h / 2));

      // Fill inner rects
      if(ctx.fillColor[3] > 0) {
        image.visit([&](auto &img) {
          // Central horizontal band
          for(int i = x + r; i < x + w - r; ++i)
            for(int j = y; j < y + h; ++j)
              blendPixel(img, i, j, ctx.fillColor, ctx.fillColor[3]);
          // Left + right side bands
          for(int i = x; i < x + r; ++i)
            for(int j = y + r; j < y + h - r; ++j)
              blendPixel(img, i, j, ctx.fillColor, ctx.fillColor[3]);
          for(int i = x + w - r; i < x + w; ++i)
            for(int j = y + r; j < y + h - r; ++j)
              blendPixel(img, i, j, ctx.fillColor, ctx.fillColor[3]);
          // Corner fills (quarter circles)
          for(int dy = 0; dy < r; ++dy) {
            int dx = static_cast<int>(std::sqrt(r * r - (r - dy) * (r - dy)));
            // Top-left
            for(int ix = r - dx; ix <= r; ++ix)
              blendPixel(img, x + ix, y + dy, ctx.fillColor, ctx.fillColor[3]);
            // Top-right
            for(int ix = 0; ix < dx; ++ix)
              blendPixel(img, x + w - r + ix, y + dy, ctx.fillColor, ctx.fillColor[3]);
            // Bottom-left
            for(int ix = r - dx; ix <= r; ++ix)
              blendPixel(img, x + ix, y + h - 1 - dy, ctx.fillColor, ctx.fillColor[3]);
            // Bottom-right
            for(int ix = 0; ix < dx; ++ix)
              blendPixel(img, x + w - r + ix, y + h - 1 - dy, ctx.fillColor, ctx.fillColor[3]);
          }
        });
      }
      // Outline
      line(image, x + r, y, x + w - r, y);
      line(image, x + r, y + h - 1, x + w - r, y + h - 1);
      line(image, x, y + r, x, y + h - r - 1);
      line(image, x + w - 1, y + r, x + w - 1, y + h - r - 1);
      // Corner arcs via pixel plotting
      image.visit([&](auto &img) {
        for(int dy = 0; dy < r; ++dy) {
          int dx = static_cast<int>(std::sqrt(r * r - (r - dy) * (r - dy)));
          blendPixel(img, x + r - dx, y + dy, ctx.drawColor, ctx.drawColor[3]);
          blendPixel(img, x + w - 1 - r + dx, y + dy, ctx.drawColor, ctx.drawColor[3]);
          blendPixel(img, x + r - dx, y + h - 1 - dy, ctx.drawColor, ctx.drawColor[3]);
          blendPixel(img, x + w - 1 - r + dx, y + h - 1 - dy, ctx.drawColor, ctx.drawColor[3]);
        }
      });
    }
  }

  void triangle(Image &image, int x1, int y1, int x2, int y2, int x3, int y3) {
    auto &ctx = activeContext();
    image.visit([&](auto &img) {
      drawTriangleFill(img, x1, y1, x2, y2, x3, y3,
                       ctx.fillColor, ctx.fillColor[3],
                       ctx.drawColor, ctx.drawColor[3]);
    });
  }

  void circle(Image &image, int x, int y, int r) {
    auto &ctx = activeContext();
    image.visit([&](auto &img) {
      drawCircleFill(img, x, y, r,
                     ctx.fillColor, ctx.fillColor[3],
                     ctx.drawColor, ctx.drawColor[3]);
    });
  }

  void polygon(Image &image, const std::vector<Point> &corners) {
    ICLASSERT_THROW(corners.size() >= 3, ICLException("qt::polygon needs at least 3 points"));
    auto &ctx = activeContext();
    if(ctx.fillColor[3] > 0) {
      // Fill using triangle fan from centroid
      Point32f center;
      for(auto &p : corners) { center.x += p.x; center.y += p.y; }
      center *= 1.0f / corners.size();

      // Temporarily use fill color as draw color for the fill triangles
      ctx.pushColorState();
      color(ctx.fillColor[0], ctx.fillColor[1], ctx.fillColor[2], ctx.fillColor[3]);
      for(size_t i = 0; i < corners.size(); ++i) {
        size_t j = (i + 1) % corners.size();
        triangle(image, static_cast<int>(center.x), static_cast<int>(center.y),
                 corners[i].x, corners[i].y, corners[j].x, corners[j].y);
      }
      ctx.popColorState();
    }
    linestrip(image, corners, true);
  }

  // ---- text rendering (Qt-dependent) ----

  void text(Image &image, int xoffs, int yoffs, const std::string &txt) {
#ifdef ICL_HAVE_QT
    auto &ctx = activeContext();
    int n = 0;
    char **ppc = nullptr;
    if(!qApp) new QApplication(n, ppc);

    QFont f(ctx.fontFamily.c_str(), ctx.fontSize, QFont::DemiBold);
    QFontMetrics m(f);
    QSize br = m.size(Qt::TextSingleLine, txt.c_str());

    QImage qimg(br.width() + 2, br.height() + 2, QImage::Format_ARGB32_Premultiplied);
    qimg.fill(0);

    QPainter painter(&qimg);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(f);
    painter.setPen(QColor(255, 255, 255, 254));
    painter.drawText(QPoint(1, qimg.height() - m.descent() - 1), txt.c_str());
    painter.end();

    QImageConverter qic(&qimg);
    const Img8u &t = *(qic.getImg<icl8u>());

    image.visit([&](auto &img) {
      using T = typename std::remove_reference_t<decltype(img)>::type;
      int nc = std::min(img.getChannels(), 3);
      for(int c = 0; c < nc; ++c) {
        for(int tx = 0; tx < t.getWidth(); ++tx) {
          for(int ty = 0; ty < t.getHeight(); ++ty) {
            int ix = tx + xoffs, iy = ty + yoffs;
            if(ix >= 0 && iy >= 0 && ix < img.getWidth() && iy < img.getHeight()) {
              T &v = img(ix, iy, c);
              float A = (static_cast<float>(t(tx, ty, std::min(c, t.getChannels() - 1))) / 255.0f)
                      * (ctx.drawColor[3] / 255.0f);
              v = static_cast<T>((1.0f - A) * v + A * ctx.drawColor[c]);
            }
          }
        }
      }
    });
#else
    (void)image; (void)xoffs; (void)yoffs; (void)txt;
    ERROR_LOG("text rendering requires Qt support");
#endif
  }

  Image label(const Image &imageIn, const std::string &txt) {
    Image image = copy(imageIn);
    auto &ctx = activeContext();
    ctx.pushColorState();

    ctx.fontSize = 10;
    ctx.fontFamily = "Arial";
    ctx.drawColor[3] = 255;

    // Shadow
    std::fill(ctx.drawColor, ctx.drawColor + 3, 1.0f);
    text(image, 6, 6, txt);

    // Foreground
    std::fill(ctx.drawColor, ctx.drawColor + 3, 255.0f);
    text(image, 5, 5, txt);

    ctx.popColorState();
    return image;
  }

} // namespace icl::qt
