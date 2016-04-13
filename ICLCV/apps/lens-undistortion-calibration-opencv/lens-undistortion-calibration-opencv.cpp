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
//#include <ICLMarkers/AdvancedMarkerGridDetector.h>

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
FiducialDetector *fid = 0;
//AdvancedMarkerGridDetector markerGridDetector;
DisplacementMap dmap;

struct MarkerInfo {
  std::vector<int> markerIdList;
  Size gridSize;

  Array2D<Fiducial> sort2D(const std::vector<Fiducial> & fids){
    std::map<int,Point> posLut;
    int idx = 0;
    for(int y=0;y<gridSize.height;++y){
      for(int x=0;x<gridSize.width;++x, ++idx){
        posLut[markerIdList[idx]] = Point(x,y);
      }
    }
    Array2D<Fiducial> a(gridSize);
    for(size_t i=0;i<fids.size();++i){
      Point p = posLut[fids[i].getID()];
      a(p.x,p.y) = fids[i];
    }
    return a;
  }
} markerInfo;

struct ConfigurableUDist : public Configurable{
  std::vector<double> defaultValues;
  bool paramChanged;
  SmartPtr<ImageUndistortion> udist;
  ConfigurableUDist(){
    setConfigurableID("udist");
    paramChanged = true;
  }

  void cb(const Property &p){
    //return ;
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
      registerCallback(function(this,&ConfigurableUDist::cb));
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
  }
  catch (...){
    std::cout << "error while saving parameters" << std::endl;
  }
  std::cout << "parameters saved" << std::endl;
}


SceneObject *create_grid_preview_object(const std::vector<Point32f> &markerCorners, 
                                        const std::vector<Point32f> &objCoordsXY,
                                        const Camera &cam, const Size &gridDim,
                                        const LensUndistortionCalibrator::Info &info){
  CoplanarPointPoseEstimator cpe(CoplanarPointPoseEstimator::worldFrame, CoplanarPointPoseEstimator::HomographyBasedOnly);
  Mat T = cpe.getPose(objCoordsXY.size(), objCoordsXY.data(), markerCorners.data(), cam);

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
  const int w = info.gridDef.getGridBoundarySize().width, h = info.gridDef.getGridBoundarySize().height;

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

  //gui["calibrate"].disable();
  //  gui["save"].disable();

  vecDraw->render();
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
  /*
  std::vector<Fiducial> ret;
  std::vector<Fiducial>::const_iterator it = fids.begin();
  std::vector<Fiducial>::const_iterator prevIt = it;

  for (++it; it != fids.end(); ++it) {
    if (prevIt->getID() != it->getID()) {
      ret.push_back(*prevIt);
      prevIt = it;
    } else {
      do ++it; while (it != fids.end() && prevIt->getID() == it->getID());
      prevIt = it;
      if (it == fids.end()) break;
    }
  }

  if (prevIt != fids.end()) ret.push_back(*prevIt);

  return ret;
      */
}

// For every id in "markerIdList" this function creates a list
// of bool values indicating if the marker was found
int createFoundList(const std::vector<Fiducial> &fids, std::vector<bool> &foundList) {
  int validMarkers = 0;
  Size &s = markerInfo.gridSize;
  std::vector<int> &ids = markerInfo.markerIdList;
  std::vector<Fiducial>::const_iterator it = fids.begin();

  foundList = std::vector<bool>(s.width*s.height, false);
  for(size_t i=0;i<fids.size();++i){
    int id = fids[i].getID();
    std::vector<int>::iterator it = std::find(ids.begin(), ids.end(), id);
    if(it != ids.end()){
      foundList[(int)(it - ids.begin())] = true;
    }
  }
  for(size_t i=0;i<foundList.size();++i){
    if(foundList[i]) ++validMarkers;
  }

#if 0
  

  for (int y = 0; y < s.height && it != fids.end(); ++y) {
    for (int x = 0; x < s.width; ++x) {
      int pos = x + y*s.width;

      if (it != fids.end() && ids[pos] == it->getID()) {
        foundList[pos] = true;
        it++;
      }
    }
  }
#endif

  return validMarkers;
}

// This functions fills the vector "obj" with object coordinates
// of the marker grid, but only with the found markers
void createObjCoords(const std::vector<Point32f> &grid,
                     const std::vector<bool> &foundList,
                     std::vector<Point32f> &obj) {
  ICLASSERT(grid.size() == 4 * foundList.size());
  obj.clear();
  SHOW(grid.size());
  SHOW(foundList.size());

  std::vector<Point32f>::const_iterator it = grid.begin();
  
  for(size_t i=0;i<foundList.size();++i){
    if(foundList[i]){
      std::cout << "found index " << i << std::endl;
      for(int j=0;j<4;++j) obj.push_back(*it++);
    }else{
      std::cout << "didn't find index " << i << std::endl;
      it+=4;
    }
  }
  /*
  std::vector<bool>::const_iterator it  = foundList.begin();
  std::vector<bool>::const_iterator end = foundList.end();

  for (; it != end; ++it) {
    if (*it) {
      obj.push_back(*gridIt); gridIt++;
      obj.push_back(*gridIt); gridIt++;
      obj.push_back(*gridIt); gridIt++;
      obj.push_back(*gridIt); gridIt++;
    } else {
      gridIt++;
      gridIt++;
      gridIt++;
      gridIt++;
    }
  }*/
}

