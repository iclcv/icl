/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/lens-undistortion-calibration/            **
**          lens-undistortion-calibration.cpp                      **
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

#include <ICLUtils/ConfigFile.h>
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
ImageUndistortion udist;
std::vector<Point32f> lastCapturedCorners;
std::map<int,Point32f> lastCapturedMarkers;
ImgBase *splitImage = 0;
WarpOp warp;
FiducialDetector *fid = 0;

struct MarkerInfo {
  std::vector<int> markerIdList;
  Size gridSize;
} markerInfo;

struct ImageSplit {
  Mutex mutex;
  int pos;      // current position of the line
  bool drag;    // shows if the line is being dragged
  bool overlap; // shows if mouse is near the line
} imageSplit;

// mouse handling inside of the draw component
void splitImageMouse(const MouseEvent &e) {
  imageSplit.mutex.lock();
  imageSplit.overlap = false;

  // take split line
  if (e.hitImage()) {
    if (abs(e.getPos().x - imageSplit.pos) < 5) {
      imageSplit.overlap = true;
      if (e.isPressEvent() && e.isLeftOnly())
        imageSplit.drag = true;
    }
  }

  // drag split line
  if (imageSplit.drag && e.isDragEvent()) {
    static DrawHandle draw = gui["image"];
    int iWidth = draw->getImageSize().width;
    int &pos = imageSplit.pos;
    pos = e.getPos().x;
    if (pos < 0)
      pos = 0;
    else if (pos >= iWidth)
      pos = iWidth - 1;
  }

  // release split line
  if (e.isReleaseEvent())
    imageSplit.drag = false;

  imageSplit.mutex.unlock();
}

