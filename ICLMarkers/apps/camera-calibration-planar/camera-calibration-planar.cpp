/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/camera-calibration-planar/             **
**          camera-calibration-planar.cpp                          **
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
#include <ICLGeom/Geom.h>
#include <ICLGeom/PoseEstimator.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>
#include <ICLMarkers/MarkerGridPoseEstimator.h>
#include <ICLCV/CheckerboardDetector.h>
#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>
#include "GridIndicatorObject.h"
#include "PlanarCalibrationTools.h"
#include <fstream>


VBox gui;
GUI relGUI,poseEstGUI,fidGUI,captureFramesGUI;

typedef AdvancedMarkerGridDetector Detector;
typedef Detector::AdvancedGridDefinition GridDef;
typedef Detector::Marker Marker;
typedef Detector::MarkerGrid MarkerGrid;


Scene scene;


Mat compute_relative_transform_n(const std::vector<Camera> &s, const std::vector<Camera> &d){
  int n = (int)iclMin(s.size(), d.size());
  DynMatrix<float> Rs(3,3*n), Rd(3,3*n);
  Vec3 dT(0,0,0);
  for(int i=0;i<n;++i){
    Mat ms = s[i].getInvCSTransformationMatrix();
    Mat md = d[i].getInvCSTransformationMatrix();
    Vec3 Ts = ms.part<3,0,1,3>();
    Vec3 Td = md.part<3,0,1,3>();
    dT = dT + (Td - Ts);
    for(int r=0;r<3;++r){
      std::copy(ms.row_begin(r), ms.row_begin(r)+3, Rs.row_begin(3*i+r));
      std::copy(md.row_begin(r), md.row_begin(r)+3, Rd.row_begin(3*i+r));
    }
  }
  dT *= 1./n;
  DynMatrix<float> Rrel = Rs.pinv(true) * Rd;
  Mat D(Rrel(0,0), Rrel(1,0), Rrel(2,0), dT[0],
        Rrel(0,1), Rrel(1,1), Rrel(2,1), dT[1],
        Rrel(0,2), Rrel(1,2), Rrel(2,2), dT[2],
        0,0,0,1);

  return D;
}

Mat compute_relative_transform(const Camera &s, const Camera &d){
  Mat ms = s.getInvCSTransformationMatrix();
  Mat md = d.getInvCSTransformationMatrix();
  //    Mat rel = d.getInvCSTransformationMatrix() * s.getCSTransformationMatrix();

  Mat3 Rs = ms.part<0,0,3,3>();
  Mat3 Rd = md.part<0,0,3,3>();

  Vec3 Ts = ms.part<3,0,1,3>();
  Vec3 Td = md.part<3,0,1,3>();

  Mat3 Rrel = Rs.transp() * Rd;
  Vec3 Trel = Td - Ts;

  Mat T = Rrel.resize<4,4>(0);
  T.col(3) = Trel.resize<1,4>(1);

  //Mat T2 = compute_relative_transform_n(std::vector<Camera>(10,s),
  //                                      std::vector<Camera>(10,d));
  return T;
}



ComplexCoordinateFrameSceneObject *cs = 0;
GridIndicatorObject *gridIndicator;

struct View{
  GenericGrabber grabber;
  Detector detector;
  CheckerboardDetector cbDetector;
  MarkerGridPoseEstimator poseEst;
  CoplanarPointPoseEstimator cbPoseEst;
  Camera camera;
  Camera calibratedCamera;
  const ImgBase *lastImage;
  ComplexCoordinateFrameSceneObject *cs;
  std::vector<Camera> capturedFrames;
  View():cbPoseEst(CoplanarPointPoseEstimator::worldFrame,
                   CoplanarPointPoseEstimator::SimplexSampling){}
};
typedef SmartPtr<View> ViewPtr;

std::vector<ViewPtr> views;

struct GLCallback : public ICLDrawWidget3D::GLCallback{
  virtual void draw(ICLDrawWidget3D *w){
    int curr = gui["visinput"];
    scene.getGLCallback(curr)->draw(w);
  }
};

struct CheckerBoardDef{
  bool used;
  Size32f bounds;
  Size cells;
  std::vector<Point32f> pts;
} cbDef = { false, Size32f(0,0), Size(0,0) };