// comparision between two fiducial objects
struct fidComp {
  bool operator() (Fiducial i, Fiducial j) { return (i.getID()<j.getID()); }
} comp;

void handleMarkerDetection(const ImgBase *img, DrawHandle &draw) {
  static ButtonHandle capture = gui["capture"];
  const int minMarkers = gui["minMarkers"];
  std::vector<Fiducial> fids = fid->detect(img);
  gui["nfound"] = str(fids.size());

  bool haveEnoughMarkers = ((int)fids.size() >= minMarkers);
  // first check if enough markers were found
  
  std::vector<bool> foundList;
  
  std::sort(fids.begin(), fids.end(), comp);
  fids = removeDuplicates(fids);
  
  Array2D<Fiducial> fids2D = markerInfo.sort2D(fids);
  
  
  
  for(int y=0;y<fids2D.getHeight();++y){
    for(int x=0;x<fids2D.getWidth();++x){
      Fiducial &f = fids2D(x,y);
      if(f){
        if(haveEnoughMarkers) draw->color(0,255,0,255);
        else draw->color(255,0,0,255);
        draw->linewidth(1);
        draw->linestrip(f.getCorners2D());
        draw->color(255,0,0,255);
        
        draw->linewidth(2);
        if(x < fids2D.getWidth()-1){
          Fiducial &left = fids2D(x+1,y);
          if(left){
            draw->line(f.getCenter2D(), left.getCenter2D());
          }
        }
        if(y < fids2D.getHeight()-1){
          Fiducial &bottom = fids2D(x,y+1);
          if(bottom){
            draw->line(f.getCenter2D(), bottom.getCenter2D());
          }
        }
      }
      
    }
  }
  
  // second check if there are enough markers after sorting out invalid markers
  if (haveEnoughMarkers) {
    const bool autoCapture = gui["autoCapture"];
    const float displacement = gui["captureDis"];
    float diff = -1.f;
    
    // visualize markers
    //for (unsigned int i = 0; i < fids.size(); ++i){
    //  draw->color(255, 0, 0, 255);
    //  draw->linestrip(fids[i].getCorners2D());
    //  draw->color(0, 100, 255, 255);
    //  draw->text(fids[i].getName(), fids[i].getCenter2D().x, fids[i].getCenter2D().y, 9);
    //}
    
    // if automatic capturing is on, calculate the current displacement
    if (autoCapture) {
      if (lastCapturedMarkers.size()) {
        int count = 0;
        std::vector<Fiducial>::const_iterator it = fids.begin();
        diff = 0;
        
        for (; it != fids.end(); ++it) {
          if (lastCapturedMarkers[it->getID()] != Point32f()) {
            Point32f tmp = it->getCenter2D() - lastCapturedMarkers[it->getID()];
            diff += sqrt(tmp.x*tmp.x + tmp.y*tmp.y);
            ++count;
          }
        }
        
        if (count < 3) diff = std::numeric_limits<float>::max();
        else diff /= count;
      }
      else diff = std::numeric_limits<float>::max();
    }
    
    // handle capturing
    if (capture.wasTriggered() || (diff >= displacement)) {
      std::vector<Point32f> obj;
      std::vector<Point32f> corners;
      LensUndistortionCalibrator::Info info = calib.getInfo();
      Size &grid = markerInfo.gridSize;
      std::vector<Fiducial>::const_iterator it = fids.begin();
      
      for (; it != fids.end(); ++it)
        lastCapturedMarkers[it->getID()] = it->getCenter2D();
      
      for (it = fids.begin(); it != fids.end(); ++it) {
        const std::vector<Fiducial::KeyPoint> &points = it->getKeyPoints2D();
        corners.push_back(points[3].imagePos);
        corners.push_back(points[0].imagePos);
        corners.push_back(points[1].imagePos);
        corners.push_back(points[2].imagePos);
      }
      
      createFoundList(fids, foundList);
      createObjCoords(info.gridDef, foundList, obj);
      ICLASSERT(corners.size() == obj.size());
      
      calib.addPoints(corners, obj);

      /// yyy 
      scene.addObject(create_grid_preview_object(corners, obj, scene.getCamera(1), grid, info));
      /*
      std::vector<Vec> ps = estimage_grid_preview(corners, obj, scene.getCamera(1), grid);
      
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
      */
      gui["calibrate"].enable();
    }
  }
}

