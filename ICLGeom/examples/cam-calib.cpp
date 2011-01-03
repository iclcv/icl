/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/cam-calib.cpp                         **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLGeom/CalibrationObject.h>
#include <ICLGeom/Scene.h>
#include <fstream>
#include <ICLQt/DefineRectanglesMouseHandler.h>
#include <ICLCC/CCFunctions.h>

DefineRectanglesMouseHandler mouse(1,10);

struct ManualCalibrationObject : public CalibrationObject{
  bool useManualData;
  std::vector<Rect> lastAutomaticBBs;
  Mutex mutex;
  Img8u grayImage;
  
  ManualCalibrationObject(CalibrationGrid *grid, 
                          const std::string &configurableID):
    CalibrationObject(grid,configurableID),useManualData(false),grayImage(Size(1,1),formatGray){

    mouse.getOptions().fillColor[3] = 0;
  }

  virtual const ImgBase *findPoints (const ImgBase *sourceImage, 
                                     std::vector< Point32f > &cogs, 
                                     std::vector< Rect > &bbs){
    
    Mutex::Locker l(mutex);
    if(useManualData){
      bbs = mouse.getRects();
      cogs.resize(bbs.size());
      for(unsigned int i=0;i<bbs.size();++i){
        cogs[i] = bbs[i].center();
      }
      const ImgBase *im = storeInputImage(sourceImage);
      im = computeAndStoreGrayImage(im);
      im = computeAndStoreThresholdImage(im);
      im = computeAndStoreDilatedImage(im);
      return im;
    }else{
      const ImgBase *retImage = CalibrationObject::findPoints(sourceImage,cogs,bbs);
      lastAutomaticBBs = bbs;
      return retImage;
    }
  }
  Rect lastSelectedROI;
  
  void switchMode(){
    Mutex::Locker l(mutex);
    useManualData = !useManualData;
    if(useManualData){
      lastSelectedROI = mouse.getRectAtIndex(0);
      mouse.clearAllRects();
      mouse.setMaxRects(getCalibrationGrid()->getDimension().getDim()*2);
      for(unsigned int i=0;i<lastAutomaticBBs.size();++i){
        mouse.addRect(lastAutomaticBBs[i]);
      }
      mouse.getOptions().fillColor[3] = 0;
      mouse.getOptions().visualizeCenter = true;
    }else{
      mouse.clearAllRects();
      mouse.setMaxRects(1);
      mouse.addRect(lastSelectedROI);
      mouse.getOptions().fillColor[3] = 0;
      mouse.getOptions().visualizeCenter = false;
    }
  }
  
};

GUI gui("hsplit");
Scene scene;
GenericGrabber grabber;
ManualCalibrationObject *obj = 0;

static const int VIEW_CAM = 0, CALIB_CAM = 1;

void change_detection_mode(){
  obj->switchMode();
}

void init(){
  mouse.getOptions().fillColor[3]=0;
  
  if(pa("-cc")){
    CalibrationGrid::create_empty_configuration_file("/dev/stdout");
    CalibrationGrid::create_empty_configuration_file("new-calib-config.xml");
    ::exit(0);
  }
  
  
  obj = new ManualCalibrationObject(new CalibrationGrid(*pa("-c")),"co");
  grabber.init(FROM_PROGARG("-i"));
  grabber.setIgnoreDesiredParams(true);
  if(pa("-dist")){
    grabber.enableDistortion(DIST_FROM_PROGARG("-dist"),grabber.grab()->getSize());
  }

  // view camera
  scene.addCamera(Camera(Vec(-500,-320,570),
                         Vec(0.879399,0.169548,-0.444871),
                         Vec(0.339963,0.263626,0.902733)));
  // calibration camera
  scene.addCamera(Camera());
  scene.getCamera(VIEW_CAM).setName("View");
  scene.getCamera(CALIB_CAM).setName("Calibrated");
  scene.addObject(obj);

  
  /// main GUI
  gui << ( GUI("tab(camera view,scene view)[@minsize=16x12]")
           << "draw3D[@minsize=16x12@label=main view@handle=mainview]"
           << ( GUI("vbox")  
                << "draw3D[@handle=sceneview@minsize=16x12]"
                << ( GUI("hbox[@maxsize=100x2]")
                     << "button(sync)[@handle=syncCams]"
                     << "checkbox(visualize cams,off)[@out=visCams]"
                   )
              )
         )
      << ( GUI("vbox")
           <<  (GUI("hbox") 
                << "checkbox(3D overlay,unchecked)[@out=3D]"
                << "checkbox(2D overlay,checked)[@out=2D]"
                << "checkbox(no ROI,checked)[@out=mouseROI]"
                << "combo(color,gray,thresh,dilated)[@handle=vis@label=vis. image]"
               )
           <<  (GUI("hbox") 
                << "togglebutton(stopped,!grabbing)[@out=grab@label=grab-loop]"
                << "togglebutton(autom,manual)[@handle=manualMode@label=detection]"
                << "label()[@label=current error: @handle=currError]" 
                << "camcfg()"
               )
           << "prop(co)"
           <<  (GUI("hbox") 
                << "button(show and print camera)[@handle=printCam]"
                << "button(save best of 10)[@handle=saveBest10]"
               )
           )
      << "!show";

  (*gui.getValue<DrawHandle3D>("sceneview"))->setImageInfoIndicatorEnabled(false);
  gui.registerCallback(new GUI::Callback(change_detection_mode),"manualMode");
  gui["mainview"].install(&mouse);
  gui["sceneview"].install(scene.getMouseHandler(0));

}