void init(){
  ProgArg po = pa("-o");

  std::vector<int> ids;
  if(pa("-ids")){
    if(!pa("-m")){
      throw ICLException("argument -ids can only be used in combination with -m (for markers)");
    }
    ids = FiducialDetectorPlugin::parse_list_str(*pa("-ids"));
  }
  GridDef d;
  if(pa("-m")){
    d = GridDef(pa("-g"),pa("-mb"), pa("-gb"), ids, pa("-m"));
  }else{
    if(pa("-mb")){
      WARNING_LOG("disregarding program argument -mb, which is only used in the marker-grid mode  (-m ..)");
    }
    cbDef.used = true;
    cbDef.cells = pa("-g").as<Size>();
    cbDef.bounds = pa("-gb").as<Size32f>();
    float dx = cbDef.bounds.width/cbDef.cells.width;
    float dy = cbDef.bounds.height/cbDef.cells.height;
    for(int y=0;y<cbDef.cells.height;++y){
      for(int x=0;x<cbDef.cells.width;++x){
        cbDef.pts.push_back(Point32f(x*dx, y*dy));
      }
    }
  }

  ProgArg pai = pa("-i");
  views.resize(pai.n()/3);

  if(po && po.n() != (int)views.size()){
    throw ICLException("number of -o sub-arguments must be equal to number of sub-arguments for -i devided by 3");
  }

  std::string inputIDs;

  FiducialDetector *fd = 0;
  Size imageSize0;
  if(pai.n() % 3) {
    throw ICLException("invalid sub-argument count to argument -input!"
                       "sub-argument count must be multiple of 3 (found "
                       + str(pai.n())+ ")");
  }
  for(int i=0;i<pai.n();i+=3){
    int id = i/3;
    views[id] = new View;
    View &v = *views[id];
    v.lastImage = 0;
    v.grabber.init(pai[i], pai[i] + "=" + pai[i+1]);
    inputIDs += "input-" + str(id) + ",";
    const ImgBase  *image = v.grabber.grab();
    if(!i) imageSize0 = image->getSize();
    v.camera = extract_camera_from_udist_file(pai[i+2]);
    v.camera.setName("Input: " + pai[i] + " " + pai[i+1]);
    scene.addCamera(v.camera);
    if(cbDef.used){
      v.cbDetector.init(cbDef.cells);
      v.cbDetector.setConfigurableID("cbd-cam"+str(id));
      v.cbPoseEst.setConfigurableID("cbPoseEst-cam"+str(id));

    }else{
      v.detector.init(d);
      fd = v.detector.getFiducialDetector();
      fd->setPropertyValue("thresh.mask size", 15);
      fd->setPropertyValue("thresh.global threshold", -10);
      fd->setConfigurableID("fd-cam"+str(id));
      v.poseEst.setConfigurableID("poseEst-cam"+str(id));
    }
    v.cs = new ComplexCoordinateFrameSceneObject(10,1);
    scene.addObject(v.cs);
  }
  inputIDs = inputIDs.substr(0,inputIDs.length()-1);
  VBox controls;
  controls.label("controls").maxSize(17,99).minSize(17,1);
  controls << Combo(inputIDs).handle("visinput").label("input index");
  if(fd){
    controls << Combo(fd->getIntermediateImageNames()).handle("visualization").label("visualization").handle("vis");
  }
  controls << ( HBox()
                << CamCfg()
                << CheckBox("image acquition",true).handle("acquisition").tooltip("if checked, new images are grabbed")
                )
           << ( HBox()
                << CheckBox("use grid center",pa("-ugc")).handle("cen").tooltip("Use the grid center as world center")
                << CheckBox("show world CS",true).handle("show CS").tooltip("Show a world coordinate frame")
                )
           << CheckBox("show grid CSs",true).handle("show grid CSs").tooltip("Show a coordinate frame attached to a grid")

           << Button("define relative transform ...").handle("rel")
           << Button("pose estimation options ...").handle("poseEst")
           << Button("detection options ...").handle("fid")
           << (HBox()
               << Button("accumulate frames ...").handle("cap").tooltip("allows a set of frames to be accumulated to get a better relative result")
               << Label("0").handle("ncap").maxSize(3,2).tooltip("Number of currenly captured frames. <b>Please note:</b> if this number is larger than 0, the internally captured frames are used for calibration (and not the current frame)")
               )
           << Plot().handle("variancePlot").label("10-frame pose std-deviation")
           << Button("save calibration").handle("save").tooltip("saves the calibration file of the current view's camera")
           << Button("save relative calibration").handle("saveRel").tooltip("saves the calibration file of the current view's "
                                                                            "camera <b>and</b> the relative calibrations of all "
                                                                            "other views wrt. the current view camera").hideIf(pai.n() < 6);

  gui << (HSplit()
          << (Tab("input view,3D scene view")
              << Draw3D(imageSize0).handle("draw").minSize(32,24)
              << (VBox()
                  << (HBox().maxSize(99,2)
                      << Button("sync cam").handle("sync")
                      << CheckBox("visualize cameras",false).handle("vis cams")
                      << CheckBox("show 10mm camera coordinate frames",false).handle("show ccs")
                      )
                  << Draw3D(imageSize0).handle("3D").minSize(32,24)
                  )
             )
          << controls
          )
      << Show();


  scene.addCamera(scene.getCamera(0));

  if(!cbDef.used){
    gridIndicator = new GridIndicatorObject(cbDef.cells, cbDef.bounds);
  }else{
    gridIndicator = new GridIndicatorObject(d);
  }
  scene.addObject(gridIndicator);

  //gui["draw"].install(scene.getMouseHandler(1));
  gui["draw"].link(new GLCallback);//scene.getGLCallback(views.size()));

  gui["3D"].link(scene.getGLCallback(views.size()));
  gui["3D"].install(scene.getMouseHandler(views.size()));

  static PlotHandle plot = gui["variancePlot"];
  plot->setPropertyValue("tics.x-distance",10);
  plot->setPropertyValue("tics.y-distance",0.00001);
  plot->setPropertyValue("labels.x-precision",0);
  plot->setPropertyValue("labels.y-precision",6);
  plot->setPropertyValue("borders.left", 60);
  plot->setPropertyValue("borders.bottom", 60);
  plot->setPropertyValue("legend.y", -34);
  plot->setPropertyValue("legend.width", -44);
  plot->setPropertyValue("legend.x", 1);
  plot->setPropertyValue("legend.height", 30);

  relGUI << ( VBox().label("rel-transformation")
           << ( HBox()
                << Spinner(0,8,pa("-t",0)).label("x-rotation *pi/4").out("rx")
                << Spinner(0,8,pa("-t",1)).label("y-rotation *pi/4").out("ry")
                << Spinner(0,8,pa("-t",2)).label("z-rotation *pi/4").out("rz")
                )
           << ( HBox()
                << Float(-100000,100000,pa("-t",3)).label("x-offset").out("tx")
                << Float(-100000,100000,pa("-t",4)).label("y-offset").out("ty")
                << Float(-100000,100000,pa("-t",5)).label("z-offset").out("tz")
                )
           )
      << Create();

  Tab poseEstTab(inputIDs), fidTab(inputIDs);
  for(size_t i=0;i<views.size();++i){
    if(cbDef.used){
      fidTab << Prop("cbd-cam"+str(i));
      poseEstTab << Prop("cbPoseEst-cam"+str(i));
    }else{
      fidTab << Prop("fd-cam"+str(i));
      poseEstTab << Prop("poseEst-cam"+str(i));
    }

  }

  poseEstGUI << poseEstTab << Create();
  fidGUI << fidTab << Create();


  gui["rel"].registerCallback(utils::function(relGUI,&GUI::switchVisibility));
  gui["poseEst"].registerCallback(utils::function(poseEstGUI,&GUI::switchVisibility));
  gui["fid"].registerCallback(utils::function(fidGUI,&GUI::switchVisibility));

  cs = new ComplexCoordinateFrameSceneObject;
  cs->setVisible(false);
  scene.addObject(cs,true);


  captureFramesGUI << Button("capture current frame").handle("capture")
                   << Label("--").label("num captured frames").handle("n")
                   << (HBox()
                       << Button("undo last").handle("undo")
                       << Button("reset").handle("reset")
                       )
                   << Create();

  gui["cap"].registerCallback(utils::function(captureFramesGUI,&GUI::switchVisibility));
}

