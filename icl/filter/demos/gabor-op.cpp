// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/filter/GaborOp.h>

HBox gui;
GenericGrabber grabber;

void init(){
  gui << Display().minSize(32,24).label("Result Image").handle("image")
      << ( VBox().handle("sidebar")
           << ( HBox()
                << Display().minSize(15,15).label("Gabor Mask").handle("mask")
                << Fps(10).handle("fps")
              )
           << ( VBox()
                << FSlider(0.1, 100, 20).label("Wave-Length -Lambda-").minSize(15,2).out("lambda")
                << FSlider(0, 3.15, 0).label("Wave-Angle -Theta-").minSize(15,2).out("theta")
                << FSlider(0, 50, 0).label("Phase-Offset -Psi-").minSize(15,2).out("psi")
                << FSlider(0.01, 10, 0.5).label("Elipticity -Gamma-").minSize(15,2).out("gamma")
                << FSlider(0.1, 30, 5).label("Gaussian Std-Dev. -Sigma-").minSize(15,2).out("sigma")
                << Slider(3, 50, 10).label("Width").minSize(15,2).out("width")
                << Slider(3, 50, 10).label("Height").minSize(15,2).out("height")
              )
         )
      << Show();

  grabber.init(pa("-i"));
  // Use the template form of useDesired so ProgArg is converted via
  // ProgArg::operator T() with the right target type. parse<T>(pa(...))
  // would conversion-chain through parse<string_view>, which throws —
  // string_view isn't stream-extractable.
  grabber.useDesired<Size>(pa("-size"));
  grabber.useDesired<format>(pa("-format"));
  grabber.useDesired<depth>(pa("-depth"));
}

void run(){
  const float lambda = gui["lambda"];
  const float theta  = gui["theta"];
  const float psi    = gui["psi"];
  const float gamma  = gui["gamma"];
  const float sigma  = gui["sigma"];
  const int   width  = gui["width"];
  const int   height = gui["height"];

  static float saveParams[5] = {};
  static Size  saveSize = Size::null;
  static std::shared_ptr<GaborOp> g;

  const float params[5] = {lambda, theta, psi, gamma, sigma};
  const Size size(width, height);

  if(!g || size != saveSize || !std::equal(params, params + 5, saveParams)){
    // GaborOp takes one std::vector<float> per parameter (so it can produce
    // a filter bank — we only want one kernel here, hence the single-element
    // vectors).
    g = std::make_shared<GaborOp>(size,
                                  std::vector<float>{lambda},
                                  std::vector<float>{theta},
                                  std::vector<float>{psi},
                                  std::vector<float>{sigma},
                                  std::vector<float>{gamma});
    // getKernels() returns std::vector<Img32f>. Detach so our in-place
    // normalize doesn't mutate GaborOp's internal kernel.
    Img32f kernel = g->getKernels()[0].detached();
    kernel.normalizeAllChannels(Range<float>(0, 255));
    gui["mask"] = Image(kernel);
    std::copy(params, params + 5, saveParams);
    saveSize = size;
  }

  Image result = g->apply(grabber.grabImage());
  result.normalizeAllChannels(Range<icl64f>(0, 255));

  gui["image"] = result;
  gui["fps"].render();
}

int main(int n, char **ppc){
  return ICLApp(n, ppc,
                "[m]-input|-i(device,device-params) "
                "-format(format=rgb) "
                "-depth(depth=depth32f) "
                "-size(size=VGA)",
                init, run).exec();
}
