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

  // ================================================================
  // DrawTarget<T,NC> — compile-time channel count, cached pointers,
  // channel-outer loops for cache-friendly bulk operations
  // ================================================================

  namespace {

    template<class T, int NC>
    struct DrawTarget {
      using value_type = T;
      static constexpr int num_channels = NC;
      T* ch[NC];
      int w, h;

      DrawTarget(Img<T> &img) : w(img.getWidth()), h(img.getHeight()) {
        for(int c = 0; c < NC; ++c) ch[c] = img.getData(c);
      }

      // ---- single pixel ----

      void blend(int x, int y, const float *color, float A) {
        if(unsigned(x) >= unsigned(w) || unsigned(y) >= unsigned(h)) return;
        int idx = x + y * w;
        for(int c = 0; c < NC; ++c) // unrolled at compile time
          ch[c][idx] = static_cast<T>((1.f - A) * ch[c][idx] + A * color[c]);
      }

      // ---- horizontal line (channel-outer) ----

      void hline(int x1, int x2, int y, const float *color, float A) {
        if(unsigned(y) >= unsigned(h)) return;
        if(x1 > x2) std::swap(x1, x2);
        int xStart = std::max(x1, 0);
        int xEnd = std::min(x2, w - 1);
        if(xStart > xEnd) return;
        int base = y * w;
        for(int c = 0; c < NC; ++c) {
          T *p = ch[c] + base;
          float col = color[c];
          for(int x = xStart; x <= xEnd; ++x)
            p[x] = static_cast<T>((1.f - A) * p[x] + A * col);
        }
      }

      // ---- vertical line (channel-outer) ----

      void vline(int x, int y1, int y2, const float *color, float A) {
        if(unsigned(x) >= unsigned(w)) return;
        if(y1 > y2) std::swap(y1, y2);
        int yStart = std::max(y1, 0);
        int yEnd = std::min(y2, h - 1);
        if(yStart > yEnd) return;
        for(int c = 0; c < NC; ++c) {
          T *p = ch[c] + x;
          float col = color[c];
          for(int y = yStart; y <= yEnd; ++y) {
            int idx = y * w;
            p[idx] = static_cast<T>((1.f - A) * p[idx] + A * col);
          }
        }
      }

      // ---- general line (channel-outer via LineSampler) ----

      void line(int x1, int y1, int x2, int y2, const float *color, float A) {
        if(x1 == x2) { vline(x1, y1, y2, color, A); return; }
        if(y1 == y2) { hline(x1, x2, y1, color, A); return; }

        LineSampler ls(Rect(0, 0, w, h));
        LineSampler::Result xys = ls.sample(Point(x1, y1), Point(x2, y2));
        for(int c = 0; c < NC; ++c) {
          T *p = ch[c];
          float col = color[c];
          for(int i = 0; i < xys.n; ++i) {
            int idx = xys[i].x + xys[i].y * w;
            p[idx] = static_cast<T>((1.f - A) * p[idx] + A * col);
          }
        }
      }

      // ---- circle fill (channel-outer for fill, per-pixel for outline) ----

      void circleFill(int cx, int cy, int radius,
                      const float *fillCol, float fillA,
                      const float *drawCol, float drawA) {
        float rr = float(radius) * radius;

        // Fill: channel-outer, scanline-inner
        if(fillA > 0) {
          int ystart = std::max(-radius, -cy);
          int yend = std::min(radius, h - cy - 1);
          for(int c = 0; c < NC; ++c) {
            T *p = ch[c];
            float col = fillCol[c];
            for(int dy = ystart; dy <= yend; ++dy) {
              int y = dy + cy;
              int dx = static_cast<int>(std::round(std::sqrt(rr - float(dy * dy))));
              int xstart = std::max(0, cx - dx);
              int xend = std::min(cx + dx, w - 1);
              int base = y * w;
              for(int x = xstart; x <= xend; ++x)
                p[base + x] = static_cast<T>((1.f - fillA) * p[base + x] + fillA * col);
            }
          }
        }

        // Outline: sparse pixels, per-pixel blend
        if(drawA > 0) {
          float outline = 2.0f * float(M_PI) * radius;
          for(float f = 0; f < 2.f * float(M_PI); f += 1.0f / outline)
            blend(int(std::round(cx + std::cos(f) * radius)),
                  int(std::round(cy + std::sin(f) * radius)),
                  drawCol, drawA);
        }
      }

      // ---- triangle fill ----

      void triangleFill(int x1, int y1, int x2, int y2, int x3, int y3,
                        const float *fillCol, float fillA,
                        const float *drawCol, float drawA) {
        if(fillA > 0) {
          Point pts[3] = {{x1,y1},{x2,y2},{x3,y3}};
          std::sort(pts, pts + 3, [](const Point &a, const Point &b){ return a.y < b.y; });
          Point pA = pts[0], pB = pts[1], pC = pts[2];

          auto hf = [&](float xa, float xb, float y) {
            hline(int(std::round(xa)), int(std::round(xb)), int(std::round(y)), fillCol, fillA);
          };

          float d1 = (pB.y - pA.y > 0) ? float(pB.x - pA.x) / (pB.y - pA.y) : float(pB.x - pA.x);
          float d2 = (pC.y - pA.y > 0) ? float(pC.x - pA.x) / (pC.y - pA.y) : 0;
          float d3 = (pC.y - pB.y > 0) ? float(pC.x - pB.x) / (pC.y - pB.y) : 0;

          Point32f S(pA.x, pA.y), E(pA.x, pA.y);
          if(d1 > d2) {
            for(; S.y <= pB.y; S.y++, E.y++, S.x += d2, E.x += d1) hf(S.x, E.x, S.y);
            E = Point32f(pB.x, pB.y);
            for(; S.y <= pC.y; S.y++, E.y++, S.x += d2, E.x += d3) hf(S.x, E.x, S.y);
          } else {
            for(; S.y <= pB.y; S.y++, E.y++, S.x += d1, E.x += d2) hf(S.x, E.x, S.y);
            S = Point32f(pB.x, pB.y);
            for(; S.y <= pC.y; S.y++, E.y++, S.x += d3, E.x += d2) hf(S.x, E.x, S.y);
          }
        }
        if(drawA > 0) {
          line(x1, y1, x2, y2, drawCol, drawA);
          line(x1, y1, x3, y3, drawCol, drawA);
          line(x2, y2, x3, y3, drawCol, drawA);
        }
      }

      // ---- rect fill (channel-outer) ----

      void rectFill(int x, int y, int rw, int rh, const float *color, float A) {
        int xStart = std::max(x + 1, 0);
        int xEnd = std::min(x + rw - 2, w - 1);
        int yStart = std::max(y + 1, 0);
        int yEnd = std::min(y + rh - 2, h - 1);
        if(xStart > xEnd || yStart > yEnd) return;
        for(int c = 0; c < NC; ++c) {
          T *p = ch[c];
          float col = color[c];
          for(int j = yStart; j <= yEnd; ++j) {
            int base = j * w;
            for(int i = xStart; i <= xEnd; ++i)
              p[base + i] = static_cast<T>((1.f - A) * p[base + i] + A * col);
          }
        }
      }

    }; // DrawTarget

    // ---- dispatch helper: depth × channel count ----

    template<class F>
    void withDrawTarget(Image &image, F &&f) {
      image.visit([&](auto &img) {
        using T = typename std::remove_reference_t<decltype(img)>::type;
        switch(std::min(img.getChannels(), 3)) {
          case 1:  { DrawTarget<T,1> dt(img); f(dt); break; }
          case 2:  { DrawTarget<T,2> dt(img); f(dt); break; }
          default: { DrawTarget<T,3> dt(img); f(dt); break; }
        }
      });
    }

  } // anon namespace

  // ================================================================
  // Public drawing functions
  // ================================================================

  void pix(Image &image, int x, int y) {
    auto &ctx = activeContext();
    float A = ctx.drawColor[3] / 255.f;
    withDrawTarget(image, [&](auto &dt) { dt.blend(x, y, ctx.drawColor, A); });
  }

  void pix(Image &image, const std::vector<Point> &pts) {
    auto &ctx = activeContext();
    float A = ctx.drawColor[3] / 255.f;
    withDrawTarget(image, [&](auto &dt) {
      for(auto &p : pts) dt.blend(p.x, p.y, ctx.drawColor, A);
    });
  }

  void pix(Image &image, const std::vector<std::vector<Point>> &pts) {
    for(auto &v : pts) pix(image, v);
  }

  void line(Image &image, int x1, int y1, int x2, int y2) {
    auto &ctx = activeContext();
    float A = ctx.drawColor[3] / 255.f;
    withDrawTarget(image, [&](auto &dt) { dt.line(x1, y1, x2, y2, ctx.drawColor, A); });
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

  void rect(Image &image, int x, int y, int rw, int rh, int rounding) {
    auto &ctx = activeContext();
    float drawA = ctx.drawColor[3] / 255.f;
    float fillA = ctx.fillColor[3] / 255.f;

    if(!rounding) {
      withDrawTarget(image, [&](auto &dt) {
        // Outline
        dt.line(x, y, x + rw - 1, y, ctx.drawColor, drawA);
        dt.line(x, y, x, y + rh - 1, ctx.drawColor, drawA);
        dt.line(x + rw - 1, y + rh - 1, x + rw - 1, y, ctx.drawColor, drawA);
        dt.line(x + rw - 1, y + rh - 1, x, y + rh - 1, ctx.drawColor, drawA);
        // Fill
        if(fillA > 0) dt.rectFill(x, y, rw, rh, ctx.fillColor, fillA);
      });
    } else {
      int r = std::min(rounding, std::min(rw / 2, rh / 2));
      withDrawTarget(image, [&](auto &dt) {
        // Fill: center band + side bands via hline
        if(fillA > 0) {
          for(int j = y; j < y + rh; ++j)
            dt.hline(x + r, x + rw - r - 1, j, ctx.fillColor, fillA);
          for(int j = y + r; j < y + rh - r; ++j) {
            dt.hline(x, x + r - 1, j, ctx.fillColor, fillA);
            dt.hline(x + rw - r, x + rw - 1, j, ctx.fillColor, fillA);
          }
          // Corner quarter-circle fills
          for(int dy = 0; dy < r; ++dy) {
            int dx = static_cast<int>(std::sqrt(float(r * r - (r - dy) * (r - dy))));
            dt.hline(x + r - dx, x + r, y + dy, ctx.fillColor, fillA);
            dt.hline(x + rw - r - 1, x + rw - r - 1 + dx, y + dy, ctx.fillColor, fillA);
            dt.hline(x + r - dx, x + r, y + rh - 1 - dy, ctx.fillColor, fillA);
            dt.hline(x + rw - r - 1, x + rw - r - 1 + dx, y + rh - 1 - dy, ctx.fillColor, fillA);
          }
        }
        // Outline
        dt.hline(x + r, x + rw - r, y, ctx.drawColor, drawA);
        dt.hline(x + r, x + rw - r, y + rh - 1, ctx.drawColor, drawA);
        dt.vline(x, y + r, y + rh - r - 1, ctx.drawColor, drawA);
        dt.vline(x + rw - 1, y + r, y + rh - r - 1, ctx.drawColor, drawA);
        // Corner arcs
        for(int dy = 0; dy < r; ++dy) {
          int dx = static_cast<int>(std::sqrt(float(r * r - (r - dy) * (r - dy))));
          dt.blend(x + r - dx, y + dy, ctx.drawColor, drawA);
          dt.blend(x + rw - 1 - r + dx, y + dy, ctx.drawColor, drawA);
          dt.blend(x + r - dx, y + rh - 1 - dy, ctx.drawColor, drawA);
          dt.blend(x + rw - 1 - r + dx, y + rh - 1 - dy, ctx.drawColor, drawA);
        }
      });
    }
  }

  void triangle(Image &image, int x1, int y1, int x2, int y2, int x3, int y3) {
    auto &ctx = activeContext();
    float drawA = ctx.drawColor[3] / 255.f;
    float fillA = ctx.fillColor[3] / 255.f;
    withDrawTarget(image, [&](auto &dt) {
      dt.triangleFill(x1, y1, x2, y2, x3, y3,
                      ctx.fillColor, fillA, ctx.drawColor, drawA);
    });
  }

  void circle(Image &image, int x, int y, int r) {
    auto &ctx = activeContext();
    float drawA = ctx.drawColor[3] / 255.f;
    float fillA = ctx.fillColor[3] / 255.f;
    withDrawTarget(image, [&](auto &dt) {
      dt.circleFill(x, y, r, ctx.fillColor, fillA, ctx.drawColor, drawA);
    });
  }

  void polygon(Image &image, const std::vector<Point> &corners) {
    ICLASSERT_THROW(corners.size() >= 3, ICLException("qt::polygon needs at least 3 points"));
    auto &ctx = activeContext();
    if(ctx.fillColor[3] > 0) {
      Point32f center;
      for(auto &p : corners) { center.x += p.x; center.y += p.y; }
      center *= 1.0f / corners.size();

      ctx.pushColorState();
      color(ctx.fillColor[0], ctx.fillColor[1], ctx.fillColor[2], ctx.fillColor[3]);
      for(size_t i = 0; i < corners.size(); ++i) {
        size_t j = (i + 1) % corners.size();
        triangle(image, int(center.x), int(center.y),
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
    int tw = t.getWidth(), th = t.getHeight();
    float colorA = ctx.drawColor[3] / 255.f;

    // Channel-outer: process each channel of the destination, then each text pixel
    withDrawTarget(image, [&](auto &dt) {
      constexpr int NCC = std::remove_reference_t<decltype(dt)>::num_channels;
      for(int c = 0; c < NCC; ++c) {
        const icl8u *tp = t.getData(std::min(c, t.getChannels() - 1));
        auto *dp = dt.ch[c];
        for(int ty = 0; ty < th; ++ty) {
          int iy = ty + yoffs;
          if(unsigned(iy) >= unsigned(dt.h)) continue;
          int dstBase = iy * dt.w;
          int srcBase = ty * tw;
          for(int tx = 0; tx < tw; ++tx) {
            int ix = tx + xoffs;
            if(unsigned(ix) >= unsigned(dt.w)) continue;
            float A = (tp[srcBase + tx] / 255.f) * colorA;
            auto &v = dp[dstBase + ix];
            using VT = typename std::remove_reference_t<decltype(dt)>::value_type;
            v = static_cast<VT>((1.f - A) * v + A * ctx.drawColor[c]);
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

    std::fill(ctx.drawColor, ctx.drawColor + 3, 1.0f);
    text(image, 6, 6, txt);

    std::fill(ctx.drawColor, ctx.drawColor + 3, 255.0f);
    text(image, 5, 5, txt);

    ctx.popColorState();
    return image;
  }

} // namespace icl::qt