void run(){

  bool sgcss = gui["show grid CSs"];
  for(size_t i=0 ;i < views.size();++i){
    views[i]->cs->setVisible(sgcss);
  }

  int currentView = gui["visinput"];
  static int lastView = currentView;

  cs->setVisible(gui["show CS"].as<bool>());
  static DrawHandle3D draw = gui["draw"];

  bool acquisition = gui["acquisition"], centeredGrid = gui["cen"];

  static Size32f bounds = pa("-gb");
  static Mat dC = create_hom_4x4<float>(0,0,0, bounds.width/2,bounds.height/2,0);

  Mat dT = centeredGrid ? dC : Mat::id();
  gridIndicator->setTransformation(dT.inv());
  Mat R = create_hom_4x4<float>(relGUI["rx"].as<float>()*M_PI/4,
                                relGUI["ry"].as<float>()*M_PI/4,
                                relGUI["rz"].as<float>()*M_PI/4,
                                relGUI["tx"],relGUI["ty"],relGUI["tz"]);
  dT = dT * R;

  for(size_t i=0;i<views.size();++i){
    View &v = *views[i];
    const ImgBase *image = !acquisition ? v.lastImage : v.grabber.grab();
    v.lastImage = image;

    Camera cam = v.camera;
    Mat T;
    const MarkerGrid *grid = 0;
    const CheckerboardDetector::Checkerboard *cb = 0;
    if(cbDef.used){
      cb = &v.cbDetector.detect(image);
      T = v.cbPoseEst.getPose(cbDef.pts.size(),
                              cbDef.pts.data(),
                              cb->corners.data(), cam);
    }else{
      grid = &v.detector.detect(image);
      T = v.poseEst.computePose(*grid, cam);
    }

    try{
      cam.setWorldFrame(T * dT);
      scene.getCamera(i) = cam;
      v.calibratedCamera = cam;
      v.cs->setVisible(gui["show ccs"].as<bool>());
      v.cs->setTransformation(cam.getInvCSTransformationMatrix());
    }catch(...){
      /// this sometimes happens when the estimated transform is weakly conditioned
    }

    if((int)i == currentView){
      //try{
      //  cs->setTransformation(T.inv());
      //}catch(...){}
      /// todo: here, we continue! We do need one Draw3D for each input
      if(cbDef.used){
        draw = image;
        draw->draw(cb->visualize());
      }else{
        draw = v.detector.getFiducialDetector()->getIntermediateImage(gui["vis"]);
        draw->draw(grid->vis());
      }

      if(lastView!=currentView){
        for(int i=0;i<10;++i) estimate_pose_variance(T);
      }

      std::vector<float> vars = estimate_pose_variance(T);
      static const int n = 100;
      static PlotWidget::SeriesBuffer bufs[6] = {
        PlotWidget::SeriesBuffer(n),
        PlotWidget::SeriesBuffer(n),
        PlotWidget::SeriesBuffer(n),
        PlotWidget::SeriesBuffer(n),
        PlotWidget::SeriesBuffer(n),
        PlotWidget::SeriesBuffer(n)
      };
      for(int j=0;j<6;++j){
        float val = (j>=3?100:1)*sqrt(vars[j]);
        if(lastView!=currentView){
          std::fill(bufs[j].begin(), bufs[j].end(), 0);
        }
        bufs[j].push(val);
      }
      lastView = currentView;

      static PlotHandle plot = gui["variancePlot"];

      static const int cs[6][4] = {
        { 255, 0, 0, 255 },
        { 0, 255 ,0, 255 },
        { 0, 0, 255, 255 },
        { 255, 255, 0, 255 },
        { 255, 0, 255, 255},
        { 0, 255, 255, 255 }
      };

      static std::string labels[6] = {
        "x", "y", "z", "rx", "ry", "rz"
      };

      plot->lock();
      plot->reset();
      for(int i=0;i<6;++i){
        plot->color(cs[i]);
        plot->label((i>=3 ? "100 x " : "") + str("var(" + labels[i] + ")"));
        plot->series(bufs[i]);
      }

      plot->unlock();
      plot->render();
    }
  }
  draw->render();

  static ButtonHandle captureFrame = captureFramesGUI["capture"];
  static ButtonHandle captureUndo = captureFramesGUI["undo"];
  static ButtonHandle captureReset = captureFramesGUI["reset"];

  bool cf = captureFrame.wasTriggered(), cu = captureUndo.wasTriggered(), cr = captureReset.wasTriggered();
  for(size_t i=0;i<views.size();++i){
    if(cf){
      views[i]->capturedFrames.push_back(views[i]->calibratedCamera);
    }else if(cu){
      if(views[i]->capturedFrames.size()){
        views[i]->capturedFrames.pop_back();
      }
    }else if(cr){
      views[i]->capturedFrames.clear();
    }
  }
  std::string nCap = str(views[0]->capturedFrames.size());
  captureFramesGUI["n"] = nCap;
  gui["ncap"] = nCap;


  std::string names[]={"save", "saveRel"};
  for(int i=0;i<2;++i){
    if(i && views.size() < 2) break;
    ProgArg p = pa("-o");
    ButtonHandle save = gui[names[i]];

    bool saveWasTriggered = save.wasTriggered();

    if(saveWasTriggered){
      std::string filename;
      if(pa("-o")){
        filename = *pa("-o",currentView);
      }else{
        try{
          filename = saveFileDialog("XML-Files (*.xml)",
                                    "save current view's calibration file");
        }catch(...){}
      }
      if(filename.length()){
        std::ofstream f(filename.c_str());
        f << views[currentView]->calibratedCamera;
        std::cout << " saved current view's calibration file as " << filename << std::endl;
      }
      if(i){
        // do the relative calibration aswell
        for(size_t j=0;j<views.size();++j){
          if((int)j == currentView) continue;
          std::string filename;
          if(pa("-o")){
            filename = p[j];
          }else{
            try{
              filename = saveFileDialog("DAT-Files (*.dat)",
                                        "save relative calibration files for view " + str(j));
            }catch(...){}
          }
          if(filename.length()){
            Mat rel;
            if(views[currentView]->capturedFrames.size()){
              rel = compute_relative_transform_n(views[currentView]->capturedFrames,
                                                 views[j]->capturedFrames);
            }else{
              rel = compute_relative_transform(views[currentView]->calibratedCamera,
                                               views[j]->calibratedCamera);
            }
            std::ofstream f(filename.c_str());
            f << rel << std::endl;
            std::cout << " saved relative calibration of view " << j << " to file " << filename << std::endl;
            size_t n = views[currentView]->capturedFrames.size();
            if(n){
              std::cout << " (relative calibration was performed on " << n << " captured relative transform tuples)" << std::endl;
            }else{
              std::cout << " (relative calibration was performed the single current frame)" << std::endl;
            }
          }
        }
      }
    }

  }

  scene.setPropertyValue("visualize cameras", gui["vis cams"].as<bool>());

  static ButtonHandle sync = gui["sync"];
  if(sync.wasTriggered()){
    scene.getCamera(views.size()) = scene.getCamera(0);
  }
  gui["3D"].render();
}

