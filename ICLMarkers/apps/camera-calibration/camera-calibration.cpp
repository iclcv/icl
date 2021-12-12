/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/camera-calibration/camera-calibration. **
**          cpp                                                    **
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

#include "CameraCalibrationUtils.h"

#include <ICLQt/Common.h>
#include <ICLGeom/Scene.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLQt/AdjustGridMouseHandler.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>

#include <QMessageBox>
#include <fstream>
#include <ICLUtils/ConfigFile.h>

typedef CameraCalibrationUtils CCU;
typedef CCU::PossibleMarker PossibleMarker;
typedef CCU::MarkerType MarkerType;
typedef CCU::FoundMarker FoundMarker;

HSplit gui;
GUI relTransGUI;
GUI markerDetectionOptionGUI;
GUI planeOptionGUI;

Scene scene;
GenericGrabber grabber;
Point32f currentMousePos;
bool haveAnyCalibration = false;
CCU::CalibFileData calibFileData;


AdjustGridMouseHandler gridAdjuster;

SmartPtr<Camera> knownIntrinsicCameraParams;


void mouse(const MouseEvent &e){
  currentMousePos = e.getPos();
  if(gui["manual mode"]){
    gridAdjuster.process(e);
  }
}

CCU::BestOfNSaver *bestOfNSaver = 0;

void save(){
  bestOfNSaver->lock();
  Camera cam = scene.getCamera(0);
  bestOfNSaver->unlock();

  CCU::save_cam_pa(cam,"-os","-o");
}

int get_n_frames(){
  return gui["save_num_frames"];
}

void change_plane(const std::string &handle){
  CCU::change_plane(handle, planeOptionGUI, scene, calibFileData);
}

struct IntrinsicParams {
  std::string id;
  Size size;
  float fx, fy, px, py, skew;
};

