/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLGeom module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_GEOM_H
#define ICL_GEOM_H

#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>



/** \mainpage ICLGeom Package for Handling 3D Geometry

    The ICLGeom package provides functions and classes for handling 3D-geometry related problems. 
    This includes for example 3D camera calibration and 3D position estimation from multiple camera views. 
    ICLGeom's fundamental component is the icl::Camera class. Another important part of the ICLGeom package
    is the lightweight scene API which essentially consists of the icl::Scene2 and the icl::Object2 classes. 
    The icl::Scene2 class is linked to the icl::Camera class which helps to visualize simple 3D scenes and
    to navigate in this scenes. 

    \image html icl-new-geom-3D-demo.png "Demo application icl-new-geom-3D-demo which demonstrates navigation in a scene with different cameras"


    \section EASY_TO_USE_INTERFACE Easy-to-Use Interface
    
    The ICLGeom package provides easy-to-use classes and functionalites for camera representation and 
    visualization of simple 3D scenes. Scenes can easily be connected with other common ICL components, such as
    GUI integration and mouse interaction. The following example demonstrates this.

    <table border=0><tr><td>
    \code
#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
Scene2 scene;

void init(){
  // create graphical user interface
  gui << "draw3D[@minsize=16x12@handle=draw@label=scene view]"
      << "fslider(0.5,20,3)[@out=f@handle=focal"
         "@label=focal length@maxsize=100x3]";
  gui.show();
  
  // create scene background
  gui["draw"] = zeros(640,480,0);

  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0),   // up-direction
             Size::VGA);   // screen-size
  scene.addCamera(cam);

  // add an object to the scene
  float data[] = {0,0,0,7,3,2};
  scene.addObject(new Object2("cuboid",data));

  // use mouse events for camera movement
  gui["draw"].install(scene.getMouseHandler(0));
}


void run(){
  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(25);
  gui_DrawHandle3D(draw);
  scene.getCamera(0).setFocalLength(gui.getValue<float>("f"));

  draw->lock();
  draw->reset3D();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  draw.update();

  limiter.wait();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
    \endcode
    </td><td valign="top">
    \image html geom-demo.png "icl-geom-demo application's screenshot"
    </td></tr></table>


    \section THREED_VIS 3D-Data Visualization
    
    <table border=0><tr><td valign="top">
    The ICLGeom package is optimized for simpel 3D visualization. The ICLDrawWidget3D from the ICLQt package
    can be used for visualizing scenes as well as images. The demo application icl-swiss-ranger-demo allows 
    to visualize a depth-map obtained from a SwissRanger time-of-flight camera (see 
    <a href="http://www.mesa-imaging.ch">Homepage Mesa Imaging</a> for details). Of course there are more sophisticated
    libraries for 3D visualization, however for simple cases, ICLGeom provides all functionalities that are needed.
    The demo application visualizes a 3D-mesh or point-cloud as well as the depth-map image provided by the
    SwissRanger camera. In particular, video-textures can easily be visualized.
    </td><td valign="top">
    \image html swiss-ranger-demo.png "screenshot of application icl-swiss-ranger-demo"
    </td></tr></table>

*/




#endif
