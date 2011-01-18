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
#include <ICLUtils/ConfigFile.h>

#include <QtGui/QPushButton>
#include <QtGui/QMenu>
#include <QtGui/QInputDialog>
#include <QtGui/QCheckBox>

GUI gui("hsplit");
Scene scene;
GenericGrabber grabber;
static const int VIEW_CAM = 0, CALIB_CAM = 1;
QMenu *menu = 0;

struct CoordinateFrame : public SceneObject{
  CoordinateFrame(int dim=100, int t=4){
    float px[] = {dim/2,0,0,dim,t,t};
    SceneObject *xaxis = new SceneObject("cuboid",px);
    xaxis->setColor(Primitive::quad,geom_red());
    xaxis->setVisible(Primitive::line,false);
    xaxis->setVisible(Primitive::vertex,false);
    addChild(xaxis);

    float py[] = {0,dim/2,0,t,dim,t};
    SceneObject *yaxis = new SceneObject("cuboid",py);
    yaxis->setColor(Primitive::quad,geom_green());
    yaxis->setVisible(Primitive::line,false);
    yaxis->setVisible(Primitive::vertex,false);
    addChild(yaxis);


    float pz[] = {0,0,dim/2,t,t,dim};
    SceneObject *zaxis = new SceneObject("cuboid",pz);
    zaxis->setColor(Primitive::quad,geom_blue());
    zaxis->setVisible(Primitive::line,false);
    zaxis->setVisible(Primitive::vertex,false);
    addChild(zaxis);
  }
} cs;

struct ManuallyDefinedPoints : public SceneObject{
  Mutex mutex;
  void lock() { mutex.lock(); }
  void unlock() { mutex.unlock(); }
  void setVertices(const std::vector<Vec> &vs){
    m_children.clear();
    for(unsigned int i=0;i<vs.size();++i){
      SceneObject *p = addSphere(0,0,0,4,10,10);
      p->setColor(Primitive::quad,geom_red(255));
      p->setVisible(Primitive::vertex,false);
      p->setVisible(Primitive::line,false);
      p->translate(vs[i]);
    }
  }
} manualPoints;