void init(){
  bool pif = pa("-if"), pip = pa("-ip"), pis = pa("-is");
  if(!!pif + !! pip + !! pis > 1){
    throw ICLException("The program arguments -if -ip and -is are exclusive");
  }
  if(pif){
    knownIntrinsicCameraParams = SmartPtr<Camera>(new Camera(Camera::create_camera_from_calibration_or_udist_file(*pa("-if"))));
  }else if(pip){
    Camera c;
    c.setFocalLength(1);
    c.setSamplingResolutionX(pa("-ip",0));
    c.setSamplingResolutionY(pa("-ip",1));
    c.setPrincipalPointOffset(pa("-ip",2),pa("-ip",3));
    c.setSkew(pa("-ip",4));
    c.setResolution(Size::null); // this means, we assume the target resolution
    knownIntrinsicCameraParams = SmartPtr<Camera>(new Camera(c));
  }else if(pis){
    static const int N = 3;
    // specs were taken from
    // 3D with Kinect
    // Jan Smisek, Michal Jancosek and Tomas Pajdla
    // CMP, Dept. of Cybernetics, FEE, Czech Technical University in Prague
    // http://cmp.felk.cvut.cz/ftp/articles/pajdla/Smisek-CDC4CV-2011.pdf
    static IntrinsicParams ips[N] = {
      { str("kinecti"), Size::VGA,  585, 585, 316, 247.6, 0 },
      { str("kinectd"), Size::VGA,  585, 585, 316, 247.6, 0 },
      { str("kinectc"), Size::VGA,  524, 524, 316.7, 238.5, 0 }
    };
    std::string s = pa("-is");
    if(s == "list"){
      std::cout << "List of well known camera specs: " << std::endl;
      for(int i=0;i<N;++i){
        IntrinsicParams &p = ips[i];
        std::cout << p.id << ": reference size: " << p.size << " fx: " << p.fx << " fy:" << p.fy
                  << " px:" << p.px  << "py:" << p.py << " skew: " << p.skew << std::endl;
      }
      throw ICLException("aborted after listing known camera specs");
    }else{
      for(int i=0;i<N;++i){
        IntrinsicParams &p = ips[i];
        if(p.id == s){
          Camera c;
          c.setFocalLength(1);
          c.setSamplingResolutionX(p.fx);
          c.setSamplingResolutionY(p.fy);
          c.setPrincipalPointOffset(p.px, p.py);
          c.setSkew(p.skew);
          c.setResolution(p.size);
          knownIntrinsicCameraParams = SmartPtr<Camera>(new Camera(c));
          break;
        }
      }
      if(!knownIntrinsicCameraParams){
        std::cout << "List of well known camera specs: " << std::endl;
        for(int i=0;i<N;++i){
          IntrinsicParams &p = ips[i];
          std::cout << p.id << ": reference size: " << p.size << " fx: " << p.fx << " fy:" << p.fy
                    << " px:" << p.px  << "py:" << p.py << " skew: " << p.skew << std::endl;
        }
        throw ICLException("Error: could not find a known camera spec with name '" + s + "'");
      }
    }
  }

  bestOfNSaver = new CCU::BestOfNSaver(get_n_frames);

  if( !pa("-c") || !pa("-c").n() ){
    pa_show_usage("program argument -c must be given with at least one sub-argument");
    ::exit(0);
  }

  for(int c = 0; c <pa("-c").n(); ++c){
    try{
      CCU::CalibFile cf = CCU::parse_calib_file(*pa("-c",c),c,calibFileData);
      calibFileData.loadedFiles.push_back(cf);
      scene.addObject(cf.obj);
    }catch(std::runtime_error &e){
      ERROR_LOG("Error parsing calibration object file " + *pa("-c",c) + ": '" + str(e.what()) + "'");
    }
  }

  grabber.init(pa("-i"));
  if(pa("-s")) grabber.useDesired(pa("-s").as<Size>());

  gui << Draw3D().handle("draw").minSize(32,24);

  markerDetectionOptionGUI = Tab(cat(calibFileData.configurables,","));

  for(unsigned int i=0;i<calibFileData.configurables.size();++i){
    markerDetectionOptionGUI << Prop(calibFileData.configurables[i]);
  }

  gui << ( VBox().minSize(14,28).maxSize(14,100)
           << (VBox().label("visualization").minSize(1,5)
               << ( HBox().maxSize(100,3).minSize(1,3)
                    << Combo(calibFileData.iin).handle("iin").label("visualized image")
                    << Slider(0,255,128).out("objAlpha").label("object-alpha")
                    )
               << ( HBox().maxSize(100,3)
                    << CheckBox("use corners",true).out("useCorners")
                    << CheckBox("show CS",false).out("showCS")
                    )
               )
           << ( VBox().label("more options").minSize(1,6).maxSize(100,6)
                << ( HBox().maxSize(99,2)
                     << Button("plane").handle("show-plane-options")
                     << Button("markers").handle("show-marker-detection-options")
                     )
                << ( HBox().maxSize(99,2)
                     << Button("rel. Transf.").handle("showRelTransGUI")
                     << CamCfg()
                     )
                << CheckBox("manually define marker grids").handle("manual mode")
                )
           << (VScroll().label("calibration objects")
               << calibFileData.objGUI
               )
           << (HBox().maxSize(100,4)
               << ( VBox()
                    << CheckBox("nomalized error",true).out("errNormalized").tooltip("if checked, the total calibration error\n"
                                                                                     "is not devided by the number of calibration points N,\n"
                                                                                     "but by N^2 in order to avoid favoring frames where\n"
                                                                                     "only few markers were found.")

                    << Label().handle("error").label("error").tooltip("The error is given by the mean square distance\n"
                                                                      "of the actually detected points and the points\n"
                                                                      "that are projected into the scene using the\n"
                                                                      "current camera calibration result\n"
                                                                      "If 'normalized error' is checked, the sum of the\n"
                                                                      "is not normalized by the number of points N\n"
                                                                      "but by N^2. To make the error comparable, it is\n"
                                                                      "also mutiplied by 100 in this case.")
                    )
               << (VBox()
                   << Combo("default,!extr").label("opt. mode").
                      handle("extr").hideIf(!knownIntrinsicCameraParams).tooltip("in case you passed the program argument "
                                                                                 "<i>\"-intr FILENAME\"</i> to the "
                                                                                 "icl-camera-calibration application, "
                                                                                 "you can here define whether to perform "
                                                                                 "the default joint in/extrinsic calibration or "
                                                                                 "wheter to use the given intrinsic parameters "
                                                                                 "to obtain more optimized extrinsic "
                                                                                 "calibration results. The lma mode will add an "
                                                                                 "additional levenberg-marquardt base "
                                                                                 "optimization step.")
                   << CheckBox("use lma",true).handle("lma").tooltip("perform 2nd tier LMA-based optrimization to minimize projection error.")
                   )
               )
           << Label("ready..").minSize(1,2).maxSize(100,2).label("detection status").handle("status")
           << ( VBox().maxSize(100,6).minSize(1,6)
                << ( HBox()
                     << Spinner(1,10000,10).out("save_num_frames").label("# frames").tooltip("When you press save, "
                                                                                             "the system will\ncapture "
                                                                                             "that many frames to find\nan "
                                                                                             "optimal calibration result")
                     << Button("save").handle("save")
                     << Button("stop").handle("save_stop")
                     )
                << ( HBox()
                     << Label("10").handle("save_remaining_frames").label("remaining")
                     << Label().handle("save_best_error").label("best error")
                   )
               )
           )
      << Show();

  planeOptionGUI << ( HBox()
                      << Combo("none,x,y,z").label("normal").handle("planeDim")
                      << Float(-10000,10000,0).label("offset mm").handle("planeOffset")
                      )
                 << ( HBox()
                      << Combo("100,200,500,!1000,2000,3000,5000,10000").label("radius mm").handle("planeRadius")
                      << Float(1,1000,10).label("tic distance mm").handle("planeTicDist")
                      )
                 << ( HBox()
                      << Label().handle("planeStatus").label("status")
                      << ColorSelect(40,40,40,255).handle("planeColor").label("color")
                      )
                 << Create();


  relTransGUI << ( VBox().label("rel-transformation")
                   << ( HBox()
                        << Spinner(0,8,0).label("x-rotation *pi/4").out("rx")
                        << Spinner(0,8,0).label("y-rotation *pi/4").out("ry")
                        << Spinner(0,8,0).label("z-rotation *pi/4").out("rz")
                        )
                   << ( HBox()
                        << Float(-100000,100000,0).label("x-offset").out("tx")
                        << Float(-100000,100000,0).label("y-offset").out("ty")
                        << Float(-100000,100000,0).label("z-offset").out("tz")
                        )
                   )
              << Button("show transformation matrix").handle("showRelTrans")
              << Create();


  markerDetectionOptionGUI.create();
  gui["show-marker-detection-options"].registerCallback(utils::function(&markerDetectionOptionGUI,&GUI::switchVisibility));
  gui["show-plane-options"].registerCallback(utils::function(&planeOptionGUI, &GUI::switchVisibility));
  gui["save"].registerCallback(utils::function(bestOfNSaver, &CCU::BestOfNSaver::init));
  gui["save_stop"].registerCallback(utils::function(bestOfNSaver, &CCU::BestOfNSaver::stop));
  gui["showRelTransGUI"].registerCallback(utils::function(&relTransGUI, &GUI::switchVisibility));

  scene.addCamera(Camera());
  scene.getCamera(0).setResolution(grabber.grab()->getSize());

  planeOptionGUI["planeOffset"].disable();
  planeOptionGUI["planeRadius"].disable();
  planeOptionGUI["planeTicDist"].disable();
  planeOptionGUI["planeColor"].disable();
  planeOptionGUI.registerCallback(change_plane,"planeOffset,planeRadius,planeTicDist,planeDim,planeColor");

  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(new MouseHandler(mouse));
}


