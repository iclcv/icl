/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/point-cloud-viewer/point-cloud-viewer.cpp**
** Module : ICLGeom                                                **
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

#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>

#include <ICLGeom/PointCloudObject.h>
//#include <ICLGeom/DepthCameraPointCloudGrabber.h>
#include <ICLGeom/GenericPointCloudGrabber.h>
#include <ICLGeom/RayCastOctreeObject.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

HSplit gui;
Scene scene;


PointCloudObject obj, obj2;
GenericPointCloudGrabber grabber;
GenericPointCloudGrabber grabber2; // used to add an extra point cloud if needed
SmartPtr<RayCastOctreeObject> octree;
Mutex octreeMutex;

SceneObject *indicatorCS = 0;
Vec pos;

void mouse(const MouseEvent &e){
  static MouseHandler *other = scene.getMouseHandler(0);
  other->process(e);
  try{
    Vec v;
    {
      Mutex::Locker lock(octreeMutex);
      if(!octree) return;
      v = octree->rayCastClosest(scene.getCamera(0).getViewRay(e.getPos()), 10);
      pos = v;
    }
    
    indicatorCS->lock();
    indicatorCS->removeTransformation();
    indicatorCS->translate(v[0],v[1],v[2]);
    indicatorCS->unlock();
  }catch(...){}
}

void init(){
  grabber.init(pa("-pci"));
  grabber.setConfigurableID("grabber");
  
  if(pa("-pci2")){
    grabber2.init(pa("-pci2"));
    grabber2.setConfigurableID("grabber2");
  }
  Camera cam;
  if(pa("-c")){
    cam = Camera(*pa("-c"));
  }

  gui << Draw3D().minSize(32,24).handle("scene")
      << Prop("grabber").hideIf(!pa("-tune")).minSize(16,1).maxSize(16,99)
      << Prop("grabber").hideIf(!pa("-tune") || !pa("-pci2")).minSize(16,1).maxSize(16,99)
      << ( HBox().label("show objects")
           << CheckBox("object 1",true).handle("o1").hideIf(!pa("-pci2"))
           << CheckBox("object 2",true).handle("o2").hideIf(!pa("-pci2"))
         )
      << Show();
  

  // kinect camera
  scene.addCamera(cam);
  scene.setBounds(1000);
  scene.addObject(&obj,false);

  gui["scene"].link(scene.getGLCallback(0));
  gui["scene"].install(mouse);

  obj.setPointSize(3);
  obj.setPointSmoothingEnabled(false);
  

  if(pa("-vc")){
    ProgArg p = pa("-vc");
    for(int i=0;i<p.n();++i){
      Camera c(p[i]);
      c.setName(p[i]);
      scene.addCamera(c);
    }
    scene.setDrawCamerasEnabled(true);
  }
  
  if(pa("-cs")){
    scene.setDrawCoordinateFrameEnabled(true);
  }

  if(pa("-p")){
    indicatorCS = new ComplexCoordinateFrameSceneObject(50,2);
    //indicatorCS->addVertex(Vec(10,10,50,1));
    //indicatorCS->addText(indicatorCS->getVertices().size()-1,"Test", 50, geom_blue(255));
    scene.addObject(indicatorCS,true);
  }

  // if this is left out creation of the first octree takes ages!
  grabber.grab(obj);
  if(pa("-pci2")){
    scene.addObject(&obj2,false);
    obj2.setPointSize(3);
    obj2.setPointSmoothingEnabled(false);
    grabber2.grab(obj2);
  }
  
  if(pa("-add-plane")){
    SceneObject *plane = new SceneObject;
    Vec p(pa("-add-plane",0),
          pa("-add-plane",1),
          pa("-add-plane",2),1);
    Vec n(pa("-add-plane",3),
          pa("-add-plane",4),
          pa("-add-plane",5),1);
    
    Vec nx;
    if(n[1] || n[2]){
      nx = cross(n, n + Vec(1,0,0));
    }else{
      nx = Vec(0,1,0,1);
    }
    Vec ny = cross(n, nx);
    
    nx = normalize3(nx);
    nx[3] = 0;
    ny = normalize3(ny);
    ny[3] = 0;

    plane->addVertex(p + nx * 1500 + ny * 1500);
    plane->addVertex(p + nx * 1500 - ny * 1500);
    plane->addVertex(p - nx * 1500 - ny * 1500);
    plane->addVertex(p - nx * 1500 + ny * 1500);
    
    plane->addQuad(0,1,2,3,GeomColor(255,50,150,100));
    scene.addObject(plane,true);

    scene.getLight(0).setAmbientEnabled(true);
    scene.getLight(0).setOn(true);
    scene.getLight(0).setAmbient(GeomColor(255,255,255,100));
  }
}

void run_octree(){
  if(!pa("-p")){
    Thread::sleep(1000);
    return;
  }
  SmartPtr<PointCloudObject> other = obj.copy();
  RayCastOctreeObject *octree = new RayCastOctreeObject(-3000,6000);
  //  Time t = Time::now();
  const DataSegment<float,4> xyzh = other->selectXYZH();
  for(int i=0;i<xyzh.getDim();i+=1){
    octree->insert(xyzh[i]);
  }
  //t.showAge("time for point insertion ... of " + str(xyzh.getDim()) + " points");
  Mutex::Locker lock(octreeMutex);
  ::octree = SmartPtr<RayCastOctreeObject>(octree);
}

void run(){
  DrawHandle3D draw = gui["scene"];
  grabber.grab(obj);

  if( pa("-pci2")){
    grabber2.grab(obj2);

    obj.setVisible(gui["o1"].as<bool>());
    obj2.setVisible(gui["o2"].as<bool>());
  }
  
  draw->color(0,100,255,255);
  draw->text(str(pos[0]) + "  " + str(pos[1]) + "  " + str(pos[2]) + "  ", 
             10, 30, -10);
  draw->color(255,255,255,255);
  draw->text(str(pos[0]) + "  " + str(pos[1]) + "  " + str(pos[2]) + "  ", 
             12, 32, -10);

  draw.render();
}


int main(int n, char **ppc){
  pa_explain("-tune","adds an extra gui that allows certain depth "
             "camera parameters to be tuned manually at runtime");
  return ICLApp(n,ppc,"[m]-point-cloud-input|-pci(point-cloud-source,descrition) "
                "-point-cloud-input-2|-pci2(point-cloud-source,descrition) "
                "-add-plane(px,py,pz,nx,ny,nz) "
                "-view-camera|-c(filename) -tune -visualize-cameras|-vc(...) "
                "-show-world-frame|-cs -pointing|-p",init,run,run_octree).exec();
}

