// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <icl/geom/Geom.h>

#include <icl/geom/PointCloudObject.h>
#include <icl/geom/GenericPointCloudGrabber.h>
#include <icl/geom/RayCastOctreeObject.h>
#include <icl/geom/ComplexCoordinateFrameSceneObject.h>

#include <icl/geom/ObjectEdgeDetector.h>
#include <icl/cv/RegionDetector.h>
#include <icl/filter/MorphologicalOp.h>

#include <fstream>
#include <mutex>

using namespace icl;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::filter;
using namespace icl::geom;
using namespace icl::cv;
using namespace icl::qt;

HSplit gui;
Scene scene;

PointCloudObject obj;
std::recursive_mutex grabberMutex;
std::shared_ptr<GenericPointCloudGrabber> grabber;
const core::Img8u &calculate(const core::Img32f &depthImage, bool filter, bool average, bool gauss);
std::shared_ptr<RayCastOctreeObject> octree;
std::shared_ptr<ObjectEdgeDetector> oed;
MorphologicalOp closing(MorphologicalOp::closeBorder);

core::Img8u seg;

struct PointsIndicator : public SceneObject{
  PointsIndicator() {
    setLockingEnabled(true);
    setPointSmoothingEnabled(false);
    setPointSize(6);
    setVisible(Primitive::vertex, true, false);
  }
} *indicatorPoints = 0;

SceneObject *indicatorCS = 0;
Vec pos;
Point32f mPos;


void mouse(const MouseEvent &e){
  static MouseHandler *other = scene.getMouseHandler(0);
  other->process(e);

  bool lOt = false;
  bool lObj = false;
  try{
    octree->lock();
    lOt = true;
    Vec v = octree->rayCastClosest(scene.getCamera(0).getViewRay(e.getPos()), 5);
    v[3] = 1;
    octree->unlock();
    Point32f p = grabber->getDepthCamera().project(v);
    static RegionDetector rd;
    rd.detect(&seg);
    ImageRegion r = rd.click(p);

    if(r){
      Mat3 C(0.0f);
      Vec3 m(0.0f);

      const std::vector<Point> &ps = r.getPixels();
      obj.lock();
      lObj=true;
      DataSegment<float,3> xyz = obj.selectXYZ();

      if(!ps.size()) throw std::runtime_error("no points");


      indicatorPoints->lock();

      indicatorPoints->getVertices().clear();
      indicatorPoints->getVertexColors().clear();

      for(size_t i=0;i<ps.size();++i){
        m += xyz(ps[i].x,ps[i].y);
        indicatorPoints->addVertex(xyz(ps[i].x,ps[i].y).resize<1,4>(1), geom_red(100));
      }
      indicatorPoints->unlock();
      m *= 1.0/ps.size();

      for(size_t i=0;i<ps.size();++i){
        Vec3 v = xyz(ps[i].x,ps[i].y) - m;
        C += v * v.transp();
      }
      C *= 1.0/ps.size();

      Mat3 evecs;
      Vec3 evals;
      C.eigen(evecs,evals);

      // ensure right handness
      if(evecs.det() < 0){
        evecs.index_yx(0, 2) *= -1;
        evecs.index_yx(1, 2) *= -1;
        evecs.index_yx(2, 2) *= -1;
      }
      Vec3 n = scene.getCamera(0).getNorm().part<0,0,1,3>();
      Vec3 z = evecs.transp().col(2);
      if(float(n.transp() * z) > 0){
        evecs = evecs * Mat3(create_hom_4x4<float>(M_PI,0,0).part<0,0,3,3>());
      }


      Mat T;
      T.part<0,0,3,3>() = evecs;
      T.part<3,0,1,3>() = v.part<0,0,1,3>();
      T.part<0,3,4,1>() = Vec(0,0,0,1).transp();


      indicatorCS->lock();
      indicatorCS->removeTransformation();
      indicatorCS->setTransformation(T);

      indicatorCS->setVisible(true);
      indicatorCS->unlock();

      obj.unlock();

      if(e.isPressEvent() && (e.isModifierActive(ShiftModifier)
                              || e.isModifierActive(ControlModifier))){
        std::scoped_lock<std::recursive_mutex> lock(grabberMutex);
        grabber->setCameraWorldFrame(T);
        scene.getCamera(0).setWorldFrame(T);

        if(e.isModifierActive(ControlModifier)){
          std::string names[] = { "depth", "color" };
          int n = 1;
          try { grabber->getColorCamera(); ++n; } catch(...){}
          for(int i=0;i<n;++i){
            std::string fn = saveFileDialog("XML-Files (*.xml)",
                                            "Please define adapted "
                                            +names[i]+ " camera file");
            Camera cam = i ? grabber->getColorCamera() : grabber->getDepthCamera();
            std::ofstream s(fn.c_str());
            s << cam;
            std::cout << "saved adapted " << names[i] << " camera to " << fn << std::endl;
          }
        }
      }
    }
  }catch(...){
    if(lOt) octree->unlock();
    if(lObj) obj.unlock();
    indicatorCS->setVisible(false);
  }
}