inline float len_vec(const Vec3 &v){
  return ::sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
}

inline Vec3 normalize_vec(const Vec3 &v){
  float len = len_vec(v);
  if(!len) return v;
  return v * 1./len;
}



void run(){
  scene.lock();
  scene.setDrawCoordinateFrameEnabled(gui["showCS"]);
  scene.unlock();

  const Mat Trel = create_hom_4x4<float>(relTransGUI["rx"].as<float>()*M_PI/4,
                                         relTransGUI["ry"].as<float>()*M_PI/4,
                                         relTransGUI["rz"].as<float>()*M_PI/4,
                                         relTransGUI["tx"],relTransGUI["ty"],relTransGUI["tz"]);

  std::vector<Mat> Ts(calibFileData.loadedFiles.size());
  std::vector<bool> enabled(Ts.size());

  for(unsigned int i=0;i<calibFileData.loadedFiles.size();++i){
    int tidx = gui.get<ComboHandle>("transform-obj-"+str(i)).getSelectedIndex();
    Ts[i] = calibFileData.loadedFiles[i].transforms[tidx].transform;
    enabled[i] = gui.get<bool>("enable-obj-"+str(i));

    SceneObject *calibObj = calibFileData.loadedFiles[i].obj;
    if(!calibObj) continue;
    calibObj->setTransformation(Trel * Ts[i]);
    const int a = gui["objAlpha"];
    if(a){
      const int r = enabled[i] ? 0 : 100, g = 100, b = enabled[i] ? 255 : 100;
      calibObj->setVisible(Primitive::quad,true);
      calibObj->setVisible(Primitive::triangle,true);
      calibObj->setVisible(Primitive::polygon,true);
      calibObj->setColor(Primitive::quad,GeomColor(r,g,b,a));
      calibObj->setColor(Primitive::triangle,GeomColor(r,g,b,a));
      calibObj->setColor(Primitive::polygon,GeomColor(r,g,b,a));
    }else{
      calibObj->setVisible(Primitive::quad,false);
      calibObj->setVisible(Primitive::triangle,false);
      calibObj->setVisible(Primitive::polygon,false);
    }
    if(!enabled[i]){
      calibObj->setColor(Primitive::line,GeomColor(200,200,200,a));
    }else{
      calibObj->setColor(Primitive::line,GeomColor(255,0,0,a));
    }
  }

  const ImgBase *image = CCU::preprocess(grabber.grab());

  std::vector<FoundMarker> markers;

  bool manualMode = gui["manual mode"];
  static bool lastManualMode = manualMode;

  /// one list of grids per loaded file
  static std::vector<std::vector<CCU::DetectedGrid> > detectedGrids(calibFileData.loadedFiles.size());
  static const Rect bounds = image->getImageRect().enlarged(100);
  if(manualMode != lastManualMode){
    if(!manualMode){
      gridAdjuster.clear();
      lastManualMode = manualMode;
    }else{
      markers = CCU::detect_markers(image,calibFileData);
      lastManualMode = manualMode;

      detectedGrids.clear();
      detectedGrids.resize(calibFileData.loadedFiles.size());

      /// associate detected markers to registered grids
      for(size_t i=0;i<markers.size();++i){
        const CCU::FoundMarker &m = markers[i];
        const CCU::PossibleMarker &p = *m.possible;
        if(p.gridIdx == -1) continue;
        const CCU::CalibFile &cf = calibFileData.loadedFiles[p.cfgFileIndex];
        const CCU::MarkerGrid &g = cf.grids[p.gridIdx];

        std::vector<CCU::DetectedGrid> &dgs = detectedGrids[p.cfgFileIndex];
        if(!dgs.size()) {
          dgs.resize(cf.grids.size());
        }
        CCU::DetectedGrid &dg = dgs[p.gridIdx];
        if(!dg){
          dg.setup(&g);
        }
        int mID = m.fid.getID();
        int gridIDIdxPos = g.getCellIdx(mID);
        if(gridIDIdxPos == -1){
          ERROR_LOG("found invalid grid idx position for markerID " << mID);
          continue;
        }
        dg.foundMarkers[gridIDIdxPos] = &m;
      }

      std::vector<std::vector<Point> > manuallyAdjustableGrids;

      std::vector<std::vector<core::Line32f> > textureLines;
      std::vector<Size32f> sizes;

      for(size_t i=0;i<detectedGrids.size();++i){
        //        std::cout << "Detected grids for CfgFile " << i << ":" << std::endl;
        for(size_t j=0;j<detectedGrids[i].size();++j){
          CCU::DetectedGrid &g = detectedGrids[i][j];
          if(!g){
            g.setup(&calibFileData.loadedFiles[i].grids[j]);
          }
          manuallyAdjustableGrids.resize(manuallyAdjustableGrids.size()+1);
          textureLines.resize(textureLines.size()+1);
          sizes.resize(sizes.size()+1);
          g.getGridCornersAndTexture(scene.getCamera(0), manuallyAdjustableGrids.back(),
                                     textureLines.back(), sizes.back(), bounds);
        }
      }
      try{
        gridAdjuster.init(bounds, manuallyAdjustableGrids);
        for(size_t i=0;i<sizes.size();++i){
          gridAdjuster.defineGridTexture(i, sizes[i], textureLines[i]);
        }
      }catch(std::runtime_error &e){
        SHOW(e.what());
      }
    }
  }

  if(manualMode){
    markers.clear();
    /// find marker geometry and use gridAdjuster.mapPoints for the mapping of the marker's points
    int gridIdx = -1;
    for(size_t ci=0;ci<detectedGrids.size();++ci){
      //      std::cout << "Detected grids for CfgFile " << ci << ":" << std::endl;
      for(size_t j=0;j<detectedGrids[ci].size();++j){
        const CCU::DetectedGrid &g = detectedGrids[ci][j];
        if(g){ // found that grid!
          ++gridIdx;
          const CCU::MarkerGrid &realGrid = *g.realGrid;
          //Vec3 dxn = normalize_vec(realGrid.dx) * realGrid.markerSize.width;
          //Vec3 dyn = normalize_vec(realGrid.dy) * realGrid.markerSize.height;

          // in grid space
          float dx = len_vec(realGrid.dx);
          float dy = len_vec(realGrid.dy);
          float rx = realGrid.markerSize.width/2;
          float ry = realGrid.markerSize.height/2;

          for(int y=0;y<realGrid.dim.height;++y){
            for(int x=0;x<realGrid.dim.width;++x){
              math::Vec3 center = realGrid.offset + realGrid.dx * x  + realGrid.dy * y;

              // corners are actually not needed because all points along the grid are
              // linearly dependend and therefore more points on the grid make the solution
              // only more fragile (they didnt' work anyways :-)
              //
              /*math::Vec3 corners[4] = {
                center + dxn - dyn,
                center + dxn + dyn,
                center - dxn + dyn,
                center - dxn - dyn
              };*/

              Point32f centerGridSpace(x * dx + rx, y * dy + ry);
              /*Point32f cornersGridSpace[4] = {
                centerGridSpace + Point32f(dx,-dy),
                centerGridSpace + Point32f(dx,dy),
                centerGridSpace + Point32f(-dx,dy),
                centerGridSpace + Point32f(-dx,-dy)
                  };


              std::vector<Point32f> ps(5);
              ps[0] = centerGridSpace;
              for(int i=0;i<4;++i){
                ps[i+1] = cornersGridSpace[i];
                  }
                  */
              std::vector<Point32f> ps;
              try{
                 ps = gridAdjuster.mapPoints(gridIdx, std::vector<Point32f>(1,centerGridSpace));
              }catch(...){
                y = realGrid.dim.height;
                break;
              }

              CCU::FoundMarker found;
              found.possible = 0;
              found.type = realGrid.type;
              found.id = realGrid.markerIDs.at(x + realGrid.dim.width * y);
              //markers::Fiducial fid;
              found.imagePos = ps[0];
              found.worldPos = center.resize<1,4>(1);
              found.hasCorners = false; // true
              /*
              for(int i=0;i<4;++i){
                found.imageCornerPositions[i] = ps[i+1];
                found.worldCornerPositions[i] = corners[i].resize<1,4>(1);
                  }*/
              found.cfgFileIndex = ci;
              // todo setup found!
              markers.push_back(found);
            }
          }
        }
      }
    }
  }else{
    markers = CCU::detect_markers(image,calibFileData);
  }

  ButtonHandle showRelTrans = relTransGUI["showRelTrans"];
  if(showRelTrans.wasTriggered()){
    std::cout << "current relative transformation is:" << std::endl << Trel << std::endl;
    for(unsigned int i=0;i<calibFileData.loadedFiles.size();++i){
      std::cout << " * combined transformation matrix for calibration object '" << pa("-c",i)
                << "' is:" << std::endl << (Trel * Ts[i]) << std::endl;
    }
  }

  bool useLMA = gui["lma"];
  bool deactivatedCenters = false;

  Camera *intr = 0;
  if(knownIntrinsicCameraParams){
    Camera &c = *knownIntrinsicCameraParams;
    Size cr = c.getResolution();
    Size ir = image->getSize();
    if(cr != Size::null && cr != ir){
      float fx = float(ir.width)/float(cr.width);
      float fy = float(ir.width)/float(cr.width);
      c.setFocalLength(1);
      c.setSamplingResolutionX(c.getSamplingResolutionX() * fx);
      c.setSamplingResolutionY(c.getSamplingResolutionY() * fy);
      // c.setSkew(c.getSkew() * fx); fx or fy? we simply dont scale it!
      c.setResolution(ir, Point32f(c.getPrincipalPointOffsetX() * fx,
                                   c.getPrincipalPointOffsetY() * fy));
    }
    if(gui["extr"].as<std::string>() != "default"){
      intr = &c;
    }
  }
  CCU::CalibrationResult res = CCU::perform_calibration(markers,enabled, Ts, Trel, image->getSize(),
                                                        deactivatedCenters, gui["useCorners"],
                                                        gui["errNormalized"], bestOfNSaver,
                                                        haveAnyCalibration, scene,
                                                        intr, useLMA);
  gui["status"] = res.status;
  gui["error"] = res.error;
  gui["save_remaining_frames"] = res.saveRemainingFrames;
  gui["save_best_error"] = res.saveBestError;

  static DrawHandle3D draw = gui["draw"];

  draw = calibFileData.lastFD->getIntermediateImage(gui["iin"].as<std::string>());

  CCU::visualize_found_markers(draw,markers,enabled,deactivatedCenters,gui["useCorners"]);

  if(calibFileData.planeObj){
    CCU::visualize_plane(draw, planeOptionGUI["planeDim"],planeOptionGUI["planeOffset"],
                         currentMousePos, scene);
  }
  if(!gridAdjuster.isNull()){
    draw->draw(gridAdjuster.vis());
  }


  draw.render();
}



