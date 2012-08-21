/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/texture-cube-demo.cpp                 **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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
#include <ICLGeom/Geom.h>

GUI gui;
Scene scene;

float cubep[] = {0,0,0,7};
struct TextureCube : public SceneObject{
  TextureCube():SceneObject("cube",cubep){
    Img8u image = cvt8u(icl::scale(create("lena"),300,300));
    
#if DO_NOT_USE_SHARED_TEXTURE
    addTexture(0,1,2,3,&image,0,0,0,0);
    addTexture(7,6,5,4,&image,1,1,1,1);
    addTexture(0,3,7,4,&image,2,2,2,2);
    addTexture(5,6,2,1,&image,3,3,3,3);
    addTexture(4,5,1,0,&image,4,4,4,4);
    addTexture(3,2,6,7,&image,5,5,5,5);
#else
    addSharedTexture(&image);

    addTexture(0,1,2,3,0,0,0,0,0);
    addTexture(7,6,5,4,0,1,1,1,1);
    addTexture(0,3,7,4,0,2,2,2,2);
    addTexture(5,6,2,1,0,3,3,3,3);
    addTexture(4,5,1,0,0,4,4,4,4);
    addTexture(3,2,6,7,0,5,5,5,5);

#endif
    setVisible(Primitive::quad,false);
  }
} cube;

struct Light : public SceneObject{
  std::vector<SceneObject*> trace;
  int idx;
  Light(int idx):
    SceneObject("sphere",FixedColVector<float,6>(0,0,0,0.4,10,10).data()),idx(idx){
    setColor(Primitive::quad,GeomColor(255,255,255,255));
    setVisible(Primitive::line,false);
    setVisible(Primitive::vertex,false);
  }
  void prepareForRendering(){
    removeTransformation();
    translate((idx==0)*8,(idx==1)*8,(idx==2)*8);
    static Time t = Time::now();
    float dt = t.age().toSecondsDouble();
    rotate((idx==2)*dt,(idx==0)*dt,(idx==1)*dt);
  }
} *lights[3] = { new Light(0),new Light(1), new Light(2) };


void init(){
  gui << Draw3D().minSize(16,12).handle("draw").label("scene view") << Show();
  
  scene.addCamera(Camera(Vec(0,-10,-10),
                         Vec(0,0.707,0.707),  
                         Vec(1,0,0)));
  scene.addObject(&cube);

  scene.getLight(0).setOn(false);
  
  for(int i=0;i<3;++i){
    SceneLight &l = scene.getLight(1+i);
    l.setOn();
    l.setDiffuseEnabled(true);
    l.setDiffuse(GeomColor((i==0)*255,(i==1)*255,(i==2)*255,255));
    l.setPosition(Vec(0,0,0,1));
    l.setAnchor(lights[i]);
    scene.addObject(lights[i]);
  }
  
  float bgsphere[] = {0,0,0,40,40,100};
  SceneObject *bg = new SceneObject("sphere",bgsphere);
  bg->setColor(Primitive::quad,GeomColor(0,0,100,255));
  bg->setColor(Primitive::vertex,GeomColor(255,255,255,255));
  bg->setPointSize(3);
  bg->setVisible(Primitive::line,false);
  scene.addObject(bg);
  
  gui["draw"].install(scene.getMouseHandler(0));
  gui["draw"].link(scene.getGLCallback(0));
}


void run(){
  Thread::msleep(20);
  gui["draw"].render();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