int main(int n, char **ppc){
  pa_explain("-g", "marker grid dimension in cells");
  pa_explain("-mb", "width and height of a sigle marker in mm");
  pa_explain("-gb", "width and height of the whole grid (left of \n"
             "left-most marker to the right of the right-most marker \n"
             " and top of the top-most marker to the bottom of the \n"
             " bottom most marker");
  pa_explain("-m", "marker type to use, this should actually not be \n"
            "adapted as the default type 'bch' provides best detection \n"
            "and reliability properties. If -m is not given, a simple \n"
             " checkerboard is tried to be detected.");
  pa_explain("-ids", "marker IDs to use. If not specified, the IDs \n"
             "[0-w*h-1] are used. The string can either be a comma-\n"
             "separated list of entries or a range specification such \n"
             "'[0-100]'");
  pa_explain("-ugc","if given, the center of the grid initially defines the \n"
             "world frame origin");
  pa_explain("-i", "Defines a number of cameras that are supposed to be\n"
             "calibrated at once. The variable argument count must be a\n"
             "multiple of 3. The argument order is\n"
             "input-type-0 input-id-0 udist-file-0 input-type-1 ...\n"
             "The given udist-filenames for the particular inputs are \n"
             "used to specifiy the intrinsic camera paramters. Here, we \n"
             "assume that initially 'icl-lense-undistortion-calibration-opencv'\n"
             "is used to generate an undistortion parameter file that contains \n"
             "not only the undistortion parameters, but also the estimated \n"
             "horizontal and vertical focal length and principal point offset of \n"
             "the camera. Please note that the undistortion file is not \n"
             "automatically also used for the input image undistortion. \n"
             "Therefore a lens-undistortion file (e.g. udist.xml) usually \n"
             "has to be provided twice, once for the image undistortion \n"
             "and once for the extraction of the intrinsic camera parameters (e.g. \n"
             " [...] -input dc800 0@udist=udist.xml udist.xml -m -g [...]");
  pa_explain("-t","gives initial transform paramters (rotation is given in integer units of PI/2)");

  return ICLApp(n, ppc, " [m]-input|-i(...) [m]-grid-cell-dim|-g(cells) "
                "-marker-bounds|-mb(mm) [m]-grid-bounds|-gb(mm) "
                "-marker-type|-m(type=bch) -marker-ids|-ids(idlist) "
                "-camera-file -use-grid-center|-ugc "
                "-initial-relative-transform|-t(rx=0,ry=0,rz=0,tx=0,ty=0,tz=0) "
                "-output-filenames|-o(...)",
                init, run).exec();
}