struct CalibTool : public DefineRectanglesMouseHandler, 
                   public CalibrationGrid, 
                   public CalibrationObject {
  int highlightRect;
  int markedRects[2];
  std::vector<Rect> lastAutomaticBBs;
  Mutex mutex;
  Img8u grayImage;
  Rect lastSelectedROI;
  DefineRectanglesMouseHandler extra;
  
  enum Mode{
    AutoDetection,
    ManualRects,
    ManualSpecial,
    AddExtraPoints
  } mode; 
  
  CalibTool():
    DefineRectanglesMouseHandler(1,10),
    CalibrationGrid(*pa("-c")),
    CalibrationObject(this,"co"),
    grayImage(Size(1,1),formatGray),
    mode(AutoDetection)
  {
    getOptions().fillColor[3] = 0;
    markedRects[0] = markedRects[1] = -1;
    highlightRect = -1;
    initExtra();
  }
  
  CalibTool(bool):
    DefineRectanglesMouseHandler(1,10),
    grayImage(Size(1,1),formatGray),
    mode(AddExtraPoints)
    
  {
    getOptions().fillColor[3] = 0;
    markedRects[0] = markedRects[1] = -1;
    highlightRect = -1;
    initExtra();
  }
  
  void initExtra(){
    DefineRectanglesMouseHandler::Options &o = extra.getOptions();
    o.edgeColor[3] = 0;
    o.fillColor[3] = 0;
    o.centerColor = Color4D(255,0,0,255);
    o.showMetaData = true;
    o.handleWidth=0;
    o.visualizeCenter = true;
    o.visualizeHovering = false;
    o.metaColor = o.centerColor;
    extra.setMaxRects(500);
  }
  
  void changeModeTo(Mode mode){
    Mutex::Locker l(mutex);
    if(mode == AutoDetection){
      this->mode = mode;
      highlightRect = -1;
      clearAllRects();
      setMaxRects(1);
      addRect(lastSelectedROI);
      getOptions().fillColor[3] = 0;
      getOptions().visualizeCenter = false;
      highlightRect = markedRects[0] = markedRects[1] = -1;
    }else{
      if(this->mode == AutoDetection){
        lastSelectedROI = getRectAtIndex(0);
        clearAllRects();
        setMaxRects(isNull() ? 0 : getCalibrationGrid()->getDimension().getDim()*2+10);
        for(unsigned int i=0;i<lastAutomaticBBs.size();++i){
          addRect(lastAutomaticBBs[i]);
        }
      }
      this->mode = mode;
      getOptions().fillColor[3] = 0;
      getOptions().visualizeCenter = true;
    }
  }
  
  int getIdxOfRectAt(int x, int y){
    for(unsigned int i=0;i<rects.size();++i){
      if(rects[i].contains(x,y)) return i;
    }
    return -1;
  }

  void process(const MouseEvent &e){
    if(mode == ManualSpecial){
      Mutex::Locker l(this);
      highlightRect = getIdxOfRectAt(e.getX(),e.getY());
      if(highlightRect >= 0 && e.isPressEvent()){
        markedRects[0]=markedRects[1];
        markedRects[1] = highlightRect;
      }
    }else if(mode == ManualRects || mode == AutoDetection){
      DefineRectanglesMouseHandler::process(e);
    }else{
      if( (e.isRight()) && (e.isPressEvent()) && (extra.getRectAt(e.getX(),e.getY()) != Rect::null)){
        QAction *action = menu->exec(QCursor::pos()+QPoint(-2,-2));
        if(!action) return;
        std::string selectedText = action->text().toLatin1().data();
        if(selectedText == "delete"){
          extra.clearRectAt(e.getX(),e.getY());
        }else if(selectedText == "dublicate"){
          Rect r = extra.getRectAt(e.getX(),e.getY());
          r.x += 5;
          r.y += 5;
          Any meta = extra.getMetaDataAt(e.getX(),e.getY());
          extra.addRect(r);
          extra.setMetaData(extra.getNumRects()-1,meta);
          extra.bringToFront(extra.getNumRects()-1);
        }else if(selectedText == "set world pos"){
          QString text=QInputDialog::getText(*gui.getValue<DrawHandle3D>("mainview"),
                                             "define world position",
                                             "Please define the x y z\nworld position of this pixel",
                                             QLineEdit::Normal,
                                             extra.getMetaDataAt(e.getX(),e.getY()).c_str());
          if(!text.isNull() && text!=""){
            FixedRowVector<float,3> v = Any(text.toLatin1().data());
            extra.setMetaDataAt(e.getX(),e.getY(),v);
          }
        }else if(selectedText == "save..."){
          try{
            std::string filename = saveFileDialog("XML-Files (*.xml)");
            ConfigFile f;
            f.setPrefix("config.");
            std::vector<Rect> rs = extra.getRects();
            f["num points"] = (int)rs.size();
            for(unsigned int i=0;i<rs.size();++i){
              f["rect-"+str(i)+".rectangle"] = str(rs[i]);
              f["rect-"+str(i)+".world-pos"] = str(extra.getMetaData(i));
            }
            f.save(filename);
          }catch(...){}
        }else if(selectedText == "load..."){
          load();
        }
      }else{
        extra.process(e);
      }
    }
  }
  
  void load(){
    try{
      std::string filename = openFileDialog("XML-Files (*.xml)");
      ConfigFile f(filename);
      f.setPrefix("config.");
      extra.clearAllRects();
      int numPoints = f["num points"];
      for(int i=0;i<numPoints;++i){
        extra.addRect(parse<Rect>(f["rect-"+str(i)+".rectangle"]));
        Any meta = f["rect-"+str(i)+".world-pos"].as<std::string>();
        extra.setMetaData(i,meta);
      }
    }catch(ICLException &ex){
      SHOW(ex.what());
    }catch(...){}
  }
  
  /// automatic visualiziation
  /** The given ICLDrawWidget must be locked and reset before */
  void visualize(ICLDrawWidget &w){
    DefineRectanglesMouseHandler::visualize(w);
    
    Mutex::Locker l(this);
    if(highlightRect != -1 && highlightRect < (int)rects.size()){
      w.color(255,100,0,255);
      w.fill(255,100,0,100);
      w.rect(rects[highlightRect]);
    }
    w.color(255,0,0,255);
    w.fill(255,0,0,100);
    if( markedRects[0] != -1 && markedRects[0] < (int)rects.size()){
      w.rect(rects[markedRects[0]]);
    }
    if( markedRects[1] != -1 && markedRects[1] < (int)rects.size()){
      w.rect(rects[markedRects[1]]);
    }
    
    extra.visualize(w);
  }
  virtual std::pair<int,int> findMarkedPoints(const std::vector<Point32f> &cogs, 
                                              const std::vector<Rect> &bbs, 
                                              const Img8u *hintImage){
    if(mode == AutoDetection){
      return CalibrationGrid::findMarkedPoints(cogs,bbs,hintImage);
    }else{
      return std::pair<int,int>(markedRects[0],markedRects[1]);
    }
  }

  virtual const ImgBase *findPoints (const ImgBase *sourceImage, 
                                     std::vector< Point32f > &cogs, 
                                     std::vector< Rect > &bbs){
    
    Mutex::Locker l(mutex);
    if(mode == AutoDetection){
      const ImgBase *retImage = CalibrationObject::findPoints(sourceImage,cogs,bbs);
      lastAutomaticBBs = bbs;
      return retImage;
    }else{
      bbs = getRects();
      cogs.resize(bbs.size());
      for(unsigned int i=0;i<bbs.size();++i){
        cogs[i] = bbs[i].center();
      }
      setIntermediateImage(InputImage,sourceImage);
      setIntermediateImage(GrayImage,sourceImage);
      setIntermediateImage(ThresholdImage,sourceImage);
      setIntermediateImage(DilatedImage,sourceImage);

      return sourceImage;
    }
  }
  void updateSceneObjectFromManualData(ManuallyDefinedPoints *obj){
    obj->lock();
    std::vector<Rect> rs = extra.getRects();
    std::vector<Vec> worldPoints(rs.size());
    for(unsigned int i=0;i<rs.size();++i){
      worldPoints[i] = extra.getMetaData(i);
      worldPoints[i][3] = 1;
    }
    obj->setVertices(worldPoints);
    obj->unlock();
  }
  Camera applyCalibrationOnManualData(float &err, const Size &imageSize){
    Mutex::Locker l(mutex);
    std::vector<Rect> rs = extra.getRects();
    std::vector<Vec> worldPoints(rs.size());
    std::vector<Point32f> imagePoints(rs.size());
    for(unsigned int i=0;i<rs.size();++i){
      if(extra.getMetaData(i) != ""){
        worldPoints[i] = extra.getMetaData(i);
        worldPoints[i][3] = 1;
        imagePoints[i] = rs[i].center();
      }
    }
    Camera cam = Camera::calibrate(worldPoints,imagePoints);
    cam.getRenderParams().viewport = Rect(Point::null,imageSize);
    cam.getRenderParams().chipSize = imageSize;

    err = CalibrationGrid::get_RMSE_on_image(worldPoints,imagePoints,cam);
    return cam;
  }

} *tool = 0;


