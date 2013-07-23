/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/marker-detection/marker-detection.cpp  **
** Module : ICLMarkers                                             **
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
#include <ICLGeom/Scene.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

#include <ICLMarkers/FiducialDetector.h>

HSplit gui;

GenericGrabber grabber;
FiducialDetector *fid = 0;
Scene scene;

struct Obj : public SceneObject{
  Obj(){
    addChild(new ComplexCoordinateFrameSceneObject);
    Size s  = parse<Size>(pa("-m",2));
    addCuboid(0,0,10,s.width,s.height,20);
  }
} *obj = 0;

#if 0
void bench();
#endif
void init(){

#if 0
  if(pa("-bench")){
    bench();
    ::exit(0);
  }
#endif

  fid = new FiducialDetector(pa("-m").as<std::string>(), 
                             pa("-m",1).as<std::string>(), 
                             ParamList("size",(*pa("-m",2)) ) );
  
  fid->setConfigurableID("fid");

  grabber.init(pa("-input"));
  if(pa("-size")) grabber.useDesired<Size>(pa("-size"));
  grabber.useDesired(formatGray);

  gui << Draw3D(pa("-size").as<Size>()).handle("draw").minSize(16,12)
      << (VBox().maxSize(16,100) 
          << Combo(fid->getIntermediateImageNames()).maxSize(100,2).handle("vis").label("visualization")
          << Prop("fid")
          << (HBox() 
              << Fps().handle("fps")
              << Label("no markers found yet").label("detected markers").handle("count")
              )
          << CheckBox("show IDs",true).out("showIDs")
          << Label("-- ms").handle("ms").label("detection time")
          << (HBox() 
              << Button("running","pause").out("pause")
              << CamCfg("")
              )
         )
      << Show();
  
  //fid->loadMarkers("[0,10]",ParamList("size",Size(96,96)));
  try{
    fid->setPropertyValue("quads.minimum region size",400);
    fid->setPropertyValue("thresh.global threshold",-11);
    fid->setPropertyValue("thresh.mask size",45);
    fid->setPropertyValue("thresh.algorithm","tiled linear");
  }catch(ICLException &e){
    WARNING_LOG("exception caught while setting initial parameters: " << e.what());
  }
  


  if(pa("-c")){
    scene.addCamera(Camera(*pa("-c")));
  }else{
    scene.addCamera(Camera());
    scene.getCamera(0).setResolution(pa("-size"));
  }

  if(pa("-3D").as<bool>() || pa("-c").as<bool>()){
    obj = new Obj;
    scene.addObject(obj);
    fid->setCamera(scene.getCamera(0));
    gui["draw"].install(scene.getMouseHandler(0));
    gui["draw"].link(scene.getGLCallback(0));
  }

}

inline float round2(float f){
  return float((int)(f*100))*0.01;
}

// working loop
void run(){
  static bool enable3D = pa("-3D").as<bool>() || pa("-c").as<bool>();
  while(gui["pause"]) Thread::msleep(100);
  const ImgBase *image = grabber.grab();
 
  Time t = Time::now();
  const std::vector<Fiducial> &fids = fid->detect(image);
  gui["ms"] = round2(t.age().toMilliSecondsDouble());

  DrawHandle3D draw = gui["draw"];
  draw = fid->getIntermediateImage(gui["vis"]);

  draw->linewidth(2);
  
  gui["count"] = fids.size();

  const bool showIDs = gui["showIDs"];
  
  for(unsigned int i=0;i<fids.size();++i){
    
    if(enable3D && fids[i].getID() == 0){
      fids[i].getPose3D();
      fids[i].getPose3D();
      obj->setTransformation(fids[i].getPose3D());
    }
    
    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());

    if(showIDs){
    draw->color(0,100,255,255);
      draw->text(fids[i].getName(),fids[i].getCenter2D().x, fids[i].getCenter2D().y,9);
    }
    draw->color(0,255,0,255);
    float a = fids[i].getRotation2D();
    draw->line(fids[i].getCenter2D(), fids[i].getCenter2D() + Point32f( cos(a), sin(a))*100 );
  }

  draw.render();
  gui["fps"].render();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2) -camera|-c(camerafile) -size|-s(size=VGA) "
                "-marker-type|-m(type=bch,whichToLoad=[0-4095],size=30x30) -3D",init,run).exec();
}


#if 0

/// marker detection benchmark
void bench(){
  FiducialDetector d("art");
  d.setPropertyValue("matching algorithm", "gray sqrdist");
  
  ImgQ rows;
  int i=0;

  int s = 2;
  for(int y=0;y<20/s;++y){
    ImgQ line;
    for(int x=0;x<20/s;++x, ++i){
      line = (line,ones(1,50*s,1)*255,cvt(d.createMarker("bch/bch"+str(i)+".png",Size(50*s,50*s),ParamList("size","50x50","border ratio",0.4))));
      d.loadMarkers("bch/bch"+str(i)+".png",ParamList("size","50x50","border ratio",0.4));
      std::cout << "marker " << i<< "loaded" << std::endl;
    } 
    rows = rows % ones(1000+20/s,1,1)*255 % line;
  }
  rows = (ones(2,rows.getHeight(),1)*255,rows,ones(2,rows.getHeight(),1)*255);
  rows = (ones(rows.getWidth(),2,1)*255 % rows % ones(rows.getWidth(),2,1)*255);
  
  Img8u image = cvt8u(rows);

  //image.setROI(Rect(200,200,620,620)); //144 / 36 markers

  image.setROI(Rect(300,300,420,420)); // 64 /16 markers

  image.setROI(Rect(400,400,220,220)); // 16 markers

  image.fillBorder(255.0);
  image.setFullROI();
  show(image);
  
  Thread::sleep(4);

  std::vector<int> found;
  found.reserve(1000);
  Time t = Time::now();
  for(int i=0;i<100;++i){
    const std::vector<Fiducial> &ms = d.detect(&image);
    found.push_back(ms.size());
  }
  std::cout << "time for art:" << t.age().toSecondsDouble()*10 <<  "ms (" << found.back() << ")" << std::endl;
   

}

#endif
