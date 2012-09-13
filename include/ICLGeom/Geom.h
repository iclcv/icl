/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/Geom.h                                 **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#pragma once

#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>

/** 
    The ICLGeom package provides functions and classes for handling 3D-geometry related problems. 
    This includes
    - camera calibration
    - 3D/6D pose estimation
    - 3D visualization
    - utils::Point Cloud Processing and visualization

    \section _CALIB_ Camera Calibration 

    ICL provides different tools for camera calibration. Basically, we distinguish between 
    -# Estimation of distortion parameters (which is not part of this package)
       Instead, the ICLGeom package assumes images to be undistorted.
    -# Camera calibration (i.e. finding a camera's intrinsic and extrinsic parameters)
       ICL provides are very easy to use camera calibration tool called icl-cam-calib-2, which
       uses a predifined 3D calibration object. The calibration object is equipped with Fiducial Markers
       that can be detected with ICL's built-in marker detection framework.
    
    \subsection _CALIB_HOW_TO_ How to Use ICL's Camera Calibration Toolbox
    
    Here, a description of how to use ICL's camera calibration toolbox and how to integrate the
    resulting camera parameters to your application.
    
    \section _POSE_EST_ Pose Estimation
    
    ICL Provides different standard algorithms for 3D pose/position estimation.
    -# Triangolization (icl::Camera::estimate_3D)
    -# Posit (icl::Posit)
    -# SoftPosit (icl::SoftPosit)
    -# Pose estimation for coplanar points (icl::CoplanarPointPoseEstimator)
    -# ICP (Iterative Closest utils::Point) (icl:ICP)
    
    
    \section _3D_VIS_ 3D Visualization
    
    3D Visualization is also a very important part for interactive applications. ICL provides a simple to use
    scene (icl::Scene) class that allows for defining 3D objects and to render these objects as an image overlay.
    The icl::Scene class is fully integrated with ICL's Qt package:
    - The scene rendering can easily be added to an icl::ICLDrawWidget3D instance using the ICLDrawWidget3D::link() 
      method
    - Objects are rendered very efficiently using OpenGL
    - a Scene does also provide MouseHandlers, that can be installed to ICLDrawWidget's for 3D scene navigation.
    
    \subsection _EX_VIS_ Simple Visualization Example

    <table border=0><tr><td>
    \code
#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>

icl::qt::GUI gui;
Scene scene;

void init(){
  // create graphical user interface
  gui << Draw3D().handle("draw") << Show();

  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));  // up-direction
  scene.addCamera(cam);

  // add an object to the scene
  scene.addObject(SceneObject::cuboid(0,0,0,7,3,2));

  // use mouse events for camera movement
  gui["draw"].install(scene.getMouseHandler(0));

  // link the visualization
  gui["draw"].link(scene.getGLCallback(0));
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
    \endcode
    </td><td valign="top">
    \image html simple-vis.png "simple visualization demo"
    </td></tr></table>
*/