template<class Iterator>
Iterator min_element_greater_zero(Iterator begin, Iterator end){
  if(begin == end) return begin;
  while(*begin < 0 && begin < end) ++begin;
  if(begin == end) return begin;

  Iterator minPos = begin;
  for(;begin < end;++begin){
    if(*begin < 0) continue;
    if(*begin < *minPos) minPos = begin;
  }
  return minPos;
}

void show_statistics(Camera *cams, float *errs, int n){
  Vec meanPos(0.0),meanNorm(0.0),meanUp(0.0);
  Vec varPos(0.0),varNorm(0.0),varUp(0.0);
  float meanErr = 0.0f, varErr = 0.0f;
  int numFound = 0;
  for(int i=0;i<n;++i){
    if(errs[i] >= 0){
      ++numFound;
      meanPos += cams[i].getPosition();
      meanNorm += cams[i].getNorm();
      meanUp += cams[i].getUp();
      meanErr += errs[i];
    }
  }
  if(!numFound) return;
  
  meanPos /= numFound;
  meanNorm /= numFound;
  meanUp /= numFound;
  meanErr /= numFound;

  
  for(int i=0;i<n;++i){
    if(errs[i] >= 0){
      for(int j=0;j<3;++j){
        varPos[j] += ::pow(meanPos[j]-cams[i].getPosition()[j],2);
        varNorm[j] += ::pow(meanNorm[j]-cams[i].getNorm()[j],2);
        varUp[j] += ::pow(meanUp[j]-cams[i].getUp()[j],2);
      }
      varErr += ::pow(meanErr-errs[i],2);
    }
  }
  varPos /= numFound;
  varNorm /= numFound;
  varUp /= numFound;
  varErr /= numFound;
  
  for(int j=0;j<3;++j){
    varPos[j] = sqrt(varPos[j]);
    varNorm[j] = sqrt(varNorm[j]);
    varUp[j] += sqrt(varUp[j]);
  }
  varErr = sqrt(varErr);
  
  std::cout << "Mean values:" << std::endl;
  std::cout << "Position: " << meanPos.resize<1,3>().transp() << std::endl;
  std::cout << "Norm vec: " << meanNorm.resize<1,3>().transp() << std::endl;
  std::cout << "Up vec  : " << meanUp.resize<1,3>().transp() << std::endl;
  std::cout << "Error   : " << meanErr << std::endl;
  std::cout << std::endl;
  std::cout << "Std-Deviations:" << std::endl;
  std::cout << "Position: " << varPos.resize<1,3>().transp() << std::endl;
  std::cout << "Norm vec: " << varNorm.resize<1,3>().transp() << std::endl;
  std::cout << "Up vec  : " << varUp.resize<1,3>().transp() << std::endl;
  std::cout << "Error   : " << varErr << std::endl;
  std::cout << std::endl;
}