void handleCheckerboardDetection(const ImgBase *img, DrawHandle &draw) {
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
  int maxMarkers = 5;
  string dStr;

  grabber.init(pa("-i"));
  if(pa("-s")){
    grabber.useDesired(pa("-s").as<Size>());
  }
  const ImgBase *image = grabber.grab();
  udist.init(image->getSize());
  
  if(pa("-cb")){
    dStr = string("checker board detection");
    grabber.useDesired(depth8u);
    checker.init(pa("-cb").as<Size>());
    Size s = pa("-cb").as<Size>();
    Size r = pa("-r").as<Size>();
    calib.init(image->getSize(), LensUndistortionCalibrator::GridDefinition(s, Size(r.width / s.width, r.height / s.height)));
    checker.setConfigurableID("detectionProps");
  }else if (pa("-m")) {
    dStr = string("quad detection");
    std::vector<int> &idList = markerInfo.markerIdList;
    Size &s = markerInfo.gridSize;

    idList = FiducialDetectorPlugin::parse_list_str(pa("-m", 1).as<std::string>());
    //std::sort(idList.begin(), idList.end()); // nooooo!

    s = pa("-g").as<Size>();
    if (s.width*s.height != (int)idList.size())
      throw ICLException("the number of markers must be equal to the grid size");

    fid = new FiducialDetector(pa("-m").as<std::string>(),
                               pa("-m", 1).as<std::string>(),
                               ParamList("size", (*pa("-m", 2))));
    LensUndistortionCalibrator::GridDefinition def(pa("-g").as<Size>(), pa("-m", 2).as<Size32f>(), pa("-sp").as<Size32f>());
    calib.init(image->getSize(), def);

    maxMarkers = s.width*s.height;

    //QuadDetector &qd = ((FiducialDetectorPluginForQuads*)fid->getPlugin())->getQuadDetector();
    fid->setConfigurableID("detectionProps");
    fid->setPropertyValue("max bch errors",1);
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
           << CheckBox("auto capture", false).out("autoCapture")
              .tooltip("if checked and the displacement is higher than a threshold,\n"
                       "the current detection will be captured.")
           << FSlider(0.f, 200.f, 10.f).out("captureDis").label("displacement")
           << CamCfg()
           << Button("capture").handle("capture")
           << Button("calibrate").handle("calibrate")
           << Button("save").handle("save")
           << Button("reset").handle("reset")
           
           << (HBox()
               << Slider(5, maxMarkers, 5).hideIf(!fid).out("minMarkers")
                  .label("minimum markers").tooltip("minimum number of markers that is needed for the calibration")
               << Label("--").handle("nfound").label("#markers found").hideIf(!fid).maxSize(5,2)
               )
           << ( Tab("manual params,detection")
                << Prop("udist")
                << Prop("detectionProps")
              )
         )
      << Show();

  
  Camera cam(Vec(0,0,-1,1));
  cam.setResolution(image->getSize());
  
  struct Frustrum : public SceneObject{
    Frustrum(const Camera &cam, const Size &size){
      const Point ps[] = { Point(0,0), Point(size.width-1, 0),
                           Point(size.width-1,size.height-1),
                           Point(0, size.height-1) };
      addVertex(cam.getPosition());
      for(int i=0;i<4;++i){
        addVertex(cam.getViewRay(ps[i])(1000));
        addLine(0,i+1);
        addLine(i+1, i==3 ? 1 : i+2);
      }
      
      setColor(Primitive::line,geom_blue(200));
    }
  };
  
  scene.addCamera(cam);
  scene.addCamera(cam);
  scene.addObject(new Frustrum(cam,image->getSize()));
  
  gui["plot"].link(scene.getGLCallback(0));
  gui["plot"].install(scene.getMouseHandler(0));
  gui["calibrate"].disable();
  //gui["save"].disable();
}

void run(){
  static ButtonHandle reset     = gui["reset"];
  static ButtonHandle calibrate = gui["calibrate"];
  static ButtonHandle save      = gui["save"];
  static DrawHandle draw = gui["image"];
  static ImageHandle udraw = gui["uimage"];
  static DrawHandle vecDraw = gui["vecImage"];
  const bool detection   = gui["detection"];

  // handle reset button
  if (reset.wasTriggered()) {
    resetData();
  }

  const ImgBase *img = grabber.grab();
  draw = img;
  if(warp.getWarpMap().getSize().getDim()){
    static Img8u warped;
    warped.fill(0);
    warp.apply(img,bpp(warped));
    udraw = warped;
  }

  if(detection){
    if(fid) {
      handleMarkerDetection(img, draw);
    }
    if(!checker.isNull()) {
      handleCheckerboardDetection(img, draw);
    }
  }

  // handle calibrate button
  if(calibrate.wasTriggered()){
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
    "(number of ids should match the marker grid size)"
    "The last argument defines the size of one marker in milimeter.")
    ("-g", "size of the marker grid")
    ("-sp", "spacing between markers")
    ("-s", "output size of the input image");
  return ICLApp(n,ppc,"[m]-input|-i(2) -checkerboard|-cb(WxH) "
                "-real-checkerboard-dim-mm|-r(WxH=240x180) "
                "-marker-type|-m(type=bch,whichToLoad=[0-4095],size=30x30) "
                "-marker-spacing|-sp(WxH=35x35) -marker-grid|-g(WxH=4x3) "
                "-force-size|-s(size)",init,run).exec();
}
