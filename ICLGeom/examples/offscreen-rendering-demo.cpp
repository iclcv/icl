/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/offscreen-rendering-demo.cpp          **
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
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLCore/Random.h>

// global data
GUI gui("hbox");
Scene scene;

struct OSRCube : public SceneObject{
  Mutex mutex;
  Img8u image;

  virtual void lock(){ mutex.lock(); }
  virtual void unlock(){ mutex.unlock(); }
  
  OSRCube():SceneObject("cube",Vec(0,0,0,3).data()){
    image = Img8u(Size(300,300),formatRGB);
    std::fill(image.begin(0),image.end(0),255);
    addTexture(0,1,2,3,image);
    addTexture(7,6,5,4,image);
    addTexture(0,3,7,4,image);
    addTexture(5,6,2,1,image);
    addTexture(4,5,1,0,image);
    addTexture(3,2,6,7,image);
    
    setVisible(Primitive::texture,true);
    setVisible(Primitive::quad,false);
  }

  int updateImage(){
    Time t = Time::now();
    const Img8u &screen = scene.render(0);
    gui["image"] = screen;


    Mutex::Locker lock(mutex);
    screen.deepCopy(&image);
    image.setROI(Rect(10,10,280,280));
    URand r(-50,50);
    static icl64f c[] = {128,128,128};
    for(int i=0;i<3;++i){
      c[i] += r;
      if(c[i]<0) c[i]=0;
      if(c[i]>255) c[i]=255;
    }

    image.fillBorder(std::vector<icl64f>(c,c+3));
    return (Time::now()-t).toMilliSeconds();
  }

} cube;

void init(){
  // create graphical user interface
  gui << "draw3D(300x300)[@label=3D scene@minsize=16x16@handle=draw]" 
      << "draw[@minsize=16x16@handle=image@label=offscreen rendered image]"
      << "!show";
  
  // create camera and add to scene instance
  Camera cam(Vec(0,0,-13), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  cam.setResolution(Size(300,300));
  scene.addCamera(cam);

  scene.addObject(&cube);
  scene.setDrawCamerasEnabled(false);

  // link the scene's first camera with mouse gestures in the draw-GUI-component
  gui["draw"].install(scene.getMouseHandler(0));
}


void run(){
  DrawHandle3D draw = gui["draw"];
  DrawHandle image = gui["image"];

  int ms = cube.updateImage();
  image->lock();
  image->reset();
  image->color(255,255,255,255);
  image->text("offscreen rendering time: " + str(ms)+" ms",10,10,9);
  image->unlock();
  image.update();

  draw->lock();    // lock the internal drawing queue
  draw->reset3D(); // remove former drawing commands
  draw->callback(scene.getGLCallback(0)); // render the whole scene
  draw->unlock();  // unlock the internal drawin queue
  draw.update();   // post an update-event (don't use draw->update() !!)
}


int main(int n, char**ppc){
  /// create a whole application 
  return ICLApplication(n,ppc,"",init,run).exec();
}