int main(int n, char **ppc){
  std::vector<std::string> args(ppc+1,ppc+n);
  if(std::find(args.begin(),args.end(),"-cc") != args.end() ||
     std::find(args.begin(),args.end(),"-create-emtpy-config-file") != args.end()){
    std::cout << CCU::create_sample_calibration_file_content() << std::endl;
    return 0;
  }

  pa_explain("-crop-and-rescale","crops the outer pixels of the image (hcrop_pix on the left and on the "
             "right image border and vcrop_pix on the top and bottom image border). The resulting smaller image "
             "is then scaled up to the target image size given by target_width and target_height.")
  ("-if","use an already performed camera calibrtion file's intrinsic parameters. This is recommended in case of "
   "a calibration object that is not very close to the camera and whose projection therefore doesn't fill the camera "
   "image properly. In such cases, it is recommended to perform the camera calibration in a two tier-fashion. First "
   "the calibration object is placed optimally in front of the camera in order to optimize the intrinsic camera "
   "parameter calibration result. Then, in a second step, the result from the first calibration step is given using "
   "the '-intr filename' arg, so that only the extrinsic camera parameters have to be optimized in the second step. ")
  ("-ip","Though this program argument, known intrinsic camera parameters can be passed directly. By these means, "
   "it is not necessary to create a camera-xml file in order to pass externally known parameters to this program.")
  ("-is","For a set of cameras, known intrinsic camera parameters can be used, which usually leads to a much better "
   "accuracy of the extrinsic camera parameter set. The available spec-names for known camera parameter sets can be "
   "queried, by passing 'list' as a sub-arg of -is (icl-camera-calibration -is list)")
  ("-normalize-input-image","automatically scales the input image range to [0,255] and converts the input image to depth8u");

  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-config|-c(...) "
                "-camera|-cam(camera_file_to_load) "
                "-initial-transform-id|-it(idx=0) "
                "-create-empty-config-file|-cc "
                "-force-size|-s|-size(WxH) "
                "-convert-output-size|-os(WxH) "
                "-output|-o(output-xml-file-name) "
                "-normalize-input-image|-n "
                "-use-intrinsic-camera-parameter-file|-intr-file|-if(camera-filename) "
                "-use-intrinsic-camera-parameters|-intr-params|-ip(fx,fy,px,py,skew) "
                "-use-intrinsic-camera-parameter-spec|-intr-spec|-is(spec) "
                "-crop-and-rescale|-cr(crop_x_pix,crop_y_pix,new_width,new_height) "
                ,init,run).exec();
}