void save_params(){
  try{
    std::string filename = saveFileDialog("XML-Files (*.xml)");
    ConfigFile f;
    f.setPrefix("config.");

    f["size.width"] = udist.getImageSize().width;
    f["size.height"] = udist.getImageSize().height;
    f["model"] = udist.getModel();

    const std::vector<double> &p = udist.getParams();
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

// delete all collected and calculated data
void resetData() {
  static DrawHandle vecDraw = gui["vecImage"];

  if (scene.getObjectCount() < 2) return;

  scene.removeObjects(1);
  calib.clear();
  udist = ImageUndistortion();

  gui["calibrate"].disable();
  gui["save"].disable();

  vecDraw->render();
}

const ImgBase *getSplitImage(const ImgBase *image) {
  const int split = imageSplit.pos;

  // if the undistortion was calculated, draw the undistorted image
  if (!udist.isNull()) {
    if (splitImage) splitImage->clear();
    warp.apply(image, &splitImage);

    const Img8u &src = *image->as8u();
    Img8u &dst = *splitImage->as8u();

    // left part of the image should be the original input
    const icl::icl8u *r = src.begin(0);
    const icl::icl8u *g = src.begin(1);
    const icl::icl8u *b = src.begin(2);
    icl::icl8u *dR = dst.begin(0);
    icl::icl8u *dG = dst.begin(1);
    icl::icl8u *dB = dst.begin(2);
    for (int y = 0; y < src.getHeight(); ++y) {
      std::copy(r, r + split, dR);
      std::copy(g, g + split, dG);
      std::copy(b, b + split, dB);
      r += src.getWidth();
      g += src.getWidth();
      b += src.getWidth();
      dR += src.getWidth();
      dG += src.getWidth();
      dB += src.getWidth();
    }

    return splitImage;
  }
  else return image;
}

// This functions removes all non-unique markers
std::vector<Fiducial> removeDuplicates(const std::vector<Fiducial> &fids) {
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
}

// For every id in "markerIdList" this function creates a list
// of bool values indicating if the marker was found
int createFoundList(const std::vector<Fiducial> &fids, std::vector<bool> &foundList) {
  int validMarkers = 0;
  Size &s = markerInfo.gridSize;
  std::vector<int> &ids = markerInfo.markerIdList;
  std::vector<Fiducial>::const_iterator it = fids.begin();

  foundList = std::vector<bool>(s.width*s.height, false);

  for (int y = 0; y < s.height && it != fids.end(); ++y) {
    for (int x = 0; x < s.width; ++x) {
      int pos = x + y*s.width;

      if (it != fids.end() && ids[pos] == it->getID()) {
        foundList[pos] = true;
        it++;
      }
    }
  }

  return validMarkers;
}

// This functions fills the vector "obj" with object coordinates
// of the marker grid, but only with the found markers
void createObjCoords(const std::vector<Point32f> &grid,
                     const std::vector<bool> &foundList,
                     std::vector<Point32f> &obj) {
  ICLASSERT(grid.size() == 4 * foundList.size());
  obj.clear();

  std::vector<Point32f>::const_iterator gridIt = grid.begin();
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
  }
}

// comparision between two fiducial objects
struct fidComp {
  bool operator() (Fiducial i, Fiducial j) { return (i.getID()<j.getID()); }
} comp;

void handleMarkerDetection(const ImgBase *img, DrawHandle &draw) {
  static ButtonHandle capture = gui["capture"];
  const int minMarkers = gui["minMarkers"];
  std::vector<Fiducial> fids = fid->detect(img);

  // first check if enough markers were found
  if ((int)fids.size() >= minMarkers) {
    std::vector<bool> foundList;

    std::sort(fids.begin(), fids.end(), comp);
    fids = removeDuplicates(fids);

    // second check if there are enough markers after sorting out invalid markers
    if ((int)fids.size() >= minMarkers) {
      const bool autoCapture = gui["autoCapture"];
      const float displacement = gui["captureDis"];
      float diff = -1.f;

      // visualize markers
      for (unsigned int i = 0; i < fids.size(); ++i){
        draw->color(255, 0, 0, 255);
        draw->linestrip(fids[i].getCorners2D());
        draw->color(0, 100, 255, 255);
        draw->text(fids[i].getName(), fids[i].getCenter2D().x, fids[i].getCenter2D().y, 9);
      }

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

        gui["calibrate"].enable();
      }
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
      LensUndistortionCalibrator::Info info = calib.getInfo();
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
    std::sort(idList.begin(), idList.end());

    s = pa("-g").as<Size>();
    if (s.width*s.height != (int)idList.size())
      throw ICLException("the number of markers must be equal to the grid size");

    fid = new FiducialDetector(pa("-m").as<std::string>(),
                               pa("-m", 1).as<std::string>(),
                               ParamList("size", (*pa("-m", 2))));
    calib.init(image->getSize(), LensUndistortionCalibrator::GridDefinition(pa("-g").as<Size>(), pa("-m", 2).as<Size>(), pa("-sp").as<Size>()));

    maxMarkers = s.width*s.height;

    QuadDetector &qd = ((FiducialDetectorPluginForQuads*)fid->getPlugin())->getQuadDetector();
    qd.setConfigurableID("detectionProps");
  } else {
    throw ICLException("other modes than checkerboard and markers "
                       "detection are not yet supported");
  }

  gui << ( VSplit().label("image")
           << Draw().label("input image / undistorted image").handle("image")
         )
      << ( VSplit().label("data")
           << Draw3D().label("recorded data").handle("plot")
           << Draw().label("vector image").handle("vecImage")
         )
      << ( VBox().label("controls").minSize(15,1)
           << CheckBox("detection", true).out("detection").tooltip("if checked the application will try to find\n"
                                                                   "a checkboard inside the image.")
           << CheckBox("auto capture", false).out("autoCapture").tooltip("if checked and the displacement is higher than a threshold,\n"
                                                                          "the current detection will be captured.")
           << FSlider(0.f, 200.f, 10.f).out("captureDis").label("displacement")
           << Button("capture").handle("capture")
           << Button("calibrate").handle("calibrate")
           << Button("save").handle("save")
           << Button("reset").handle("reset")
           << Slider(5, maxMarkers, maxMarkers).hideIf(!fid).out("minMarkers").label("minimum markers").tooltip("minimum number of markers that is needed for the calibration")
           << Prop("detectionProps").label(dStr.c_str())
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
  gui["image"].install(new MouseHandler(splitImageMouse));
  gui["calibrate"].disable();
  gui["save"].disable();

  imageSplit.pos = image->getWidth() / 2;
}

void run(){
  static ButtonHandle reset     = gui["reset"];
  static ButtonHandle calibrate = gui["calibrate"];
  static ButtonHandle save      = gui["save"];
  static DrawHandle draw = gui["image"];
  static DrawHandle vecDraw = gui["vecImage"];
  const bool detection   = gui["detection"];

  // handle reset button
  if (reset.wasTriggered()) {
    resetData();
  }

  const ImgBase *img = grabber.grab();
  draw = getSplitImage(img);

  // get the current configuration of the split line
  imageSplit.mutex.lock();
  int splitPos = imageSplit.pos;
  bool splitDrag = imageSplit.drag;
  bool splitOverlap = imageSplit.overlap;
  imageSplit.mutex.unlock();

  // draw the seperation line between input image and the undistorted image
  draw->color(255, 255, 255, 255);
  if (splitDrag || splitOverlap) draw->linewidth(3);
  draw->line(splitPos, 0, splitPos, img->getHeight() - 1);
  draw->linewidth(1);

  if (detection) {
    if (fid) {
      handleMarkerDetection(img, draw);
    }
    if (!checker.isNull()) {
      handleCheckerboardDetection(img, draw);
    }
  }

  // handle calibrate button
  if(calibrate.wasTriggered()){
    udist = calib.computeUndistortion();
    SHOW(udist);
    gui["save"].enable();

    const Img32f &mapping = udist.createWarpMap();
    warp.setWarpMap(mapping);

    // draw the displacement between the input image and the undistorted image
    for (int y = 0; y < mapping.getHeight(); y += 5)
      for (int x = 0; x < mapping.getWidth(); x += 5) {
        float distX = mapping[0](x, y);
        float distY = mapping[1](x, y);
        vecDraw->arrow(distX, distY, x, y, 1.0f);
      }
    vecDraw->render();
  }

  // handle save button
  if (save.wasTriggered() && !udist.isNull()) {
    save_params();
  }

  draw->render();
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
