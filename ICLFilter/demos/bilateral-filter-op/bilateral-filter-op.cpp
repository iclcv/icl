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

#include <ICLUtils/StackTimer.h>
#include <ICLQt/Common.h>

#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/BilateralFilterOp.h>

#include <ICLGeom/Camera.h>
#include <ICLGeom/PointCloudCreator.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/PointCloudNormalEstimator.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/PCLPointCloudObject.h>

//------------------------------------------------------------------------------

//==============================================================================

GUI gui;

GenericGrabber grabber;

BilateralFilterOp *bi_filter;

//==============================================================================

template<typename T>
void grab_cb(const ImgBase *img) {

	BENCHMARK_THIS_FUNCTION;

	Img<T> color_original = *img->asImg<T>();

	Img<T> color_median(color_original.getParams());
	Img<T> color_bilateral(color_original.getParams());
	MedianOp median(utils::Size(gui["median_radius"].as<int>(),gui["median_radius"].as<int>()));

	{
		BENCHMARK_THIS_SECTION(median_call);
		median.apply(&color_original,bpp(color_median));
	}

	bi_filter->setRadius(gui["bi_radius"].as<int>());
	bi_filter->setSigmaR(gui["sigma_r"].as<float>());
	bi_filter->setSigmaS(gui["sigma_s"].as<float>());
	bi_filter->setUseLAB(gui["use_lab"].as<bool>());
	{
		BENCHMARK_THIS_SECTION(bilateral_filter_call);
		bi_filter->apply(&color_original,bpp(color_bilateral));
	}

	// set images
	gui["view1"] = &color_original;
	gui["view2"] = &color_median;
	gui["view3"] = &color_bilateral;

	// update view:
	gui["view1"].render();
	gui["view2"].render();
	gui["view3"].render();

}



//==============================================================================

void init() {

	grabber.init(pa("-i"));
	int depth = grabber.getDesiredDepthInternal();
	SHOW(depth);

    // create the GUI
	gui << ( VBox()
			 << ( HBox()
				  << Draw().label("Original").handle("view1").minSize(16, 12)
				  << Draw().label("Median").handle("view2").minSize(16, 12)
				  << Draw().label("Bilateral Filtered").handle("view3").minSize(16, 12)
				  )
			 << CheckBox("Use LAB",true).handle("use_lab")
			 << Slider(1,24,4).label("Bilateral Kernel Radius").handle("bi_radius")
			 << Slider(1,24,4).label("Median Kernel Radius").handle("median_radius")
			 << FSlider(0.1,200,5).label("sigma_r (bilateral)").handle("sigma_r")
			 << FSlider(0.1,200,5).label("sigma_s (bilateral)").handle("sigma_s")
			 << Fps().handle("fps")
			 );
    gui << Show();

	bi_filter = new BilateralFilterOp();

}

//==============================================================================

void run() {

	const ImgBase* img = 0;
	{
		BENCHMARK_THIS_SECTION(grabber);
		img = grabber.grab();
	}

	switch(img->getDepth()) {
		case(core::depth8u): {
			grab_cb<icl8u>(img);
			break;
		}
		case(core::depth16s): {
			grab_cb<icl16s>(img);
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

	ICLApp app(argc,argv,"[m]-input|-i(2)",init,run);
    return app.exec();

    return 0;
}

