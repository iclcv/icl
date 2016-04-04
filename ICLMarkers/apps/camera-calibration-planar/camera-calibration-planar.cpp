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
#include <ICLMarkers/MarkerGridPoseEstimator.h>

//#include <ICLIO/ImageUndistortion.h>
#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>
//#include <ICLGeom/CoplanarPointPoseEstimator.h>

#include "GridIndicatorObject.h"
#include "PlanarCalibrationTools.h"
#include <fstream>

#if 0
VBox gui;
GUI relGUI,poseEstGUI,fidGUI;

typedef AdvancedMarkerGridDetector Detector;
typedef Detector::AdvancedGridDefinition GridDef;
typedef Detector::Marker Marker;
typedef Detector::MarkerGrid MarkerGrid;


Scene scene;


ComplexCoordinateFrameSceneObject *cs = 0;
GridIndicatorObject *gridIndicator;

struct View{
  GenericGrabber grabber;
  Detector detector;
  MarkerGridPoseEstimator poseEst;
  Camera cam;
  ImgBase *lastImage;
};  
typedef SmartPtr<View> ViewPtr;

std::vector<ViewPtr> views;

void init(){
  std::vector<int> ids;
  if(pa("-ids")){
    ids = FiducialDetectorPlugin::parse_list_str(*pa("-ids"));
  }
  GridDef d(pa("-g"),pa("-mb"), pa("-gb"), ids, pa("-m"));

  ProgArg pai = pa("-i");
  views.resize(pai.n()/3);
  std::string inputIDs;
  
  for(int i=0;i<pai.n();i+=3){
    int id = i/3;
    views[id] = new View;
    View &v = *views[id3];
    v.lastImage = 0;
    v.grabber.init(pai[i], pa[i] + "=" + pa[i+1]);
    inputIDs += "input-" + str(id) + ",";
    const ImgBase  *image = v.grabber.grab();
    v.camera = extract_camera_from_udist_file(image->getSize(), pai[i+2]);
    scene.addCamera(v.camera);

    FiducialDetector *fd = detector.getFiducialDetector();
    fd->setPropertyValue("thresh.mask size", 15);
    fd->setPropertyValue("thresh.global threshold", -10);
    fd->setConfigurableID("fd-cam"+str(id));
    v.detector.init(d);
    poseEst.setConfigurableID("poseEst-cam"+str(id));
  }
  inputIDs = inputIDs.substr(0,inputIDs.legth()-1);
  VBox controls;
  controls.label("controls").maxSize(14,99).minSize(14,1);
  constrols << Combo(inputIDs).handle("visinput").label("input index")
            << Combo(fd->getIntermediateImageNames()).handle("visualization").label("visualization").handle("vis")
            << ( HBox()
                 << CamCfg()
                 << CheckBox("image acquition",true).handle("acquisition").tooltip("if checked, new images are grabbed")
                 )
            << ( HBox()
                 << CheckBox("use grid center",pa("-ugc")).handle("cen").tooltip("Use the grid center as world center")
                 << CheckBox("show world CS",true).handle("show CS").tooltip("Show a world coordinate frame")
                 )
            << Button("define relative transform ...").handle("rel")
            << Button("pose estimation options ...").handle("poseEst")
            << Button("fiducial detection options ...").handle("fid")
            << Plot().handle("variancePlot").label("10-frame pose variance plot")
            << Button("save calibration").handle("save");
  
  gui << (HSplit()
          << Draw3D(image->getSize()).handle("draw").minSize(32,24)
          << controls
          )
      << Show();
  
  
  gridIndicator = new GridIndicatorObject(d);
  
  scene.addCamera(scene.getCamera(0));
  scene.addObject(gridIndicator);
  
  //gui["draw"].install(scene.getMouseHandler(1));
  gui["draw"].link(scene.getGLCallback(views.size()));

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

  Tab poseEstTab(inputIDs), fidTag(inputIDs);
  for(size_t i=0;i<views.size();++i){
    fidTab << Prop("fd-cam"+str(id));
    poseEstTab << Prop("poseEst-cam"+str(id));
  }
  
  poseEstGUI << poseEstTab << Create();
  fidGUI << fidTab << Create();


  gui["rel"].registerCallback(utils::function(relGUI,&GUI::switchVisibility));  
  gui["poseEst"].registerCallback(utils::function(poseEstGUI,&GUI::switchVisibility));  
  gui["fid"].registerCallback(utils::function(fidGUI,&GUI::switchVisibility));  
  
  cs = new ComplexCoordinateFrameSceneObject;
  cs->setVisible(false);
  scene.addObject(cs,true);
}

