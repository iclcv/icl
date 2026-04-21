// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/StackTimer.h>
#include <icl/qt/Common2.h>
#include <icl/filter/ConvolutionOp.h>
#include <icl/filter/MedianOp.h>
#include <icl/filter/BilateralFilterOp.h>
#include <icl/filter/CannyOp.h>

GUI gui;
GenericGrabber grabber;
BilateralFilterOp bi_filter;
CannyOp canny(200,255,0);

void run() {
  BENCHMARK_THIS_FUNCTION;

  int roi_percent = gui["roi_size"].as<int>();
  canny.setThresholds(gui["canny_low_th"].as<int>(),gui["canny_high_th"].as<int>());

  Image color_original = grabber.grabImage();
  bool use_gray = gui["to_gray"].as<bool>();

  Size size = color_original.getSize();
  size.width = std::floor(size.width*((float)roi_percent/100.0));
  size.height = std::floor(size.height*((float)roi_percent)/100.0);
  size.width = size.width%2+size.width;
  size.height = size.height%2+size.height;
  Point offset(color_original.getWidth()/2.0-size.width/2.0,color_original.getHeight()/2.0-size.height/2.0);
  color_original.setROI(offset,size);

  Image input = use_gray ? gray(color_original) : color_original;

  Image edge_original;
  canny.apply(input, edge_original);

  Image color_median;
  {
    BENCHMARK_THIS_SECTION(median_call);
    MedianOp median(utils::Size(gui["median_radius"].as<int>(),gui["median_radius"].as<int>()));
    median.apply(input, color_median);
  }

  Image edge_median;
  canny.apply(color_median, edge_median);

  bi_filter.setRadius(gui["bi_radius"].as<int>());
  bi_filter.setSigmaR(gui["sigma_r"].as<float>());
  bi_filter.setSigmaS(gui["sigma_s"].as<float>());
  bi_filter.setUseLAB(gui["use_lab"].as<bool>());

  Image color_bilateral;
  {
    BENCHMARK_THIS_SECTION(bilateral_filter_call);
    bi_filter.apply(color_median, color_bilateral);
  }

  Image edge_bilateral;
  canny.apply(color_bilateral, edge_bilateral);

  gui["view1"] = input;
  gui["view2"] = color_median;
  gui["view3"] = color_bilateral;

  gui["viewedge1"] = edge_original;
  gui["viewedge2"] = edge_median;
  gui["viewedge3"] = edge_bilateral;

  gui["view1"].render();
  gui["view2"].render();
  gui["view3"].render();
  gui["viewedge1"].render();
  gui["viewedge2"].render();
  gui["viewedge3"].render();
  gui["fps"].render();
}

void init() {
  grabber.init(pa("-i"));

  if (pa("-s")) {
    utils::Size size = pa("-s");
    grabber.setDesiredSizeInternal(size);
  }

  gui << ( VBox()
           << ( HBox()
                << Canvas().label("Original").handle("view1").minSize(16, 12)
                << Canvas().label("Median").handle("view2").minSize(16, 12)
                << Canvas().label("Bilateral Filtered").handle("view3").minSize(16, 12)
                )
           << ( HBox()
                << Canvas().label("Original").handle("viewedge1").minSize(16, 12)
                << Canvas().label("Median").handle("viewedge2").minSize(16, 12)
                << Canvas().label("Bilateral Filtered").handle("viewedge3").minSize(16, 12)
                )
           << CheckBox("Use LAB",true).handle("use_lab")
           << CheckBox("Use gray image",false).handle("to_gray")
           << Slider(1,24,4).label("Bilateral Kernel Radius").handle("bi_radius")
           << Slider(1,24,4).label("Median Kernel Radius").handle("median_radius")
           << FSlider(0.1,200,5).label("sigma_r (bilateral) ").handle("sigma_r")
           << FSlider(0.1,200,5).label("sigma_s (bilateral) ").handle("sigma_s")
           << Slider(0,255,200).label("Canny low th").handle("canny_low_th")
           << Slider(0,255,255).label("Canny high th").handle("canny_high_th")
           << Slider(10,100,100).label("ROI of Img (Percent)").handle("roi_size")
           << Fps().handle("fps")
           );
  gui << Show();
}

int main(int argc, char **argv) {
  return ICLApp(argc,argv,"[m]-input|-i(2) -size|-s(1)",init,run).exec();
}
