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
#include <ICLMarkers/FiducialDetectorPluginForQuads.h>

HSplit gui;

GenericGrabber grabber;
FiducialDetector *fid = 0;
Scene scene;
bool canShowRegionCorners = false;

struct Obj : public SceneObject{
  Obj(){
    addChild(new ComplexCoordinateFrameSceneObject);
    Size s  = parse<Size>(pa("-m",2));
    addCuboid(0,0,10,s.width,s.height,20);
  }
} *obj = 0;


void init(){
  fid = new FiducialDetector(pa("-m").as<std::string>(), 
                             pa("-m",1).as<std::string>(), 
                             ParamList("size",(*pa("-m",2)) ) );
  
  fid->setConfigurableID("fid");
  canShowRegionCorners = (*pa("-m") == "bch" || *pa("-m") == "art");

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
          << (HBox().label("show")
              << ( VBox()
                   << CheckBox("IDs",true).out("showIDs")
                   << CheckBox("markers",true).out("showMarkerCorners")
                   )
              << (VBox()
                  << CheckBox("regions",false).out("showRegionCorners").hideIf(!canShowRegionCorners)
                  << CheckBox("angles",true).out("showAngles")
                  )
              )
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
  const bool showRegionCorners = canShowRegionCorners && gui["showRegionCorners"].as<bool>();
  const bool showMarkerCorners = gui["showMarkerCorners"].as<bool>();
  const bool showAngles = gui["showAngles"];

  static bool use3Dfor1stVisibleMarker = pa("-3D-for-first-visible-marker");
  for(unsigned int i=0;i<fids.size();++i){
    
    if((enable3D && fids[i].getID() == 0)
       || (enable3D && use3Dfor1stVisibleMarker && !i)){
      fids[i].getPose3D();
      fids[i].getPose3D();
      obj->setTransformation(fids[i].getPose3D());
    }
    
    if(showMarkerCorners){
      draw->color(255,0,0,255);
      draw->linestrip(fids[i].getCorners2D());
    }

    if(showIDs){
    draw->color(0,100,255,255);
      draw->text(fids[i].getName(),fids[i].getCenter2D().x, fids[i].getCenter2D().y,9);
    }
    if(showRegionCorners){
      static QuadDetector &qd = ((FiducialDetectorPluginForQuads*)fid->getPlugin())->getQuadDetector();
      draw->color(0,100,255,255);
      std::vector<Point32f> cs = qd.computeCorners(fids[i].getImageRegion());
      draw->linestrip(cs);
      for(size_t i=0;i<cs.size();++i){
        draw->sym(cs[i],'x');
      }
    }
    if(showAngles){
      draw->color(0,255,0,255);
      float a = fids[i].getRotation2D();
      draw->line(fids[i].getCenter2D(), fids[i].getCenter2D() + Point32f( cos(a), sin(a))*100 );
    }
  }

  draw.render();
  gui["fps"].render();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2) -camera|-c(camerafile) -size|-s(size=VGA) "
                "-marker-type|-m(type=bch,whichToLoad=[0-4095],size=30x30) -3D "
                "-3D-for-first-visible-marker",init,run).exec();
}


