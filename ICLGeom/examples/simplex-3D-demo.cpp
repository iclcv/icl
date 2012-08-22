/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/simplex-3D-demo.cpp                   **
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

#include <ICLQuick/Common.h>
#include <ICLUtils/SimplexOptimizer.h>
#include <ICLGeom/Geom.h>
#include <ICLGeom/CoordinateFrameSceneObject.h>
#include <ICLUtils/Random.h>

GUI gui;
Scene scene;

typedef FixedColVector<float,3> Pos;
Pos initPos(1000,1000,1000);
float error_function(const Pos &p){
  return ::sqrt(sqr(p[0]+3) + sqr(p[1]-5) + sqr(p[2]-4));
}

void init(){
  gui << Draw3D().minSize(20,20).handle("draw") << Show();
  Camera cam;
  cam.setPosition(Vec(-611.637,-332.427,-814.748,1));
  cam.setNorm(Vec(0.331055,0.486567,0.808489,1));
  cam.setUp(Vec(0.893045,0.175791,-0.414207,1));
  scene.addCamera(cam);
  SceneObject *o = new CoordinateFrameSceneObject;
  o->translate(-3,5,4);
  scene.addObject(o);

  o = new CoordinateFrameSceneObject;
  o->translate(initPos[0],initPos[1],initPos[2]);
  scene.addObject(o);
  
  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
}

static std::vector<Pos> createRandomSimplex(const Pos &p){
  std::vector<Pos> simplex(4,p);
  for(int i=0;i<4;++i){
    for(int d=0;d<3;++d){
      simplex[i][d] += gaussRandom(0,15);
    }
  }
  return simplex;
}

void run(){
  static SimplexOptimizer<float,Pos> opt(error_function,3,1);
  //  static std::vector<Pos> curr = SimplexOptimizer<float,Pos>::createDefaultSimplex(initPos);
  static std::vector<Pos> curr = createRandomSimplex(initPos);
  static float err = 10000;
  if(err > 0){
    scene.lock();
    SceneObject *o = new SceneObject;
    o->addVertex(curr[0].resize<1,4>(1));
    o->addVertex(curr[1].resize<1,4>(1));
    o->addVertex(curr[2].resize<1,4>(1));
    o->addVertex(curr[3].resize<1,4>(1));
    
    static int step = 0; ++step;
    GeomColor col((!(step%3))*255, (!((step+1)%3))*255, (!((step+2)%3))*255,120);
    o->addTriangle(0,1,2,col);
    o->addTriangle(0,1,3,col);
    o->addTriangle(1,2,3,col);
    o->addTriangle(0,2,3,col);
    o->setVisible(Primitive::triangle,true);
    o->setVisible(Primitive::line,false);
    o->setVisible(Primitive::vertex,false);
    scene.addObject(o);
    scene.unlock();
    
    gui["draw"].render();
    
    SimplexOptimizer<float,Pos>::Result r = opt.optimize(curr);

    curr = r.vertices;
    err = r.fx;
  }
  
  Thread::msleep(100);  
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
};
