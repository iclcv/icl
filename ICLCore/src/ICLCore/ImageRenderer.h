// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>


namespace icl{
  namespace core{

    class ICLCore_API ImageRenderer {
      struct Data;
      Data *m_data;

      public:
      ImageRenderer(const ImageRenderer&) = delete;
      ImageRenderer& operator=(const ImageRenderer&) = delete;

      ImageRenderer(ImgBase *image=0);
      ~ImageRenderer();

      void setImage(ImgBase *image);

      void color(int r, int g, int b, int a);
      void fill(int r, int g, int b, int a);
      void symsize(float size);
      void fontsize(float size);

      void sym(char sym, int x, int y);
      void rect(int x, int y, int w, int h);
      void triangle(int x1, int y1, int x2, int y2, int x3, int y3);
      void line(int x1, int y1, int x2, int y2);
      void linestrip(int n, int *xs, int *ys, int xStride=1, int yStride=1);
      void pix(int x1, int x2);
      void pix(int n, int *xs, int *ys, int xStride=1, int yStride=1);
      void circle(int cx, int cy, int r);
      void ellipse(int x, int y, int w, int h);
      void text(int x, int y, const std::string &text);
    };

  }
}
