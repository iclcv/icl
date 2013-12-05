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
#include <ICLGeom/Geom.h>

GUI gui;
Scene scene;

RayCastOctreeObject octree(-100,200);

struct Mouse : public MouseHandler{
  void process(const MouseEvent &e){
    if(e.isPressEvent() && e.isModifierActive(ShiftModifier)){
      const Camera &c = scene.getCamera(0);
      ViewRay v = c.getViewRay(e.getPos());
      SceneObject *o = new SceneObject;
      static const float r = pa("-r");
      
      if(pa("-1")){
        try{
          Vec vv = octree.rayCastClosest(v,r);
          o->getVertices().push_back(vv);
        }catch(ICLException &e){
          std::cout << "no point close enough to ray!" << std::endl;
        }
      }else{
        o->getVertices() = octree.rayCast(v,r);//0.01);
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
  scene.addCamera(Camera(Vec(0,0,150,1)));
  scene.setBounds(200);
 
  octree.setRenderBoxes(false);
  octree.setRenderPoints(true);

  gui << Draw3D().handle("plot") << Show();

  for(float x=0;x<320;++x){
    for(float y=0;y<240;++y){
      octree.insert( Vec((x-160)/2, (y-120)/2, 0,1 ) );
    }
  }
    octree.setPointSize(2);
  
  
  scene.addObject(&octree,false);
  
  gui["plot"].link(scene.getGLCallback(0));
  gui["plot"].install(scene.getMouseHandler(0));
  gui["plot"].install(&mouse);
  
  
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-r(float=1) -1",init).exec();
}
