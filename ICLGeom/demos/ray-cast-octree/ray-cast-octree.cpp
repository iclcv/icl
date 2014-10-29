/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/ray-cast-octree/ray-cast-octree.cpp      **
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
#include <ICLGeom/RayCastOctreeObject.h>
#include <ICLGeom/GenericPointCloudGrabber.h>
#include <ICLGeom/Geom.h>

GUI gui;
Scene scene;

RayCastOctreeObject octree(-2000,4000);
GenericPointCloudGrabber grabber;

struct Mouse : public MouseHandler{
  void process(const MouseEvent &e){
    if(e.isPressEvent() && e.isModifierActive(ShiftModifier)){
      const Camera &c = scene.getCamera(0);
      ViewRay v = c.getViewRay(e.getPos());
      SceneObject *o = new SceneObject;
      static const float r = pa("-r");
      
      if(pa("-1")){
        try{
          octree.lock();
          Vec vv = octree.rayCastClosest(v,r);
          octree.unlock();
          o->getVertices().push_back(vv);
        }catch(ICLException &e){
          std::cout << "no point close enough to ray!" << std::endl;
        }
      }else{
        octree.lock();
        o->getVertices() = octree.rayCast(v,r);//0.01);
        octree.unlock();
      }
      o->setPointSize(5);
      scene.addObject(o);

      SceneObject *l = new SceneObject;
      l->addVertex(v.offset);
      l->addVertex(v.offset + v.direction * 400);
      l->addLine(0,1,geom_green());
      scene.addObject(l);
    }
  }  
} mouse;




void init(){
  if(pa("-c")){
    scene.addCamera(Camera(*pa("-c")));
  }else{
    scene.addCamera(Camera(Vec(0,0,1500,1)));
  }
  scene.addCamera(scene.getCamera(0));
  
  scene.setBounds(2000);
 
  octree.setRenderBoxes(true);
  octree.setRenderPoints(true);
  octree.setLockingEnabled(true);

  gui << Draw3D().handle("plot") << Show();


  if(pa("-pci")){
    grabber.init(pa("-pci"));
  }else{
    for(float x=0;x<6400;x+=10){
      for(float y=0;y<4800;y+=10){
        octree.insert( Vec((x-3200)/2, (y-2400)/2, 0,1 ) );
      }
    }
  }
  octree.setPointSize(2);
  
  scene.addObject(&octree,false);
  
  gui["plot"].link(scene.getGLCallback(0));
  gui["plot"].install(scene.getMouseHandler(0));
  gui["plot"].install(&mouse);
}

void run(){
  if(pa("-pci")){
    static PointCloudObject obj;
    grabber.grab(obj);
    Time t = Time::now();
    octree.fill(obj,OctreeObject::PointFilter(scene.getCamera(1)),true);
    //octree.fill(obj);
    t.showAge("time for filling the octree");

  }else{
    Thread::msleep(100000);
  }

  gui["plot"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-r(float=1) -1 -pci(2) -c(initial-camera)",init,run).exec();
}
