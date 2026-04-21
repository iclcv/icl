// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Image approximation via polynomial regression: each cell of the image
// is independently approximated by a polynomial in (x,y) → (r,g,b).

#include <icl/qt/Common2.h>
#include <icl/math/PolynomialRegression.h>

using Scalar = float;
using Matrix = DynMatrix<Scalar>;
using Reg = PolynomialRegression<Scalar>;

static const char *polyFuncs[] = {
  "1+x0+x1",
  "1+x0+x1+x0*x1",
  "1+x0+x1+x0*x0+x1*x1",
  "1+x0+x1+x0*x1+x0*x0+x1*x1",
  "1+x0+x1+x0*x1+x0*x0+x1*x1+x0*x0*x1+x0*x1*x1+x0*x0*x0+x1*x1*x1",
};
static const char *polyNames[] = {
  "linear", "bilinear", "quadratic", "full quadratic", "cubic",
};
static constexpr int NUM_POLY = sizeof(polyFuncs) / sizeof(polyFuncs[0]);

HSplit gui;
GenericGrabber grabber;
Reg *reg = nullptr;
int lastPolyIdx = -1;

Image approxCell(const Img32f &src, const Rect &r){
  const int w = r.width, h = r.height, N = w * h;
  Matrix xs(2, N);
  Matrix ys(3, N);

  for(int y = 0, idx = 0; y < h; ++y)
    for(int x = 0; x < w; ++x, ++idx){
      xs(idx, 0) = x;
      xs(idx, 1) = y;
      ys(idx, 0) = src(r.x + x, r.y + y, 0);
      ys(idx, 1) = src(r.x + x, r.y + y, 1);
      ys(idx, 2) = src(r.x + x, r.y + y, 2);
    }

  const Reg::Result &result = reg->apply(xs, ys);
  const Matrix &z = result(xs);

  Img32f out(Size(w, h), 3);
  for(int c = 0; c < 3; ++c)
    std::copy(z.col_begin(c), z.col_end(c), out.begin(c));
  return Image(out);
}

std::string polyComboStr() {
  std::string s;
  for(int i = 0; i < NUM_POLY; ++i){ if(i) s += ","; s += polyNames[i]; }
  return s;
}

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(formatRGB);
  grabber.useDesired(depth32f);
  grabber.useDesired(Size::QVGA);

  gui << Display().handle("input").minSize(16, 12)
      << Display().handle("result").minSize(16, 12)
      << ( VBox().minSize(14, 1)
           << Combo(polyComboStr(), 3).handle("poly").label("polynomial basis")
           << Slider(2, 64, 16).handle("cellsize").label("cell size")
           << Label("--").handle("fused").label("basis functions")
           << Label("--").handle("status").label("status")
           << Label("--").handle("compression").label("compression")
         )
      << Show();
}

void run(){
  Img32f image = grabber.grabImage().convert(depth32f).as32f();

  int polyIdx = gui["poly"].as<int>();
  int cellsize = gui["cellsize"];
  while(image.getWidth() % cellsize || image.getHeight() % cellsize) --cellsize;
  if(cellsize < 2) return;

  // (Re)create regression if polynomial changed
  if(!reg || polyIdx != lastPolyIdx){
    delete reg;
    reg = new Reg(polyFuncs[polyIdx]);
    gui["fused"] = reg->getFunctionString();
    gui["status"] = str("ok");
    lastPolyIdx = polyIdx;
  }

  Image input(image);
  Image result = zeros(image.getWidth(), image.getHeight(), 3, depth32f);

  try {
    for(int cy = 0; cy < image.getHeight() / cellsize; ++cy){
      for(int cx = 0; cx < image.getWidth() / cellsize; ++cx){
        Rect r(cx * cellsize, cy * cellsize, cellsize, cellsize);
        roi(result, r) = approxCell(image, r);
      }
    }
  } catch(ICLException &e) {
    gui["status"] = str(e.what());
  }

  int ts = tok(reg->getFunctionString(), "+").size();
  int orig = cellsize * cellsize * 3;
  int compr = ts * 3 * sizeof(float);
  gui["compression"] = str(int(float(compr) / orig * 100)) + "%";
  gui["input"] = input;
  gui["result"] = result;
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "[m]-input|-i(2)", init, run).exec();
}
