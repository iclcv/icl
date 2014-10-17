/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/[...]/surf-based-object-tracking/.cpp     **
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

#include <ICLCV/SurfFeatureDetector.h>
#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLGeom/RansacBasedPoseEstimator.h>

GenericGrabber grabber;
SmartPtr<SurfFeatureDetector> surf;
RansacBasedPoseEstimator *pe = 0;

Size32f ts; // template pixel -> mm
VBox gui;
GUI ransacOptions;
Scene scene;
SceneObject *obj = 0;

void init(){
  grabber.init(pa("-i"));

  Size32f t(pa("-t",1).as<float>(), pa("-t",2).as<float>());
  float d = pa("-t",3);
  scene.addCamera(*pa("-cam"));
  pe = new RansacBasedPoseEstimator(scene.getCamera(0));

  obj = SceneObject::cuboid(t.width/2,t.height/2,d/2,t.width,t.height,d);
  obj->setColor(Primitive::quad, GeomColor(0,100,255,50));
  obj->setColor(Primitive::line, geom_red());
  obj->setLineWidth(3);
  scene.addObject(obj);

  surf = new SurfFeatureDetector(5,4,2,0.00005,"opensurf");
  Img8u templ = load<icl8u>(pa("-t"));
  surf->setReferenceImage(&templ);

  ts = Size32f(t.width/templ.getWidth(), t.height/templ.getHeight());

  gui << Draw3D().handle("draw").minSize(32,24)
      << (HBox() 
          << FSlider(-7,-1,-3).handle("t").maxSize(99,3).label("threshold exponent")
          << Button("ransac ...").handle("ransac options").maxSize(6,3)
          << CheckBox("vis error").handle("vise").maxSize(5,3)
          )
      << Show();
  
  pe->setConfigurableID("pe");
  pe->adaptProperty("iterations","range","[1,5000]:1","");
  pe->adaptProperty("max error","range","[1,100]","");
  pe->adaptProperty("min points","range","[4,10]:1","");
  pe->adaptProperty("min points for good model","range","[4,100]:1","");


  pe->setPropertyValue("iterations",200);
  pe->setPropertyValue("max error",30);
  pe->setPropertyValue("min points",4);
  pe->setPropertyValue("min points for good model",20);
  pe->setPropertyValue("store last consensus set",true);

  ransacOptions << Prop("pe") << Create();
  
  gui["ransac options"].registerCallback(function(&ransacOptions, &GUI::switchVisibility));

  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
}



void run(){
  DrawHandle3D draw = gui["draw"];
  const ImgBase *image = grabber.grab();
  draw = image;
  
  float tExp = gui["t"];
  float t = ::pow(10,tExp);

  surf->setThreshold(t);
  const std::vector<SurfMatch> &ms = surf->match(image);
  if(ms.size() >= 4){
    std::vector<Point32f> curr(ms.size()),templ(ms.size());
    for(size_t i=0;i<ms.size();++i){
      draw->draw(ms[i].first.vis());
      curr[i] = Point32f(ms[i].first.x,ms[i].first.y);
      templ[i] = Point32f(ms[i].second.x,ms[i].second.y).transform(ts.width,ts.height);
    }
    RansacBasedPoseEstimator::Result result = pe->fit(templ,curr);
   

    Mat T = result.T; 
    obj->setTransformation(T);

    if(gui["vise"]){

      draw->linewidth(2);
      
      std::vector<Point32f> lastSet = pe->getLastConsensusSet();
      
      for(size_t i=0;i<ms.size();++i){
        Vec a = T * Vec(templ[i].x, templ[i].y,0,1);
        Point32f pa = scene.getCamera(0).project(a);
        if(std::find(lastSet.begin(),lastSet.end(), curr[i]) != lastSet.end()){
          draw->color(255,100,0,255);
        }else{
          draw->color(255,100,0,100);
        }
        draw->line(pa,curr[i]);
      }
    }
  }

  draw.render();
}


int main(int n, char **args){
  pa_explain("-p","select surf-feature detection plugin (opensurf, clsurf or best)");
  return ICLApp(n,args,"[m]-i(2) -t(filename,obj-width-mm,obj-height-mm,obj-thickness-mm) "
                "-cam(file) -surf-plugin|-p(plugin=opensurf)",init,run).exec();
}
