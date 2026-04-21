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

Img32f approxCell(const Img32f &cell){
  const int N = cell.getDim();
  Matrix xs(2, N);
  Matrix ys(3, N);

  for(int y = 0, idx = 0; y < cell.getHeight(); ++y)
    for(int x = 0; x < cell.getWidth(); ++x, ++idx){
      xs(idx, 0) = x;
      xs(idx, 1) = y;
      ys(idx, 0) = cell(x, y, 0);
      ys(idx, 1) = cell(x, y, 1);
      ys(idx, 2) = cell(x, y, 2);
    }

  const Reg::Result &result = reg->apply(xs, ys);
  const Matrix &z = result(xs);

  Img32f out(cell.getSize(), 3);
  for(int c = 0; c < 3; ++c)
    std::copy(z.col_begin(c), z.col_end(c), out.begin(c));
  return out;
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

  Img32f result(image.getSize(), formatRGB);

  try {
    Img32f cellBuf(Size(cellsize, cellsize), formatRGB);
    for(int cy = 0; cy < image.getHeight() / cellsize; ++cy){
      for(int cx = 0; cx < image.getWidth() / cellsize; ++cx){
        Rect r(cx * cellsize, cy * cellsize, cellsize, cellsize);

        // Extract cell
        image.setROI(r);
        image.deepCopyROI(&cellBuf);

        // Approximate
        Img32f approxed = approxCell(cellBuf);

        // Paste into result: copy channel by channel into the right position
        for(int c = 0; c < 3; ++c){
          const float *src = approxed.getData(c);
          for(int ly = 0; ly < cellsize; ++ly){
            float *dst = &result(cx*cellsize, cy*cellsize + ly, c);
            std::copy(src + ly*cellsize, src + (ly+1)*cellsize, dst);
          }
        }
      }
    }
  } catch(ICLException &e) {
    gui["status"] = str(e.what());
  }

  image.setFullROI();

  int ts = tok(reg->getFunctionString(), "+").size();
  int orig = cellsize * cellsize * 3;
  int compr = ts * 3 * sizeof(float);
  gui["compression"] = str(int(float(compr) / orig * 100)) + "%";
  gui["input"] = Image(image);
  gui["result"] = Image(result);
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "[m]-input|-i(2)", init, run).exec();
}
