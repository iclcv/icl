/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/lens-undistortion-calibration-opencv/     **
**          lens-undistortion-calibration-opencv.cpp               **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Christian Groszewski              **
**          Andre Ueckermann, Sergius Gaulik                       **
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

#define ICL_NO_USING_NAMESPACES


#include "DisplacementMap.h"
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Mutex.h>
#include <ICLCore/ConvexHull.h>
#include <ICLFilter/WarpOp.h>
#include <ICLQt/Common.h>
#include <ICLCV/LensUndistortionCalibrator.h>
#include <ICLCV/CheckerboardDetector.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLMarkers/FiducialDetectorPluginForQuads.h>
#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLMarkers/MarkerGridPoseEstimator.h>

#include <ICLUtils/Array2D.h>
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::cv;
using namespace icl::io;
using namespace icl::filter;
using namespace icl::geom;
using namespace icl::math;
using namespace icl::core;
using namespace icl::markers;

HSplit gui;
GenericGrabber grabber;
CheckerboardDetector checker;
LensUndistortionCalibrator calib;
Scene scene;
//ImageUndistortion udist;
std::vector<Point32f> lastCapturedCorners;
std::map<int,Point32f> lastCapturedMarkers;
ImgBase *splitImage = 0;
WarpOp warp;
//FiducialDetector *fid = 0;
AdvancedMarkerGridDetector markerGridDetector;
MarkerGridPoseEstimator markerGridPoseEstimator;

DisplacementMap dmap;
std::vector<Mat> capturedPoses;


bool is_far_enough(const Mat &T, float f){
  float fEuler = f/200.0 * M_PI/2; // 45 deg
  float fTrans = f;                // 200 mm

  Mat3 R = T.part<0,0,3,3>();
  Vec3 t = T.part<3,0,1,3>();

  bool allFarEnough = true;
  for(size_t i=0;i<capturedPoses.size();++i){
    const Mat &C = capturedPoses[i];
    Mat3 CR = C.part<0,0,3,3>();
    Vec3 Ct = C.part<3,0,1,3>();
    Vec3 dEuler = extract_euler_angles(R.transp() * CR);
    Vec3 dTrans = t - Ct;

    bool oneFarEnough = false;
    for(int i=0;i<3;++i){
      if(fabs(dEuler[i]) > fEuler || fabs(dTrans[i]) > fTrans){
        oneFarEnough = true; break;
      }
    }
    if(!oneFarEnough){
      allFarEnough = false;
      break;
    }
  }
  return allFarEnough;
}

struct ConfigurableUDist : public Configurable{
  std::vector<double> defaultValues;
  bool paramChanged;
  SmartPtr<ImageUndistortion> udist;
  ConfigurableUDist(){
    setConfigurableID("udist");
    paramChanged = true;
  }

  void cb(const Property &p){
    if(p.name != "activated" && getPropertyValue("activated").as<bool>()){
      DEBUG_LOG("before: " << *udist);
      std::vector<double> ps = udist->getParams();
      if(p.name == "fx"){
        ps[0] = parse<double>(p.value);
      }else if(p.name == "fy"){
        ps[1] = parse<double>(p.value);
      }else if(p.name == "ix"){
        ps[2] = parse<double>(p.value);
      }else if(p.name == "iy"){
        ps[3] = parse<double>(p.value);
      }else if(p.name == "skew"){
        ps[4] = parse<double>(p.value);
      }else{
        int idx = parse<int>(p.name.substr(1));
        ps[idx+4] = parse<double>(p.value);
      }
      udist->setParams(ps);
      DEBUG_LOG("after: " << *udist);
      paramChanged = true;
    }
  }

  void updateConfigurableParams(){
    setPropertyValue("activated",false);
    const std::vector<double> &values = udist->getParams();
    std::vector<std::string> ps = tok("fx,fy,ix,iy,skew,k1,k2,k3,k4,k5",",");
    for(int i=0;i<10;++i){
      setPropertyValue(ps[i], values[i]);
    }
    paramChanged = true;
  }

