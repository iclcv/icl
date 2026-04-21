// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once
#include <icl/utils/CompatMacros.h>
#include <icl/geom/GeomDefs.h>
#include <icl/geom/Scene.h>
#include <icl/geom/SceneObject.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Plot3D.h>
#include <icl/geom/PlotWidget3D.h>
#include <icl/geom/PointCloudObject.h>
#include <icl/geom/DepthCameraPointCloudGrabber.h>
#include <icl/geom/ComplexCoordinateFrameSceneObject.h>
#ifdef ICL_HAVE_PCL
#include <icl/geom/PCLPointCloudObject.h>
#endif

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
#include <icl/qt/Common.h>
#include <icl/geom/Geom.h>
#include <icl/utils/FPSLimiter.h>

icl::qt::GUI gui;
Scene scene;

void init(){
  // create graphical user interface
  gui << Canvas3D().handle("draw") << Show();

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
