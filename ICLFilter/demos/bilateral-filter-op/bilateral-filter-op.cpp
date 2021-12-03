/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/demos/affine-op/affine-op.cpp                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <iostream>
#include <fstream>
#include <limits>

//------------------------------------------------------------------------------

#include <ICLCore/CCFunctions.h>

#include <ICLUtils/StackTimer.h>
#include <ICLQt/Common.h>

#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/BilateralFilterOp.h>

#include <ICLFilter/CannyOp.h>

#include <ICLGeom/Camera.h>
#include <ICLGeom/PointCloudCreator.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/PointCloudNormalEstimator.h>
#include <ICLGeom/Scene.h>

using namespace icl;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::io;
using namespace icl::filter;
using namespace icl::qt;

//==============================================================================

GUI gui;

GenericGrabber grabber;

BilateralFilterOp *bi_filter;

CannyOp canny(200,255,0);

//==============================================================================

template<typename T>
void grab_cb(const ImgBase *img) {

	BENCHMARK_THIS_FUNCTION;

	int roi_percent = gui["roi_size"].as<int>();

	canny.setThresholds(gui["canny_low_th"].as<int>(),gui["canny_high_th"].as<int>());

	Img<T> color_original = *img->asImg<T>();

	Size size = color_original.getSize();

	size.width = std::floor(size.width*((float)roi_percent/100.0));
	size.height = std::floor(size.height*((float)roi_percent)/100.0);

	size.width = size.width%2+size.width;
	size.height = size.height%2+size.height;

	Point offset(color_original.getWidth()/2.0-size.width/2.0,color_original.getHeight()/2.0-size.height/2.0);

	color_original.setROI(offset,size);

	Img<T> color_median(color_original.getParams());
	MedianOp median(utils::Size(gui["median_radius"].as<int>(),gui["median_radius"].as<int>()));

	bool use_gray = gui["to_gray"].as<bool>();
	Img<T> gray_image(color_original.getSize(),core::formatGray);
	//color_original.setFormat(core::formatRGB);
	Img8u edge_;
	if (use_gray) {
		core::cc(&color_original,&gray_image);
		canny.apply(&gray_image,bpp(edge_));
	} else {
		canny.apply(&color_original,bpp(edge_));
	}

	{
		BENCHMARK_THIS_SECTION(median_call);
		if (use_gray)
			median.apply(&gray_image,bpp(color_median));
		else
			median.apply(&color_original,bpp(color_median));

	}
	Img8u edge_median;//(color_median.getSize(),core::formatGray);
	canny.apply(&color_median,bpp(edge_median));

	bi_filter->setRadius(gui["bi_radius"].as<int>());
	bi_filter->setSigmaR(gui["sigma_r"].as<float>());
	bi_filter->setSigmaS(gui["sigma_s"].as<float>());
	bi_filter->setUseLAB(gui["use_lab"].as<bool>());
	Img<T> color_bilateral(color_median.getParams());
	{
		BENCHMARK_THIS_SECTION(bilateral_filter_call);
		if (use_gray)
			bi_filter->apply(&color_median,bpp(color_bilateral));
		else
			bi_filter->apply(&color_median,bpp(color_bilateral));
	}

	Img8u edge_bi_filtered;//(edge_bi_filtered.getSize(),core::formatGray);
	canny.apply(&color_bilateral,bpp(edge_bi_filtered));

	// set images
	if (use_gray)
		gui["view1"] = &gray_image;
	else
		gui["view1"] = &color_original;
	gui["view2"] = &color_median;
	gui["view3"] = &color_bilateral;

	gui["viewedge1"] = &edge_;
	gui["viewedge2"] = &edge_median;
	gui["viewedge3"] = &edge_bi_filtered;

	// update view:
	gui["view1"].render();
	gui["view2"].render();
	gui["view3"].render();

	gui["viewedge1"].render();
	gui["viewedge2"].render();
	gui["viewedge3"].render();

}

//==============================================================================

void init() {

	grabber.init(pa("-i"));

	if (pa("-s")) {
		utils::Size size = pa("-s");
		grabber.setDesiredSizeInternal(size);
	}

    // create the GUI
	gui << ( VBox()
			 << ( HBox()
				  << Draw().label("Original").handle("view1").minSize(16, 12)
				  << Draw().label("Median").handle("view2").minSize(16, 12)
				  << Draw().label("Bilateral Filtered").handle("view3").minSize(16, 12)
				  )
			 << ( HBox()
				  << Draw().label("Original").handle("viewedge1").minSize(16, 12)
				  << Draw().label("Median").handle("viewedge2").minSize(16, 12)
				  << Draw().label("Bilateral Filtered").handle("viewedge3").minSize(16, 12)
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

	bi_filter = new BilateralFilterOp();

}

//==============================================================================

void run() {

	const ImgBase* img = grabber.grab();

	switch(img->getDepth()) {
		case(core::depth8u): {
			grab_cb<icl8u>(img);
			break;
		}
		case(core::depth32f): {
			grab_cb<icl32f>(img);
			break;
		}
		default: break;
	}

	gui["fps"].render();
}

//==============================================================================
int main(int argc, char **argv) {

	ICLApp app(argc,argv,"[m]-input|-i(2) -size|-s(1)",init,run);
    return app.exec();

    return 0;
}