  void init(const Size &size, bool addProperties=true){
#define add(X,MIN,MAX,CUR) \
    addProperty(#X,"range","[" +str(MIN)+ "," +str(MAX)+ "]",CUR);     \
    defaultValues.push_back(CUR);

    if(addProperties){
      add(fx,10,10000,1000);
      add(fy,10,10000,1000);
      add(ix,0,size.width,size.width/2);
      add(iy,0,size.height,size.height/2);
      add(skew,0,10,0);
      add(k1,-10,10,0);
      add(k2,-100,100,0);
      add(k3,-10,10,0);
      add(k4,-10,10,0);
      add(k5,-10000,10000,0);
      registerCallback(icl::utils::function(this,&ConfigurableUDist::cb));
    }

    udist = new ImageUndistortion("MatlabModel5Params", defaultValues, size);
    paramChanged = true;
    addProperty("activated","flag","",false,0,"Activate this to manually adapt values.<b>Please note</b> that the "
                "slider-based setting of the parameters doesn't allow all parameter to be set as accurate as it would "
                "be needed. Therefore, this flag is automatically disabled when the automatic calibration is performed");
  }

  void reset(){
    init(udist->getImageSize(),false);
  }
} udist;

void save_params(){
  try{
    std::string filename = saveFileDialog("XML-Files (*.xml)");
    ConfigFile f;
    f.setPrefix("config.");

    f["size.width"] = udist.udist->getImageSize().width;
    f["size.height"] = udist.udist->getImageSize().height;
    f["model"] = udist.udist->getModel();

    const std::vector<double> &p = udist.udist->getParams();
    f["intrin.fx"] = p[0];
    f["intrin.fy"] = p[1];
    f["intrin.ix"] = p[2];
    f["intrin.iy"] = p[3];
    f["intrin.skew"] = p[4];
    f["udist.k1"] = p[5];
    f["udist.k2"] = p[6];
    f["udist.k3"] = p[7];
    f["udist.k4"] = p[8];
    f["udist.k5"] = p[9];

    f.save(filename);
    DEBUG_LOG("parameters saved");
  }
  catch (...){
    ERROR_LOG("saving parameters");
  }
}


SceneObject *create_grid_preview_object(const std::vector<Point32f> &markerCorners,
                                        const std::vector<Point32f> &objCoordsXY,
                                        const AdvancedMarkerGridDetector::MarkerGrid &grid,
                                        const Camera &cam, const Mat &T){
  static std::vector<GeomColor> cs;
  if(!cs.size()){
    const int c[3] = {0,128,255};
    for(int r=0;r<3;++r){
      for(int g=0;g<3;++g){
        for(int b=0;b<3;++b){
          if(c[r] + c[g] + c[b] > 128){
            cs.push_back(GeomColor(c[r], c[g], c[b],200));
          }
        }
      }
    }
  }
  static size_t cidx = 0;
  GeomColor c = cs[cidx++];
  if(cidx >= cs.size()) cidx = 0;


  SceneObject *obj = new SceneObject;
  int n = (int)objCoordsXY.size();
  for(int i=0; i<n; i+=4){
    for(int d=0;d<4;++d){
      obj->addVertex(Vec(objCoordsXY[i+d].x, objCoordsXY[i+d].y, 0, 1));
    }
    obj->addLine(i,i+1, c);
    obj->addLine(i+1,i+2,c);
    obj->addLine(i+2,i+3, c);
    obj->addLine(i+3,i, c);
  }
  const int w = grid.getGridDef().getGridBounds().width, h = grid.getGridDef().getGridBounds().height;

  obj->addVertex(Vec(0,0,0,1));
  obj->addVertex(Vec(w,0,0,1));
  obj->addVertex(Vec(w,h,0,1));
  obj->addVertex(Vec(0,h,0,1));
  obj->addLine(n,n+1, c);
  obj->addLine(n+1,n+2, c);
  obj->addLine(n+2,n+3, c);
  obj->addLine(n+3,n,c);

  obj->setVisible(Primitive::vertex,false);
  obj->setVisible(Primitive::quad,true);
  obj->setVisible(Primitive::line,true);
  obj->setTransformation(T);

  return obj;
}

#if 0
std::vector<Vec> estimage_grid_preview(const std::vector<Point32f> &imageCoords,
                                       const std::vector<Point32f> &obj,
                                       const Camera &cam, const Size32f &realBoardDim){
  static CoplanarPointPoseEstimator cpe(CoplanarPointPoseEstimator::worldFrame, CoplanarPointPoseEstimator::HomographyBasedOnly);
  Mat T = cpe.getPose(obj.size(), obj.data(), imageCoords.data(), cam);

  // get the boundary point of the object (just for visualization purpose)
  std::vector<Point32f> hull = convexHull(obj);

  // transform object boundary points to the world
  std::vector<Vec> r;
  for(size_t i = 0;i<hull.size();++i){
    r.push_back(T * Vec(hull[i].x, hull[i].y, 0, 1) );
  }

  return r;
}
#endif

// delete all collected and calculated data
void resetData() {
  static DrawHandle vecDraw = gui["vecImage"];

  if (scene.getObjectCount() < 2) return;

  scene.removeObjects(1);
  calib.clear();
  udist.reset();

  gui["calibrate"].disable();
  gui["save"].disable();

  capturedPoses.clear();
  vecDraw->render();
}

void undoLast() {
  static DrawHandle vecDraw = gui["vecImage"];

  if (scene.getObjectCount() < 2) return;

  calib.undoLast();
  if(capturedPoses.size()){
    capturedPoses.pop_back();
    scene.removeObject(scene.getObjectCount()-1);
    if(gui["autoCalib"]){
      *udist.udist = calib.computeUndistortion();
      udist.updateConfigurableParams();
      udist.paramChanged = true;
    }
    vecDraw->render();
  }

}



// This functions removes all non-unique markers
/** Doubled markers are removed (all instances, as we don't
    know which one is the right one) */
std::vector<Fiducial> removeDuplicates(const std::vector<Fiducial> &fids) {

  std::map<int,std::vector<Fiducial> > lut; // id->count
  for(size_t i=0;i<fids.size();++i){
    lut[fids[i].getID()].push_back(fids[i]);
  }
  std::vector<Fiducial> r;
  for(std::map<int,std::vector<Fiducial> >::iterator it = lut.begin();
      it != lut.end();++it){
    if(it->second.size() == 1){
      r.push_back(it->second.front());
    }
  }
  return r;
}




void handleMarkerDetection(const ImgBase *img, DrawHandle &draw, bool &captured) {
  static ButtonHandle capture = gui["capture"];
  const int minMarkers = gui["minMarkers"];

  typedef AdvancedMarkerGridDetector::Marker Marker;
  typedef AdvancedMarkerGridDetector::MarkerGrid MarkerGrid;

  const MarkerGrid &grid = markerGridDetector.detect(img);
  std::vector<Point32f> imageCoords, objectCoords;
  for(int i=0;i<grid.getDim();++i){
    const Marker &m = grid[i];
    if(m.wasFound()){
      m.getImagePoints().appendCornersTo(imageCoords);
      m.getGridPoints().appendCornersTo(objectCoords);
    }
  }
  draw->draw(grid.vis());

  if((int)imageCoords.size()/4 >= minMarkers){
    bool autoCapture = gui["autoCapture"];
    bool doCapture = capture.wasTriggered();
    Mat T = markerGridPoseEstimator.computePose(grid, scene.getCamera(1));

    if(!doCapture && autoCapture){
      float f = gui["captureDis"];
      if(is_far_enough(T, f)){
        doCapture = true;
      }
    }
    if(doCapture){
      captured = true;
      capturedPoses.push_back(T);
      calib.addPoints(imageCoords, objectCoords);
      scene.addObject(create_grid_preview_object(imageCoords, objectCoords,
                                                 grid, scene.getCamera(1), T));
      gui["calibrate"].enable();
      gui["save"].enable();
    }
  }
}

void handleCheckerboardDetection(const ImgBase *img, DrawHandle &draw, bool &captured) {
  static ButtonHandle capture = gui["capture"];

  const CheckerboardDetector::Checkerboard &cb = checker.detect(*img->as8u());

  if (cb.found){
    const bool autoCapture = gui["autoCapture"];
    const float displacement = gui["captureDis"];
    float diff = -1.f;
    draw->draw(cb.visualize());

    // if automatic capturing is on, calculate the current displacement
    if (autoCapture) {
      if (lastCapturedCorners.size()) {
        ICLASSERT(lastCapturedCorners.size() == cb.corners.size());
        diff = 0;

        for (unsigned int i = 0; i < lastCapturedCorners.size(); ++i) {
          Point32f tmp = lastCapturedCorners[i] - cb.corners[i];
          diff += sqrt(tmp.x*tmp.x + tmp.y*tmp.y);
        }

        diff /= lastCapturedCorners.size();
      }
      else diff = displacement;
    }

    // handle capturing
    if (capture.wasTriggered() || (diff >= displacement)){
      captured = true;
      lastCapturedCorners = cb.corners;
      calib.addPoints(cb.corners);
      //LensUndistortionCalibrator::Info info = calib.getInfo();
      TODO_LOG("add correct object!");
      //      scene.addObject(create_grid_preview_object(cb.corners, ubfi,gridDef, scene.getCamera(1), grid, info);xxx
#if 0
      std::vector<Vec> ps = estimage_grid_preview(cb.corners, info.gridDef, scene.getCamera(1), pa("-r"));
      struct LineStrip : public SceneObject{
        LineStrip(const std::vector<Vec> &ps){
          m_vertices = ps;
          m_vertexColors.resize(ps.size(), geom_red());
          for (size_t i = 0; i<ps.size(); ++i){
            addLine(i, (i + 1) % ps.size(), geom_red());
          }
        }
      };
      scene.addObject(new LineStrip(ps));
#endif
      gui["calibrate"].enable();
    }
  }
}

void init(){
  std::string dStr;
  int maxMarkers = 5;
  grabber.init(pa("-i"));
  if(pa("-s")){
    grabber.useDesired(pa("-s").as<Size>());
  }
  const ImgBase *image = grabber.grab();
  udist.init(image->getSize());

  if(pa("-cb")){
    dStr = "checker board detection";
    grabber.useDesired(depth8u);
    checker.init(pa("-cb").as<Size>());
    Size s = pa("-cb").as<Size>();
    Size r = pa("-r").as<Size>();
    calib.init(image->getSize(), LensUndistortionCalibrator::GridDefinition(s, Size(r.width / s.width, r.height / s.height)));
    checker.setConfigurableID("detectionProps");
  }else if (pa("-m")) {
    dStr = "quad detection";

    std::vector<int> ids = FiducialDetectorPlugin::parse_list_str(pa("-m", 1).as<std::string>());
    Size gridCells = pa("-g");
    Size32f markerBounds = pa("-m",2);
    Size32f spacing = pa("-sp");
    Size32f gridBounds((gridCells.width-1) * spacing.width + markerBounds.width * gridCells.width,
                       (gridCells.height-1) * spacing.height + markerBounds.height * gridCells.height);

    maxMarkers = gridCells.getDim();
    AdvancedMarkerGridDetector::AdvancedGridDefinition def(gridCells, markerBounds, gridBounds, ids, pa("-m",0));
    markerGridDetector.init(def);
    calib.init(image->getSize());

    markerGridDetector.setConfigurableID("detectionProps");
  } else {
    throw ICLException("other modes than checkerboard and markers "
                       "detection are not yet supported\nPlease use "
                       "either -m or -cb program argument to choose "
                       "'checkerboard' or 'markers' mode");
  }

  gui << ( VSplit().label("image")
           << Draw().label("input image").handle("image")
           << Image().label("undistorted image").handle("uimage")
         )
      << ( VSplit().label("data")
           << Draw3D(image->getSize()).label("recorded planes").handle("plot")
           << Draw().label("displacement map").handle("vecImage")
         )
      << ( VBox().label("controls").minSize(15,1)
           << CheckBox("detection", true).out("detection")
              .tooltip("if checked the application will try to find\n"
                       "a checkboard inside the image.")
           << (HBox()
               << CheckBox("auto capture", false).out("autoCapture")
               .tooltip("if checked and the displacement is higher than a threshold,\n"
                        "the current detection will be captured.")
               << CheckBox("auto calib",false).out("autoCalib")
               .tooltip("automatically trigger calibration after capturing a frame")
              )
           << FSlider(0.f, 200.f, 60.f).out("captureDis").label("min displacement").tooltip("minimal displacement for automatic capturing")
           << CamCfg()
           << ( HBox()
                << Button("capture").handle("capture")
                << Button("calibrate").handle("calibrate")
              )
           << ( HBox()
                << Button("reset").handle("reset")
                << Button("undo last").handle("undo")
              )
           << Button("save").handle("save")
           << (HBox()
               << Slider(5, maxMarkers, 5).hideIf(!pa("-g")).out("minMarkers")
                  .label("minimum markers").tooltip("minimum number of markers that is needed for the calibration")
               << Label("--").handle("nfound").label("#markers found").hideIf(!pa("-g")).maxSize(5,2)
               )
           << ( Tab("manual params,detection")
                << Prop("udist")
                << Prop("detectionProps")
              )
         )
      << Show();


  Camera cam(Vec(0,0,-1,1));
  cam.setFocalLength(1);
  cam.setResolution(image->getSize());
  cam.setSamplingResolution(image->getSize().width, image->getSize().width);
  struct Frustrum : public SceneObject{
    Frustrum(const Camera &cam, const Size &size){
      const Point ps[] = { Point(0,0), Point(size.width-1, 0),
                           Point(size.width-1,size.height-1),
                           Point(0, size.height-1) };
      addVertex(cam.getPosition());
      for(int i=0;i<4;++i){
        addVertex(cam.getViewRay(ps[i])(2*size.width));
        addLine(0,i+1);
        addLine(i+1, i==3 ? 1 : i+2);
      }

      setColor(Primitive::line,geom_blue(200));
    }
  };

  scene.addCamera(cam);
  scene.addCamera(cam);
  scene.addObject(new Frustrum(cam,image->getSize()));
  scene.setCursor(Vec(0,0,-image->getSize().width));

  gui["plot"].link(scene.getGLCallback(0));
  gui["plot"].install(scene.getMouseHandler(0));
  gui["calibrate"].disable();
  //gui["save"].disable();
}

void run(){
  static ButtonHandle reset     = gui["reset"];
  static ButtonHandle calibrate = gui["calibrate"];
  static ButtonHandle save      = gui["save"];
  static ButtonHandle undo      = gui["undo"];
  static DrawHandle draw = gui["image"];
  static ImageHandle udraw = gui["uimage"];
  static DrawHandle vecDraw = gui["vecImage"];
  const bool detection   = gui["detection"];

  // handle reset button
  if (reset.wasTriggered()) {
    resetData();
  }
  if(undo.wasTriggered()){
    undoLast();
  }


  const ImgBase *img = grabber.grab();
  draw = img;

  if(warp.getWarpMap().getSize().getDim()){
    static ImgBase *warped = 0;
    if(warped){
      warped->clear(-1,0,false);
    }
    warp.apply(img,&warped);
    udraw = warped;
  }

  bool captured = false;
  if(detection){
    static bool useMarkers = pa("-g");
    if(useMarkers) {
      handleMarkerDetection(img, draw, captured);
    }
    if(!checker.isNull()) {
      handleCheckerboardDetection(img, draw, captured);
    }
  }

  // handle calibrate button
  if(calibrate.wasTriggered() || (gui["autoCalib"].as<bool>() && captured)){
    *udist.udist = calib.computeUndistortion();
    udist.updateConfigurableParams();
    udist.paramChanged = true;
    SHOW(udist.udist);
    // gui["save"].enable();
  }
  if(udist.paramChanged){
    udist.paramChanged = false;
    const Img32f &mapping = udist.udist->createWarpMap();
    warp.setWarpMap(mapping);
    dmap.visualizeTo(mapping, vecDraw);
  }
  draw->render();

  // handle save button
  if (save.wasTriggered() && !udist.udist->isNull()) {
    save_params();
  }

  gui["plot"].render();
}

int main(int n, char **ppc){
  pa_explain
    ("-i", "The first sub-argument defines the input device type. "
    "The 2nd sub-argument defines the id of the device.")
    ("-cb", "If defined the application will use a checker board for the calibration. "
    "The argument defines the grid size.")
    ("-r", "dimension of the checker board in milimeter")
    ("-m", "The first sub-argument defines the marker type. "
    "The second one defines the marker ids, for which the application will search for."
    "(number of ids must match the marker grid size)"
    "The last argument defines the size of one marker in milimeters.")
    ("-g", "size of the marker grid, i.e. number of grid cells")
    ("-sp", "spacing between markers in mm (usually in the order of a 10th of "
     "the marker-size, i.e. the size of the white space between the markers)")
    ("-s", "output size of the input image");
  return ICLApp(n,ppc,"[m]-input|-i(2) -checkerboard|-cb(WxH) "
                "-real-checkerboard-dim-mm|-r(WxH=240x180) "
                "-marker-type|-m(type=bch,whichToLoad=[0-629],size=8x8) "
                "-marker-spacing|-sp(WxH=0.8x0.8) -marker-grid|-g(WxH=30x21) "
                "-force-size|-s(size)",init,run).exec();
}
