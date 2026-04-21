// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#define GLEW_MX
#include <string>
//#include <libfreenect2/opengl.h>///
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#undef GLEW_MX

//#include <Kinect2Grabber.h>//

#include <icl/qt/Common.h>
#include <icl/geom/Geom.h>
#include <icl/core/CCFunctions.h>

#include <icl/geom/PointCloudCreator.h>
#include <icl/geom/PointCloudObject.h>



HSplit gui;
libfreenect2::Freenect2 *ctx = 0;
libfreenect2::Freenect2Device *dev = 0;
libfreenect2::SyncMultiFrameListener *listener = 0;
libfreenect2::FrameMap *frames = 0;


void init(){
  glfwInit();

  ctx = new libfreenect2::Freenect2;
  listener = new libfreenect2::SyncMultiFrameListener(libfreenect2::Frame::Color
                                                      | libfreenect2::Frame::Ir
                                                      | libfreenect2::Frame::Depth);
  frames = new libfreenect2::FrameMap;

  dev = ctx->openDefaultDevice();

  if(!dev){
    throw ICLException("no device connected or failure opening the default one!");
  }

  dev->setColorFrameListener(listener);
  dev->setIrAndDepthFrameListener(listener);
  dev->start();

  std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
  std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;

  gui << ( VBox()
           << Display().handle("hdepth").minSize(10,8)
           << Display().handle("hcolor").minSize(10,8)
           << Display().handle("hir").minSize(10,8)
         )
      << ( HSplit()
           << Canvas3D().handle("draw3D").minSize(40,30)
           )
      << Show();
}


void run(){

  listener->waitForNewFrame(*frames);
  libfreenect2::Frame *rgb = (*frames)[libfreenect2::Frame::Color];
  libfreenect2::Frame *ir = (*frames)[libfreenect2::Frame::Ir];
  libfreenect2::Frame *depth = (*frames)[libfreenect2::Frame::Depth];


  static Img8u colorImage(Size(rgb->width,rgb->height),3);

  if(depth){
    Img32f depthImage(Size(depth->width,depth->height),formatMatrix,
                      std::vector<float*>(1, (float*)depth->data));
    gui["hdepth"] = &depthImage;
  }else{
    throw ICLException("error detected in libfreenect2.so: please ensure to deactivate"
                       " visualization in ....h by setting debug_on to false");
  }

  Img32f irImage(Size(ir->width,ir->height),formatMatrix,
                 std::vector<float*>(1,(float*)ir->data));

  interleavedToPlanar(rgb->data, &colorImage);
  colorImage.swapChannels(0,2);

  //gui["hdepth"] = &depthImage;
  gui["hcolor"] = &colorImage;
  gui["hir"] = &irImage;

  listener->release(*frames);
}


int main(int n, char **ppc){
  int result =  ICLApp(n,ppc,"",init,run).exec();
  dev->stop();
  dev->close();
  delete dev;
  delete frames;
  delete listener;
  delete ctx;

  return result;
}
