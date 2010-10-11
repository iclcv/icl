/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLAlgorithms/examples/softposit-demo.cpp      **
 ** Module : ICLAlgorithms                                          **
 ** Authors: Christian Groszewski                                   **
 **                                                                 **
 **                                                                 **
 ** Commercial License                                              **
 ** ICL can be used commercially, please refer to our website       **
 ** www.iclcv.org for more details.                                 **
 **                                                                 **
 ** GNU General Public License Usage                                **
 ** Alternatively, this file may be used under the terms of the     **
 ** GNU General Public License version 3.0 as published by the      **
 ** Free Software Foundation and appearing in the file LICENSE.GPL  **
 ** included in the packaging of this file.  Please review the      **
 ** following information to ensure the GNU General Public License  **
 ** version 3.0 requirements will be met:                           **
 ** http://www.gnu.org/copyleft/gpl.html.                           **
 **                                                                 **
 ** The development of this software was supported by the           **
 ** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
 ** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
 ** Forschungsgemeinschaft (DFG) in the context of the German       **
 ** Excellence Initiative.                                          **
 **                                                                 **
 *********************************************************************/
#include <ICLQuick/Quick.h>
#include <ICLQt/QtMacros.h>
#include <ICLQt/Application.h>
#include <ICLQt/GUI.h>
#include <ICLCore/Img.h>
#include <ICLQt/DrawWidget.h>
#include <ICLAlgorithms/SoftPosit.h>
using namespace icl;
GUI gui("hsplit");
Img<icl8u> image(Size(800,600),3);

void run(){
	double imagePtsData[] = { 172.3829, -15.4229,
			174.9147, -183.8248,
			-28.3942, -147.8052,
			243.2142,  105.4463,
			252.6934,  -72.3310,
			25.7430,  -28.9218,
			35.9377,  149.1948};
	DynMatrix<double> imagePts = DynMatrix<double>(2,7,imagePtsData);

	double imageAdjData[] = { 1,     1,     0,     1,     0,     0,     0,
			1,     1,     1,     0,     1,     0,     0,
			0,     1,     1,     0,     0,     1,     0,
			1,     0,     0,     1,     1,     0,     1,
			0,     1,     0,     1,     1,     1,     0,
			0,     0,     1,     0,     1,     1,     1,
			0,     0,     0,     1,     0,     1,     1};
	//DynMatrix<double> imageAdj(7,7);
	DynMatrix<double> imageAdj = DynMatrix<double>(7,7,imageAdjData);
	double worldPtsData[] = { -0.5000,   -0.5000,   -0.5000,
			0.5000,   -0.5000,   -0.5000,
			0.5000,    0.5000,   -0.5000,
			-0.5000,    0.5000,   -0.5000,
			-0.5000,   -0.5000,    0.5000,
			0.5000,   -0.5000,    0.5000,
			0.5000,    0.5000,    0.5000,
			-0.5000,    0.5000,    0.5000};
	DynMatrix<double> worldPts = DynMatrix<double>(3,8,worldPtsData);
	double worldAdjData[] = { 1,     1,     0,     1,     1,     0,     0,     0,
			1,     1,     1,     0,     0,     1,     0,     0,
			0,     1,     1,     1,     0,     0,     1,     0,
			1,     0,     1,    1,     0,     0,     0,     1,
			1,     0,     0,     0,     1,     1,     0,     1,
			0,     1,     0,     0,     1,     1,     1,     0,
			0,     0,     1,     0,     0,     1,     1,     1,
			0,     0,     0,    1,     1,     0,    1,     1};

	//DynMatrix<double> worldAdj(8,8);
	DynMatrix<double> worldAdj = DynMatrix<double>(8,8,worldAdjData);
	double beta0 = 2.0e-04;
	double noiseStd = 0;
	double initRotData[] = { 0.9149,    0.1910,   -0.3558,
			-0.2254,    0.9726,   -0.0577,
			0.3350,    0.1330,    0.9328};
	DynMatrix<double> initRot(3,3,initRotData);
	double initTransData[] = {0, 0, 50};
	DynMatrix<double> initTrans(1,3,initTransData);
	double focalLength = 1500;
	double centerData[] = {0, 0};
	DynMatrix<double> center(2,1,centerData);

	SoftPosit softposit;
	softposit.init();
	gui_DrawHandle(draw_object);
	softposit.softPosit(imagePts, imageAdj, worldPts, worldAdj, beta0,
			noiseStd, initRot, initTrans, focalLength, **draw_object, center);
}

void init(){
	gui << (GUI("vbox")
		<< "draw[@handle=draw_object@minsize=40x30@label=plot]");

	gui.show();
	gui_DrawHandle(draw_object);
	draw_object = image;
}

int main(int n, char *args[]){
	return ICLApp(n,args,"",init,run).exec();
}