void run(){
  gui_DrawHandle3D(mainview);
  gui_DrawHandle3D(sceneview);
  gui_ButtonHandle(printCam);
  gui_ButtonHandle(saveBest10);
  gui_ButtonHandle(syncCams);
  gui_LabelHandle(currError);
  gui_bool(grab);

  
  static SmartPtr<const ImgBase> image;
  if(grab || !image){
    const ImgBase *grabbed = grabber.grab();
    if(obj->useManualData){
      image = SmartPtr<const ImgBase>(grabbed,false);
    }else{
      image = SmartPtr<const ImgBase>(grabbed->shallowCopy(mouse.getRectAtIndex(0) & grabbed->getImageRect()));
    }
  }

  CalibrationObject::CalibrationResult result = obj->find(image.get());
  if(result){
    currError = result.error;
    scene.getCamera(CALIB_CAM) = result.cam;
    scene.getCamera(CALIB_CAM).setName("Calibrated");
  }else{
    currError = "no found";
  }

  // -------------------------------------------------
  // ------------ main view --------------------------
  // -------------------------------------------------

  const ImgBase *visim = obj->getIntermediateImage((CalibrationObject::IntermediateImageType)(int)gui["vis"]);
  mainview = visim ? visim : image.get();
  mainview->lock();
  mainview->reset();
  mainview->reset3D();
  
  if(gui["2D"]){
    obj->visualizeGrid2D(**mainview);
  }
  if(gui["3D"]){
    mainview->callback(scene.getGLCallback(CALIB_CAM));
  }
  mouse.visualize(**mainview);
  mainview->unlock();
  mainview.update();

  // -------------------------------------------------
  // ------------ scene view -------------------------
  // -------------------------------------------------

  if(syncCams.wasTriggered()){
    scene.getCamera(VIEW_CAM) = scene.getCamera(CALIB_CAM);
  }
  
  scene.setDrawCamerasEnabled(gui["visCams"]);
  scene.getCamera(CALIB_CAM).setName("Calibrated");
  scene.getCamera(VIEW_CAM).setName("View");
  
  static Img8u pseudoImage(image->getSize(),1);
  sceneview = &pseudoImage;
  sceneview->lock();
  sceneview->reset3D();
  sceneview->callback(::scene.getGLCallback(VIEW_CAM));
  sceneview->unlock();
  sceneview.update();


  // -------------------------------------------------
  // ------------ print camera------------------------
  // -------------------------------------------------

  if(printCam.wasTriggered()){
    Camera c = scene.getCamera(CALIB_CAM);
    c.setName(* pa("-i",0) + "-" + *pa("-i",1));
    std::cout << "------------------------------------------------------" << std::endl;
    std::string filename = pa("-o");
    std::cout << "new config file: (written to " <<  filename << ")" << std::endl;
    std::cout << c << std::endl;
    std::ofstream file(filename.c_str());
    file << c;
    std::cout << "------------------------------------------------------" << std::endl;
  }

  // -------------------------------------------------
  // ------------ save best of 10 --------------------
  // -------------------------------------------------

  static int framesLeft = -1;
  if(framesLeft != -1 || saveBest10.wasTriggered()){
    float lastError = result.error;
    
    static Camera cams[10];
    static float errs[10];

    std::cout << "captured frame " << ((framesLeft == -1) ? 1 : (9-framesLeft)) << "/10" << "\t" 
              << (lastError == -1 ? str("object not found!") : "err:" + str(lastError)) 
              << std::endl;
    
    if(framesLeft < 0){
      framesLeft = 9;
      cams[framesLeft] = scene.getCamera(CALIB_CAM);
      errs[framesLeft] = lastError;
      framesLeft--;

    }else if(framesLeft > 0){
      cams[framesLeft] = scene.getCamera(CALIB_CAM);
      errs[framesLeft] = lastError;
      framesLeft--;

    }else if(framesLeft == 0){
      framesLeft = -1;
      cams[0] = scene.getCamera(CALIB_CAM);
      errs[0] = lastError;
      
      show_statistics(cams,errs,10);
      
      int idx = (int)(min_element_greater_zero(errs,errs+10)-errs);
      if(idx == 10){
        WARNING_LOG("object was not found in any of the last 10 frames -> calibration not finished!");
      }else{
        Camera c = cams[idx];
        c.setName(* pa("-i",0) + "-" + *pa("-i",1));
        
        std::cout << "------------------------------------------------------" << std::endl;
        std::cout << "estimated camera pos is:" << c.getPosition().transp() <<std::endl;
        //std::cout << "worldOffset is:" << worldOffset.transp() << std::endl;
        std::cout << "best calibration error was:" << errs[idx] << std::endl << std::endl;
        //c.setPosition(c.getPosition()+worldOffset);
        std::string filename = pa("-o");
        std::cout << "new config file: (written to " <<  filename << ")" << std::endl;
        std::cout << c << std::endl;
        
        std::ofstream file(filename.c_str());
        file << c;
        std::cout << "------------------------------------------------------" << std::endl;
      }
    }
  }
  Thread::msleep(10);
}


int main(int n, char **ppc){
  // {{{ open
  paex
  ("-input","define input device e.g. '-input dc 0' or '-input file *.ppm'")
  ("-o","define output config xml file name (./extracted-camera-cfg.xml)")
  ("-config","define input marker config file (calib-config.xml by default)")
  ("-dist","give 4 distortion parameters")
  ("-create-empty-config-file","if this flag is given, an empty config file is created as ./new-calib-config.xml");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-config|-c(config-xml-file-name=calib-config.xml) "
                "-dist|d(float,float,float,float) "
                "-create-empty-config-file|-cc "
                "-output|-o(output-xml-file-name=extracted-cam-cfg.xml)",init,run).exec();
}