void init(){
  grabber.reset(new GenericPointCloudGrabber);
  grabber->init(pa("-pci"));
  grabber->setConfigurableID("grabber");
  grabber->grab(obj);

  seg = Img8u(obj.getSize(),1);

  scene.setLightingEnabled(false);

  Camera cam(*pa("-d"));

  gui << Canvas3D(obj.getSize()).minSize(32,24).handle("scene")
      << (VBox().minSize(16,1)
          << Prop("grabber").hideIf(!pa("-tune"))
          << Label("Use \"SHIF-click\" in the 3D view to define\n"
                   "world frame and \"CTRL-click\" to also save it").minSize(15,2)
          << Display().handle("seg")
        )
      << Show();


  // kinect camera
  scene.addCamera(cam);
  scene.setBounds(1000);
  scene.addObject(&obj,false);

  gui["scene"].link(scene.getGLCallback(0));
  gui["scene"].install(mouse);

  obj.setPointSize(3);
  obj.setPointSmoothingEnabled(false);

  if(pa("-vc")){
    ProgArg p = pa("-vc");
    for(int i=0;i<p.n();++i){
      Camera c(p[i]);
      c.setName(p[i]);
      scene.addCamera(c);
    }
    scene.setDrawCamerasEnabled(true);
  }

  if(pa("-cs")){
    scene.setDrawCoordinateFrameEnabled(true);
  }

  indicatorCS = new ComplexCoordinateFrameSceneObject(50,2);
  indicatorCS->setVisible(false);
  scene.addObject(indicatorCS,true);

  indicatorPoints = new PointsIndicator;
  scene.addObject(indicatorPoints,true);

  oed.reset(new geom::ObjectEdgeDetector);
  octree.reset(new RayCastOctreeObject(-3000,6000));


  closing.setClipToROI(false);
  //closing.setCheckOnly(true);
}

void run(){
  grabberMutex.lock();
  DrawHandle3D draw = gui["scene"];
  grabber->grab(obj);
  grabberMutex.unlock();
  octree->fill(obj);


  grabberMutex.lock();
  const Img32f *di = grabber->getDepthDisplay();
  if(!di){
    throw std::logic_error("option '-redefine-world-frame' "
                           "does not work with the selected point cloud "
                           "grabber backend");
  }
  const Img8u &seg = oed->calculate(*di, true, true, true);
  grabberMutex.unlock();
  closing.apply(&seg,bpp(::seg));

  gui["seg"] = ::seg;

  draw.render();
}


int main(int n, char **ppc){
  pa_explain("-tune","adds an extra gui that allows certain depth "
             "camera parameters to be tuned manually at runtime");
  return ICLApp(n,ppc,"[m]-point-cloud-input|-pci(point-cloud-source,descrition) "
                "[m]-depth-camera|-d(filename) -color-camera|-c(filename) "
                "-tune -visualize-cameras|-vc(...) "
                "-show-world-frame|-cs",init,run).exec();
}
