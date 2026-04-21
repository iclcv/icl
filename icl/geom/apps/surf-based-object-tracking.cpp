// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/SurfFeatureDetector.h>
#include <icl/qt/Common2.h>
#include <icl/geom/Geom.h>
#include <icl/geom/Material.h>
#include <icl/geom/RansacBasedPoseEstimator.h>

GenericGrabber grabber;
std::shared_ptr<SurfFeatureDetector> surf;
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
  obj->setMaterial(Material::fromColors(GeomColor(0,100,255,50), geom_red()));
  obj->setLineWidth(3);
  scene.addObject(obj);

  surf.reset(new SurfFeatureDetector(5,4,2,0.00005,"opensurf"));
  Img8u templ = icl::qt::load(pa("-t")).as8u();
  surf->setReferenceImage(&templ);

  ts = Size32f(t.width/templ.getWidth(), t.height/templ.getHeight());

  gui << Canvas3D().handle("draw").minSize(32,24)
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

  gui["ransac options"].registerCallback([]{ ransacOptions.switchVisibility(); });

  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
}



void run(){
  DrawHandle3D draw = gui["draw"];
  Image image = grabber.grabImage();
  draw = image.ptr();

  float tExp = gui["t"];
  float t = ::pow(10,tExp);

  surf->setThreshold(t);
  const std::vector<SurfMatch> &ms = surf->match(image.ptr());
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
