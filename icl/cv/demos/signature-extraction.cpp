// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Interactive signature extraction: load a photo, rotate, threshold to
// separate ink from background, smooth the alpha mask, preview with red
// overlay for rejected pixels, and save as transparent PNG.
//
// Usage:
//   signature-extraction-demo -input image.jpg

#include <icl/qt/Common.h>
#include <icl/filter/LocalThresholdOp.h>
#include <icl/cv/RegionDetector.h>
#include <icl/utils/FPSLimiter.h>
#include <icl/filter/RotateOp.h>
#include <iostream>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::filter;
using namespace icl::cv;

HSplit gui;
ImgQ image;


void init() {
  image = load(pa("-input"));

  gui << (HSplit()
       << Display().handle("orig").minSize(32, 24).label("orig")
       << Display().handle("preview").minSize(32, 24).label("Preview")
       << (VBox().minSize(14, 0)
          << Combo("0,90,180,270").handle("rotation").label("Rotation")
          << Slider(2, 100, 15).handle("maskSize").label("Local Region Size")
          << Slider(-30, 30, 0).handle("globalThresh").label("Global Threshold")
          << Slider(0, 5000, 50).handle("minRegion").label("Min Region Size")
          << Slider(0, 10, 0).handle("blur").label("Alpha Smoothing")
          << Slider(1, 4, 1).handle("scale").label("Scale down factor")
          << Button("Save PNG...").handle("save")))
     << Show();
}

void run() {
  static RotateOp rotate;
  rotate.setAngle(float(gui["rotation"].as<int>()) * M_PI / 180.f);
  float scale = 1.f / gui["scale"].as<int>();
  ImgQ rotated = rotate.apply(image).as32f();
  ImgQ scaled = qt::scale(rotated, rotated.getWidth()*scale, rotated.getHeight()*scale);
  ImgQ gray = qt::gray(scaled);

  static LocalThresholdOp localThresh;
  localThresh.setMaskSize(gui["maskSize"].as<int>());
  localThresh.setGlobalThreshold(gui["globalThresh"].as<float>());
  Image binary = localThresh.apply(cvt8u(gray));

  // Remove small regions (noise, dust, paper texture)
  int minRegion = gui["minRegion"].as<int>();
  if (minRegion > 0) {
    Img8u &bin8u = binary.as<icl8u>();
    static RegionDetector rd;
    rd.setConstraints(1, bin8u.getDim(), 255, 255);
    const auto &regions = rd.detect(&bin8u);
    for (const auto &reg : regions) {
      if (reg.getSize() < minRegion) {
        for (const auto &p : reg.getPixels()) {
          bin8u(p.x, p.y, 0) = 0;
        }
      }
    }
  }

  ImgQ blurred = qt::blur(cvt(binary.as8u()), gui["blur"]);

  std::cout << "1" << std::endl;


  
  rotated.setChannels(3);
  ImgQ result = scaled | blurred;

  std::cout << "Result: " << result.getWidth() << "x" << result.getHeight() << " (" << result.getChannels() << " channels)" << std::endl;
  ImgQ preview = copy(result);
  icl32f* r = preview.getData(0), *a = preview.getData(3);
  const int w = preview.getWidth(), h = preview.getHeight();
  for(int i=0; i<w*h;++i){    
    // tint rejected pixels red and make them transparent
    r[i] = a[i] > 10 ? r[i] : 255;
    a[i] = 255;
  }
  

  std::cout << "2" << std::endl;

  gui["orig"] = Image(scaled);
  gui["preview"] = Image(preview);

  if (gui["save"].as<ButtonHandle>().wasTriggered()) {
    std::function<void(int)> saveFn = [=](int) {
      std::string filename = saveFileDialog("PNG (*.png);;All (*)");
      if (!filename.empty()) {
        if (filename.find(".png") == std::string::npos &&
            filename.find(".PNG") == std::string::npos)
          filename += ".png";
        save(result, filename);
        std::cout << "Saved: " << filename << " (" << w << "x" << h << ")" << std::endl;
      }
    };
    ICLApplication::instance()->executeInGUIThread(saveFn, 0, true);
  }

  static FPSLimiter fps(30);
  fps.wait();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "-input|-i(filename)", init, run).exec();
}
