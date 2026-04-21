// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>
#include <icl/qt/QuickContext.h>
#include <icl/utils/Point.h>

#include <string>
#include <vector>
#include <algorithm>

namespace icl::qt {

  /** @{ @name Drawing state (Quick2)
      All state is per-context (see QuickContext). */

  /// Sets the current draw color (r,g,b,alpha). If g<0, g=b=r.
  inline void color(float r, float g = -1, float b = -1, float alpha = 255) {
    auto &ctx = activeContext();
    ctx.drawColor[0] = r;
    ctx.drawColor[1] = g < 0 ? r : g;
    ctx.drawColor[2] = b < 0 ? r : b;
    ctx.drawColor[3] = alpha;
  }

  /// Sets the current fill color (r,g,b,alpha). If g<0, g=b=r.
  inline void fill(float r, float g = -1, float b = -1, float alpha = 255) {
    auto &ctx = activeContext();
    ctx.fillColor[0] = r;
    ctx.fillColor[1] = g < 0 ? r : g;
    ctx.fillColor[2] = b < 0 ? r : b;
    ctx.fillColor[3] = alpha;
  }

  /// Returns the current color and fill state
  inline void colorinfo(float col[4], float fil[4]) {
    auto &ctx = activeContext();
    std::copy(ctx.drawColor, ctx.drawColor + 4, col);
    std::copy(ctx.fillColor, ctx.fillColor + 4, fil);
  }

  /// Sets up the current font
  inline void font(int size, const std::string &family = "Arial") {
    auto &ctx = activeContext();
    ctx.fontSize = size;
    ctx.fontFamily = family;
  }

  /// Sets current font size
  inline void fontsize(int size) {
    activeContext().fontSize = size;
  }

  /** @} */

  /** @{ @name Drawing primitives (Quick2)
      All functions work on any Image depth via visit(). */

  /// Draws a small cross (6x6) at the given position
  ICLQt_API void cross(core::Image &image, int x, int y);
  inline void cross(core::Image &image, const utils::Point &p) { cross(image, p.x, p.y); }

  /// Draws a line
  ICLQt_API void line(core::Image &image, int x1, int y1, int x2, int y2);
  inline void line(core::Image &image, const utils::Point &p1, const utils::Point &p2) {
    line(image, p1.x, p1.y, p2.x, p2.y);
  }

  /// Draws a connected line strip
  ICLQt_API void linestrip(core::Image &image, const std::vector<utils::Point> &pts, bool closeLoop = true);

  /// Draws a filled polygon
  ICLQt_API void polygon(core::Image &image, const std::vector<utils::Point> &corners);

  /// Draws a rectangle (optionally with rounded corners)
  ICLQt_API void rect(core::Image &image, int x, int y, int w, int h, int rounding = 0);
  inline void rect(core::Image &image, const utils::Rect &r, int rounding = 0) {
    rect(image, r.x, r.y, r.width, r.height, rounding);
  }

  /// Draws a filled triangle
  ICLQt_API void triangle(core::Image &image, int x1, int y1, int x2, int y2, int x3, int y3);
  inline void triangle(core::Image &image, const utils::Point &a, const utils::Point &b, const utils::Point &c) {
    triangle(image, a.x, a.y, b.x, b.y, c.x, c.y);
  }

  /// Draws a filled circle
  ICLQt_API void circle(core::Image &image, int x, int y, int r);

  /// Draws a single pixel
  ICLQt_API void pix(core::Image &image, int x, int y);
  inline void pix(core::Image &image, const utils::Point &p) { pix(image, p.x, p.y); }

  /// Draws a set of points
  ICLQt_API void pix(core::Image &image, const std::vector<utils::Point> &pts);

  /// Draws sets of points
  ICLQt_API void pix(core::Image &image, const std::vector<std::vector<utils::Point>> &pts);

  /// Renders text (requires Qt)
  ICLQt_API void text(core::Image &image, int x, int y, const std::string &text);
  inline void text(core::Image &image, const utils::Point &p, const std::string &t) {
    text(image, p.x, p.y, t);
  }

  /// Labels an image in the upper-left corner (requires Qt)
  ICLQt_API core::Image label(const core::Image &image, const std::string &text);

  /** @} */

} // namespace icl::qt
