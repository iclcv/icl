// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// viz3d port of the legacy geom/surf-based-object-tracking app. The SURF
// matching + RANSAC pose estimation are untouched (they live in the CV core);
// only the visualisation moves to viz3d — the tracked box is a CuboidNode whose
// transform is set from the estimated pose each frame.

#include <icl/cv/SurfFeatureDetector.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/GeomDefs.h>
#include <icl/cv3d/RansacBasedPoseEstimator.h>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::cv;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

ImageSource grabber;
std::shared_ptr<SurfFeatureDetector> surf;
RansacBasedPoseEstimator *pe = 0;

Size32f ts; // template pixel -> mm
VBox gui;
GUI ransacOptions;
Scene2 scene;
std::shared_ptr<CuboidNode> obj;

void init(){
  grabber.init(pa("-i"));

  Size32f t(pa("-t",1).as<float>(), pa("-t",2).as<float>());
  float d = pa("-t",3);
  scene.addCamera(*pa("-cam"));
  pe = new RansacBasedPoseEstimator(scene.getCamera(0));

  obj = CuboidNode::create(t.width/2,t.height/2,d/2,t.width,t.height,d);
  obj->setMaterial(Material::fromColors(GeomColor(0,100,255,50), geom_red()));
  obj->setPrimitiveVisible(PrimLine, true);
  obj->setLineWidth(3);
  scene.addNode(obj);

  surf.reset(new SurfFeatureDetector(5,4,2,0.00005,"opensurf"));
  Img8u templ = icl::io::load(pa("-t")).as8u();
  surf->setReferenceImage(&templ);

  ts = Size32f(t.width/templ.getWidth(), t.height/templ.getHeight());

  gui << Canvas3D({.handle="draw", .minSize={32, 24}})
      << (HBox()
          << FSlider(-7, -1, -3, {.handle="t", .label="threshold exponent", .maxSize={99, 3}})
          << Button("ransac ...", {.handle="ransac options", .maxSize={6, 3}})
          << CheckBox("vis error", {.handle="vise", .maxSize={5, 3}})
          )
      << Show();

  pe->setConfigurableID("pe");
  pe->adaptProperty("iterations", "range", "[1,5000]:1", "");
  pe->adaptProperty("max error", "range", "[1,100]", "");
  pe->adaptProperty("min points", "range", "[4,10]:1", "");
  pe->adaptProperty("min points for good model", "range", "[4,100]:1", "");

  pe->prop("iterations").value = 200;
  pe->prop("max error").value = 30;
  pe->prop("min points").value = 4;
  pe->prop("min points for good model").value = 20;
  pe->prop("store last consensus set").value = true;

  ransacOptions << Prop("pe") << Create();

  gui["ransac options"].registerCallback([]{ ransacOptions.switchVisibility(); });

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
}

void run(){
  DrawHandle3D draw = gui["draw"];
  Image image = grabber.grab();
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
