// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <ICLQt/Common.h>

//
// Simple example how to undistort a picture given xml-file with undistortion parameters.
// Please note, that icl-undistortion-demo -input dc 0 -udist params.xml is equivalent to
// icl-undistortion-demo -input dc 0@udist=params.xml. This feature is available for
// all ICL applications that use the GenericGrabber in combination with a program-argument
//

HSplit gui;
GenericGrabber grabber;
void init(){


  gui << Display().handle("image").minSize(20,20).label("undistorted image")
      << (pa("-wm") ? Display().handle("warp").minSize(20,20).label("warp map") : Dummy())
      << Show();

  grabber.init(pa("-i"));

  grabber.enableUndistortion(pa("-d"));

  //this is only for showing the warpmap
  if(pa("-wm")){
    gui["warp"] = grabber.getUndistortionWarpMap();
  }
}

void run(){
  gui["image"] = grabber.grabImage();

}
int main(int argc, char** argv){
  pa_explain("-d","given xml file with distortion and "
             "intrinsic parameters computed with")
            ("-wm","show warpmap as well")
            ("-i","input devide and parameters");

  std::cout << "Please note, that \"icl-undistortion-demo -input dc 0 -udist params.xml\" is equivalent to"
            <<" \"icl-undistortion-demo -input dc 0@udist=params.xml\". This feature is available for"
            <<" all ICL applications that use the GenericGrabber in combination with a program-argument "
            << std::endl;

  return ICLApp(argc,argv,"-input|-i(2) -udist|-d(fn) -wm()",init,run).exec();
}