void run(){

  int currentView = gui["visinput"];

  cs->setVisible(gui["show CS"].as<bool>());
  static DrawHandle3D draw = gui["draw"];
  static ButtonHandle save = gui["save"];

  bool acquisition = gui["acquisition"], centeredGrid = gui["cen"];

  static Size32f bounds = pa("-gb");
  static Mat dC = create_hom_4x4<float>(0,0,0, bounds.width/2,bounds.height/2,0);

  Mat dT = centerdGrid ? dC : Mat::id();
  gridIndicator->setTransformation(dT.inv());
  Mat R = create_hom_4x4<float>(relGUI["rx"].as<float>()*M_PI/4,
                                relGUI["ry"].as<float>()*M_PI/4,
                                relGUI["rz"].as<float>()*M_PI/4,
                                relGUI["tx"],relGUI["ty"],relGUI["tz"]);

  for(size_t i=0;i<views.size();++i){
    View &v = *views[i];
    const ImgBase *image = acquisition ? v.lastImage : v.grabber.grab();
    v.lastImage = image;
    
    const MarkerGrid &grid = v.detector.detect(image);
    
    Camera cam = v.cam;
    Mat T = poseEst.computePose(grid, cam);

    dT = dT * R;

    try{
      cam.setWorldFrame(T * dT);
      scene.getCamera(i) = cam;
    }catch(...){
      /// this sometimes happens when the estimated transform is weakly conditioned
    }
    
    /// todo: here, we continue! We do need one Draw3D for each input
  draw = detector.getFiducialDetector()->getIntermediateImage(gui["vis"]);
  draw->draw(grid.vis());

  draw->render();

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
  for(int i=0;i<6;++i){
    bufs[i].push((i>=3?100:1)*vars[i]);
  }
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

int main(int n, char **ppc){
  pa_explain("-g", "marker grid dimension in cells");
  pa_explain("-mb", "width and height of a sigle marker in mm");
  pa_explain("-gb", "width and height of the whole grid (left of \n"
             "left-most marker to the right of the right-most marker \n"
             " and top of the top-most marker to the bottom of the \n"
             " bottom most marker");
  pa_explain("-m", "marker type to use, this should actually not be \n"
            "adapted as the default type 'bch' provides best detection \n"
            "and reliability properties");
  pa_explain("-ids", "marker IDs to use. If not specified, the IDs \n"
             "[0-w*h-1] are used. The string can either be a comma-\n"
             "separated list of entries or a range specification such \n" 
             "'[0-100]'");
  pa_explain("-ugc","if given, the center of the grid initially defines the \n"
             "world frame origin");
  pa_explain("-u", "gives the filename of the undistortion file that is \n"
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
             " [...] -input dc800 0@udist=udist.xml -u udist.xml -m -g [...]");
  pa_explain("-t","gives initial transform paramters (rotation is given in integer units of PI/2)");

  return ICLApp(n, ppc, " [m]-input|-i(2) [m]-grid-cell-dim|-g(cells) "
                "[m]-marker-bounds|-mb(mm) [m]-grid-bounds|-gb(mm) "
                "-marker-type|-m(type=bch) -marker-ids|-ids "
                "-udist-file|-u(filename) -camera-file -use-grid-center|-ugc " 
                "-initial-relative-transform|-t(rx=0,ry=0,rz=0,tx=0,ty=0,tz=0) "
                "-output-filename|-o(filename)",
                init, run).exec();
}




#if 0


    bool saveWasTriggered = save.wasTriggered();

    if(saveWasTriggered){
      std::string filename;
      if(pa("-o")){
        filename = *pa("-o");
      }else{
        try{
          filename = saveFileDialog("XML-Files (*.xml)",
                                    "save calibration file");
        }catch(...){}
      }
      if(filename.length()){
        std::ofstream f(filename.c_str());
        f << cam;
        std::cout << " saved calibration file as " << filename << std::endl;
      }
    }

  }catch(...){

  }
#endif

#endif
int main(){}