void load_points(){
  tool->load();
}

void change_mode(const std::string &what){
  static ButtonGroupHandle special = gui["special"];
  if(what == "mode"){
    int mode = (int)gui["mode"];
    switch(mode){
      case 0:
        tool->changeModeTo(CalibTool::AutoDetection);
        special.disable();
        break;
      case 1:
        tool->changeModeTo(CalibTool::ManualRects);
        special.enable(0);
        special.enable(1);
        special.select(0);
        break;
    }
  }else{
    switch(special.getSelected()){
      case 0:
        tool->changeModeTo(CalibTool::ManualRects);
        break;
      case 1:
        tool->changeModeTo(CalibTool::ManualSpecial);
        break;
      case 2:
        tool->changeModeTo(CalibTool::AddExtraPoints);
        break;
    }
  }
}

void init(){
  if(pa("-cc")){
    CalibrationGrid::create_empty_configuration_file("/dev/stdout");
    CalibrationGrid::create_empty_configuration_file("new-calib-config.xml");
    ::exit(0);
  }
  
  if(pa("-c")){
    tool = new CalibTool;
  }else{
    tool = new CalibTool(true);
  }
  
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
  if(!tool->isNull()){
    scene.addObject(tool);
  }
  scene.addObject(&cs);
  scene.addObject(&manualPoints);
  
  /// main GUI
  gui << ( GUI("tab(camera view,scene view)[@minsize=16x12]")
           << "draw3D[@minsize=16x12@label=main view@handle=mainview]"
           << ( GUI("vbox")  
                << "draw3D[@handle=sceneview@minsize=16x12]"
                << ( GUI("hbox[@maxsize=100x2]")
                     << "button(real view)[@handle=syncCams]"
                     << "button(reset view)[@handle=resetCam]"
                     << "checkbox(visualize cams,checked)[@out=visCams]"
                   )
              )
         )
      << ( GUI("vbox[@maxsize=16x100]")
           <<  (GUI("hbox") 
                << "checkbox(3D overlay,unchecked)[@out=3D@handle=h3D]"
                << "checkbox(2D overlay,checked)[@out=2D@handle=h2D]"
                << "combo(color,gray,thresh,dilated)[@handle=vis@label=vis. image]"
               )
           <<  (GUI("hbox") 
                << "buttongroup(automatic detection,manual definition)[@handle=mode@label=detection]"
                << "buttongroup(bounding boxes,mark 2 bounding boxes,add extra ref. points)[@handle=special@label=manual options]"
               )
           <<  (GUI("hbox") 
                << "togglebutton(stopped,!grabbing)[@out=grab@label=grab-loop]"
                << "label()[@label=current error: @handle=currError]" 
                << "camcfg()"
               )
           << (!tool->isNull() ? "prop(co)" : "label(no calibration object file given)")
           <<  (GUI("hbox") 
                << "button(load points)[@handle=loadPoints]"
                << "button(show camera)[@handle=printCam]"
                << "button(save best of 10)[@handle=saveBest10]"
               )
           )
      << "!show";

  (*gui.getValue<DrawHandle3D>("sceneview"))->setImageInfoIndicatorEnabled(false);
  gui.registerCallback(new GUI::Callback(change_mode),"mode,special");
  gui["mainview"].install(tool);
  gui["sceneview"].install(scene.getMouseHandler(0));
  gui["loadPoints"].registerCallback(new GUI::Callback(load_points));
  
  ButtonGroupHandle mode = gui["mode"];
  ButtonGroupHandle special = gui["special"];
  if(tool->isNull()){
    mode.disable();
    mode.select(1);
    special.disable(0);
    special.disable(1);
    special.select(2);
    CheckBoxHandle h2D = gui["h2D"];
    CheckBoxHandle h3D = gui["h3D"];
    h2D.disable();
    h2D->setChecked(false);
    h3D->setChecked(true);
    //h3D.disable();
  }else{
    gui["special"].disable();
  }
  
  menu = new QMenu;
  menu->addAction("set world pos");
  menu->addAction("dublicate");
  menu->addAction("delete");
  menu->addAction("cancel");
  menu->addSeparator();
  menu->addAction("save...");
  menu->addAction("load...");
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
  gui_ButtonHandle(resetCam);
  gui_LabelHandle(currError);
  gui_bool(grab);

  
  static SmartPtr<const ImgBase> image;
  if(grab || !image){
    const ImgBase *grabbed = grabber.grab();
    if(pa("-s")){
      static ImgBase *scaled = 0;
      static Size size = pa("-s");
      ensureCompatible(&scaled, grabbed->getDepth(),size,grabbed->getFormat());
      scaled->setTime(grabbed->getTime());
      grabbed->scaledCopy(&scaled);
      grabbed = scaled;
    }

    image = SmartPtr<const ImgBase>(grabbed->shallowCopy(tool->getRectAtIndex(0) & grabbed->getImageRect()));
    
  }

  const ImgBase *visim = 0;

  struct CalibrationResult{
    Camera cam; 
    float error;    
    operator bool() const { return error >= 0; }
  } result = {Camera(),0.0f};

  if(!tool->isNull()){
    CalibrationObject::CalibrationResult r = tool->find(image.get());
    result.cam = r.cam;
    result.error = r.error;
    

    // -------------------------------------------------
    // ------------ main view --------------------------
    // -------------------------------------------------

    if(tool->mode == CalibTool::AutoDetection){
      visim = tool->getIntermediateImage((CalibrationObject::IntermediateImageType)(int)gui["vis"]);
    }else{
      visim = image.get();
    }
    if(result){
      currError = result.error;
      scene.getCamera(CALIB_CAM) = result.cam;
      scene.getCamera(CALIB_CAM).setName("Calibrated");
    }else{
      currError = "no found";
    }
  }else{
    try{
      tool->updateSceneObjectFromManualData(&manualPoints);
      result.cam = tool->applyCalibrationOnManualData(result.error,image->getSize());
    }catch(Camera::NotEnoughDataPointsException &ex){
      currError = "need at least\n6 data points";
      result.error = -1;
    }catch(ICLException &ex){
      currError = ex.what();
      result.error = -1;
    }catch(...){
      currError = "need at least\n6 data points";
      result.error = -1;
    }
    if(result){
      currError = result.error;
      scene.getCamera(CALIB_CAM) = result.cam;
      scene.getCamera(CALIB_CAM).setName("Calibrated");
    }
  }

  
  mainview = visim ? visim : image.get();
  mainview->lock();
  mainview->reset();
  mainview->reset3D();
  
  if((bool)gui["2D"] && !tool->isNull()){
    tool->visualizeGrid2D(**mainview); // grid
  }
  if((bool)gui["3D"] && result.error >= 0){
    mainview->callback(scene.getGLCallback(CALIB_CAM));
  }
  tool->visualize(**mainview); // mouse-rects
  mainview->unlock();
  mainview.update();

  // -------------------------------------------------
  // ------------ scene view -------------------------
  // -------------------------------------------------

  if(syncCams.wasTriggered()){
    scene.getCamera(VIEW_CAM) = scene.getCamera(CALIB_CAM);
  }
  if(resetCam.wasTriggered()){
    scene.getCamera(VIEW_CAM) = Camera();
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
  ("-config","define input marker config file (calib-config.xml by default), if no config-file is given, only the "
             "completely manual mode is enabled. Here you can define reference points manually.")
  ("-dist","give 4 distortion parameters")
  ("-s","forced image size that is used (even if the grabber device provides another size)")
  ("-create-empty-config-file","if this flag is given, an empty config file is created as ./new-calib-config.xml");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-config|-c(config-xml-file-name) "
                "-dist|d(float,float,float,float) "
                "-create-empty-config-file|-cc "
                "-size|-s(WxH) "
                "-output|-o(output-xml-file-name=extracted-cam-cfg.xml) "
                ,init,run).exec();
}
