/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLIO/examples/undistortion-demo.cpp                   **
 ** Module : ICLIO                                                  **
 ** Authors: Christian Groszewski, Christof Elbrechter              **
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

#include <ICLQuick/Common.h>

//
// Simple example how to undistort a picture given xml-file with undistortion parameters.
// Please note, that icl-undistortion-demo -input dc 0 -udist params.xml is equivalent to
// icl-undistortion-demo -input dc 0@udist=params.xml. This feature is available for
// all ICL applications that use the GenericGrabber in combination with a program-argument 
//

GUI gui("hsplit");
GenericGrabber grabber;
void init(){
  
  gui << (GUI("hbox")
          << "draw[@handle=image@minsize=20x20@label=undistorted image]");
  if(pa("-wm")){
    gui << ("draw[@handle=warp@minsize=20x20@label=warp map]");
  }
  gui.show();
  grabber.init(pa("-i"));

  grabber.enableUndistortion(pa("-d"));

  //this is only for showing the warpmap
  if(pa("-wm")){
    gui["warp"] = grabber.getUndistortionWarpMap();
    gui["warp"].update();
  }
}

void run(){
  const ImgBase *img = grabber.grab();
  gui["image"] = img;
  gui["image"].update();
  
}
int main(int argc, char** argv){
  paex("-d","given xml file with distortion and "
       "intrinsic parameters computed with")
  ("-wm","show warpmap as well")
  ("-i","input devide and parameters");

  std::cout << "Please note, that \"icl-undistortion-demo -input dc 0 -udist params.xml\" is equivalent to"
            <<" \"icl-undistortion-demo -input dc 0@udist=params.xml\". This feature is available for"
            <<" all ICL applications that use the GenericGrabber in combination with a program-argument " 
            << std::endl;
  
  return ICLApp(argc,argv,"-input|-i(2) -udist|-d(fn) -wm()",init,run).exec();
}
