// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/math/SimplexOptimizer.h>
#include <icl/geom/Geom.h>
#include <icl/geom/CoordinateFrameSceneObject.h>
#include <icl/utils/Random.h>

GUI gui;
Scene scene;

typedef FixedColVector<float,3> Pos;
Pos initPos(1000,1000,1000);
float error_function(const Pos &p){
  return ::sqrt(sqr(p[0]+3) + sqr(p[1]-5) + sqr(p[2]-4));
}

void init(){
  gui << Canvas3D().minSize(20,20).handle("draw") << Show();
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
