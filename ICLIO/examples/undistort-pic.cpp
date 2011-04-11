/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLIO/examples/undistort-picture.cpp                   **
 ** Module : ICLIO                                                  **
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
#include <ICLCore/Channel.h>
#include <ICLQt/GUI.h>
#include <ICLQt/Application.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLCore/Types.h>
#include <ICLCore/Img.h>

/**
 * Simple example how to undistort a picture given xml-file with undistortion parameters.
 */
using namespace icl;

GUI gui("hsplit");
GenericGrabber grabber;

Img32f warpMap;
ImageUndistortion *udist = 0;
void init(){

	gui << (GUI("hbox")
			<< "draw[@handle=calib_object@minsize=20x20@label=calib]");
	if(pa("-wm")){
		gui << ("draw[@handle=draw_object@minsize=20x20@label=plot]");
	}
	gui.show();
	static std::string params[] = {"*.jpg"};
	std::string dev = "file";
	grabber.init(dev,dev+"="+params[0]);


	gui.show();
	std::string fn = pa("-d");

	udist = new ImageUndistortion(fn);
	const Size &size  = pa("-s");
	warpMap.setSize(size);
	warpMap.setChannels(2);
        //this is only for showing the warpmap
	if(pa("-wm")){
		Channel32f cs[2];
		warpMap.extractChannels(cs);
		for(float xi=0;xi<size.width;++xi){
			for(float yi=0;yi<size.height; ++yi){
				Point32f point(xi,yi);
				Point32f p = udist->undistort(point,ImageUndistortion::MatlabModel5Params);
				cs[0](xi,yi) = p.x;
				cs[1](xi,yi) = p.y;
			}
		}
	}
	if(pa("-wm")){
		gui["draw_object"] = &warpMap;
		gui["draw_object"].update();
	}
	grabber.enableUndistortion(*udist,size,interpolateLIN);
}

void run(){
	const ImgBase *img = grabber.grab();
	gui["calib_object"] = img;
	gui["calib_object"].update();

}

/**
  * Useage: icl-undistort-pic -d camconfig.xml -s 640x480 -wm
  * or icl-undistort-pic -d camconfig.xml -s 640x480
  */
int main(int argc, char** argv){
	paex("-d","given xml file with distortion and intrinsic parameters computed with")
					("-s","size for warpmap")
					("-wm","show warpmap");

	ICLApp app = ICLApp(argc,argv,"-d(fn) -s(warpMapSize) -wm()",init,run);
	return app.exec();
}
