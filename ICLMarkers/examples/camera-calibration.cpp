/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/camemra-calibration.cpp               **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLQt/Common.h>
#include <ICLGeom/Scene.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLGeom/GridSceneObject.h>

#include <QtGui/QMessageBox>
#include <fstream>
#include <ICLUtils/ConfigFile.h>

struct PossibleMarker{
  PossibleMarker():loaded(false){}
  PossibleMarker(int cfgFileIndex,const Vec &v):loaded(true),center(v),hasCorners(false),cfgFileIndex(cfgFileIndex){}
  PossibleMarker(int cfgFileIndex,const Vec &v, const Vec &a, const Vec &b, const Vec &c, const Vec &d):
    loaded(true),center(v),hasCorners(true),cfgFileIndex(cfgFileIndex){
    corners[0] = a;
    corners[1] = b;   
    corners[2] = c;
    corners[3] = d;
  }
  bool loaded;
  Vec center;
  bool hasCorners;
  Vec corners[4];
  int cfgFileIndex;
};

enum MarkerType {
  BCH,
  AMOEBA
};


HSplit gui;
GUI relTransGUI;
GUI markerDetectionOptionGUI;
GUI planeOptionGUI;

Scene scene;
GenericGrabber grabber;

Point32f currentMousePos;
bool havePlane = false;
bool haveAnyCalibration = false;


SmartPtr<FiducialDetector> fds[2];

std::vector<PossibleMarker> possible[2] = {
  std::vector<PossibleMarker>(4096), 
  std::vector<PossibleMarker>(4096)
};

FiducialDetector *lastFD = 0; // used for visualization
SceneObject *planeObj = 0;



//std::vector<std::pair<SceneObject*,Mat> > calibObjs;

std::string sample= ("<config>\n"
                     "  <!-- A grid is a regular 1D or 2D grid of markers in 3D space.\n"
                     "       Each grid is defined by the (x,y,z)-offset of the upper left marker,\n"
                     "       and by two direction vectors 'x-direction' and 'y-direction'. \n"
                     "       A grid marker at grid position (i,j) is located at \n"
                     "       offset + x-direction*i + y-direction*j\n"
                     "       The grid markers are assumed to be in the defined marker-range;\n"
                     "       the markers are distributed in row-major order. Possible marker\n"
                     "       types are 'bch' and 'amoeba'. If the marker type is bch, not only\n"
                     "       the marker centers, but also the markers corners are used as\n"
                     "       reference points. -->\n"
                     "  <section id=\"grid-0\">\n"
                     "      <data id=\"dim\" type=\"string\">(NumXCells)x(NumYCells)</data>\n"
                     "      <data id=\"offset\" type=\"string\">x,y,z</data>\n"
                     "      <data id=\"x-direction\" type=\"string\">dx1,dy1,dz1</data>\n"
                     "      <data id=\"y-direction\" type=\"string\">dx2,dy2,dz2</data>\n"
                     "      <data id=\"marker-type\" type=\"string\">amoeba|bch</data>\n"
                     "      <data id=\"marker-ids\" type=\"string\">[minID,maxID]</data>\n"
                     "   </section>\n"
                     "    <!-- more grids with raising indices (also index 0 is optional)-->\n"
                     "\n"
                     "  <!-- For simplicity, 1 by 1 grids (i.e. single markers) can be defined\n"
                     "       in a simpler way. Here, less information needs to be given, however\n"
                     "       it is important to mention, that the marker corners can only be used\n"
                     "       for grids. -->\n"
                     "   <section id=\"marker-0\">\n"
                     "      <data id=\"marker-type\" type=\"string\">amoeba|bch</data>\n"
                     "      <data id=\"offset\" type=\"string\">x,y,z</data>\n"
                     "      <data id=\"marker-id\" type=\"int\">id</data>\n"
                     "   </section>\n"
                     "   <!-- more markers with raising indices (also index 0 is optional)-->\n"
                     "\n"
                     "  <!-- If the calibration object can be placed in the scene in different distinct\n"
                     "       ways (i.e. with differnt relative transformations), several world-tranforms can\n"
                     "       can be defined. All defined world transforms can be chosen interactively at runtime.\n"
                     "       By default, the first world transform is used. If no world transform is given, a dummy\n"
                     "       transform called 'identity' is added automatically -->\n"
                     "   <section id=\"world-transform-0\" type=\"string\">    <!-- also optional -->\n"
                     "      <data id=\"name\" type=\"string\">example with z-offset of 100mm</data>\n"
                     "      <data id=\"transform\" type=\"string\">\n"
                     "          1 0 0 0\n"
                     "          0 1 0 0\n"
                     "          0 0 1 100\n"
                     "          0 0 0 1\n"
                     "      </data>\n"
                     "   </section>\n"
                     "   <!-- more possible world transforms (if none is given, an identity transform is added automatically -->\n"
                     "   <data id=\"obj-file\" type=\"string\">\n"
                     "      <!-- optional .obj file content that describes the visual shape of the calibration object -->\n"
                     "   </data>\n"
                     "</config>\n");

void mouse(const MouseEvent &e){
  currentMousePos = e.getPos();
}

FiducialDetector *create_new_fd(MarkerType t, std::vector<std::string> &configurables, std::string &iin){
  static const std::string ts[2] = {"bch","amoeba"};
  FiducialDetector *fd = new FiducialDetector(ts[t]);
  fd->setConfigurableID(ts[t]);
  configurables.push_back(ts[t]);
  iin = fd->getIntermediateImageNames();
  fd->setPropertyValue("css.angle-threshold",180);
  fd->setPropertyValue("css.curvature-cutoff",66);
  fd->setPropertyValue("css.rc-coefficient",1);
  fd->setPropertyValue("thresh.global threshold",-10);
  fd->setPropertyValue("thresh.mask size",30);

  if(pa("-s").as<bool>() && parse<Size>(pa("-s")) == Size::QVGA){
    fd->setPropertyValue("pp.filter","none");
  }
  lastFD = fd;
  return fd;
}


Mutex save_lock;

static std::string get_save_filename(){
  if(pa("-o")){
    return *pa("-o");
  }else{ 
    try{
      return saveFileDialog("*.xml","save current camera","./");
    }catch(...){}
  }
  return "";
}

static void save_cam(const Camera &cam, const std::string &filename){
  if(filename.length()){
    std::ofstream s(filename.c_str());
    s << cam;
  }
}

void save(){
  save_lock.lock();
  Camera cam = scene.getCamera(0);
  save_lock.unlock();

  std::string filename = get_save_filename();
  save_cam(cam,filename);
}                   

struct BestOfNSaver : public QObject{
  std::vector<Camera> cams;
  std::vector<float> errors;
  int n;
  int num_end;
  bool inited;
  std::string filename;
  
  float lastBestError;
  std::string lastFileName;

  float runningBestError;
  
  BestOfNSaver():
    inited(false)
  {}

  virtual bool event ( QEvent * event ){
    ICLASSERT_RETURN_VAL(event,false);
    if(event->type() == QEvent::User){
      QMessageBox::information(0,"saved",("camera file has been saved to\n"+lastFileName+"\n"+"error was:"+str(lastBestError)).c_str());
      return true;
    }else{
      return QObject::event(event);
    }
  } 
  
  void init(){
    num_end = gui["save_num_frames"].as<int>();
    Mutex::Locker l(save_lock);
    if(inited) return;
    filename = get_save_filename();
    if(filename != ""){
      cams.resize(0); cams.reserve(num_end);
      errors.resize(0); errors.reserve(num_end);
      n = 0;
      inited = true;
    }
    runningBestError = 99999;
  }
  
  void stop(){
    Mutex::Locker l(save_lock);
    if(inited){
      n = num_end;
    }
  }
  
  void next_hook(const Camera &cam, float error){
    Mutex::Locker l(save_lock);
    if(!inited) return;

    if(error > 0){
      cams.push_back(cam);
      errors.push_back(error);
    }
    ++n;

    gui["save_remaining_frames"] = num_end - n;
    if(error > 0 && (error < runningBestError)){
      runningBestError = error;
    }
    gui["save_best_error"] = runningBestError;
    
    if(n >= num_end){
      if(!cams.size()){
        QMessageBox::critical(0,"error","unable to save the best of N calibration results:\n"
                              "all N calibration runs failed!");
      }
      else{
        int best = (int)(std::min_element(errors.begin(),errors.end())-errors.begin());
        int worst = (int)(std::max_element(errors.begin(),errors.end())-errors.begin());
        
        std::cout << "best error:" << errors[best] << std::endl;
        std::cout << "worst error:" << errors[worst] << std::endl;
        
        save_cam(cams[best],filename);
        lastBestError = errors[best];
        lastFileName = filename;
        QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
      }
      inited = false;
    }
  }
  
} *bestOfNSaver = 0;


static inline Vec set_3_to_1(Vec a){
  a[2] += 1;
  a[3] = 1;
  return a;
}

void change_plane(const std::string &handle){
  if(handle == "planeDim"){
    if(planeOptionGUI["planeDim"].as<std::string>() == "none"){
      planeOptionGUI["planeOffset"].disable();
      planeOptionGUI["planeRadius"].disable();
      planeOptionGUI["planeTicDist"].disable();
      planeOptionGUI["planeColor"].disable();
      planeOptionGUI["planeStatus"] = str("removed");
      scene.removeObject(planeObj);
      planeObj = 0;
      havePlane = false;
      return;
    }else{
      havePlane = true;
      planeOptionGUI["planeOffset"].enable();
      planeOptionGUI["planeRadius"].enable();
      planeOptionGUI["planeTicDist"].enable();
      planeOptionGUI["planeColor"].enable();
    }
  }
  if(planeObj){
    scene.removeObject(planeObj);
    ICL_DELETE(planeObj);
  }
  
  const std::string t = planeOptionGUI["planeDim"];
  const float offset = planeOptionGUI["planeOffset"];
  const float radius = parse<float>(planeOptionGUI["planeRadius"]);
  const float ticDist = planeOptionGUI["planeTicDist"];
  const Color4D c = planeOptionGUI["planeColor"];

  int n = (2*radius) / ticDist;
  if(n * n > 1000000){
    planeOptionGUI["planeStatus"] = str("too many nodes");
    return;
  }else{
    planeOptionGUI["planeStatus"] = str("ok (") + str(n*n) + " nodes)";
  }
  Vec o(offset*(t=="x"),offset*(t=="y"),offset*(t=="z"),1);
  Vec dx,dy;
  if(t == "x"){
    dx = Vec(0,ticDist,0,1);
    dy = Vec(0,0,ticDist,1);
  }else if(t == "y"){
    dx = Vec(ticDist,0,0,1);
    dy = Vec(0,0,ticDist,1);
  }else{
    dx = Vec(ticDist,0,0,1);
    dy = Vec(0,ticDist,0,1);
  }
  int n2 = n/2;

  planeObj = new GridSceneObject(n,n,o -dx*(n2) - dy*(n2) ,dx,dy,true,false);
  planeObj->setColor(Primitive::line,GeomColor(c[0],c[1],c[2],c[3]));
  planeObj->setVisible(Primitive::vertex,false);

  planeObj->addVertex(set_3_to_1(o-dx*n2));
  planeObj->addVertex(set_3_to_1(o+dx*n2));

  planeObj->addVertex(set_3_to_1(o-dy*n2));
  planeObj->addVertex(set_3_to_1(o+dy*n2));
  
  planeObj->addLine(n*n,n*n+1,GeomColor(255,0,0,255));
  planeObj->addLine(n*n+2,n*n+3,GeomColor(0,255,0,255));


  scene.addObject(planeObj);
}

struct NamedTransform{
  NamedTransform(){}
  NamedTransform(const std::string &name,const Mat &t):
    name(name),transform(t){}
  std::string name;
  Mat transform;
};

struct CalibFile{
  std::string filename;
  std::vector<NamedTransform> transforms;
  SceneObject* obj;
};

std::vector<CalibFile> loadedFiles;

void init(){
  bestOfNSaver = new BestOfNSaver;

  if( !pa("-c") || !pa("-c").n() ){
    pa_show_usage("program argument -c must be given with at least one sub-argument");
    ::exit(0); 
  }

  std::vector<std::string> configurables;
  std::string iin;
  GUI objGUI;

  for(int c = 0; c <pa("-c").n(); ++c){
    CalibFile cf;
    cf.filename = *pa("-c",c);
    cf.obj = 0;
    ConfigFile cfg(cf.filename);
    
    std::cout << "* parsing given configuration file '" << cf.filename << "'" << std::endl;
    


    std::ostringstream transformNameList;    
    for(int t=0;true;++t){
      try{
        const std::string name = cfg["config.world-transform-"+str(t)+".name"];
        transformNameList << (t?",":"") << name;
        const Mat transform = parse<Mat>(cfg["config.world-transform-"+str(t)+".transform"]);
        cf.transforms.push_back(NamedTransform(name,transform));
      }catch(...){
        break;
      }
    }

    const bool transformGiven = cf.transforms.size();
    if(!transformGiven){
      cf.transforms.push_back(NamedTransform("identity",Mat::id()));
    }


    File cff(cf.filename);
    std::string tt = "full path: " + cf.filename;
    objGUI << ( HBox().label(cff.getBaseName()+cff.getSuffix()).minSize(1,3).maxSize(100,3)
                << CheckBox("enable",true).out("enable-obj-"+str(c)).tooltip(tt)
                << Combo(transformNameList.str() + (transformGiven?"":",id")).handle("transform-obj-"+str(c)).tooltip(tt)
                );
    
    try{
      std::string s;
      try{
        const std::string s2 = cfg["config.obj-file"];
        s = s2;
      }catch(...){
        throw 1;
      }
      {
        std::ofstream obj("/tmp/tmp-obj-file.obj");
        obj << s << std::endl;
      }

      SceneObject *o = new SceneObject("/tmp/tmp-obj-file.obj");
      o->setColor(Primitive::quad,GeomColor(0,100,255,100));
      o->setColor(Primitive::line,GeomColor(255,0,0,255));
      o->setVisible(Primitive::line,true);
      o->setLineWidth(2);
      o->setTransformation(cf.transforms[0].transform);
      scene.addObject(o);
      o->setVisible(false);
      cf.obj = o;
    }catch(ICLException &e){
      SHOW(e.what());
    }catch(int){}
    
    int systemResult = system("rm -rf /tmp/tmp-obj-file.obj");
    (void)systemResult;
    
    enum Mode{
      ExtractGrids,
      ExtractSingleMarkers,
      ExtractionDone
    } mode = ExtractGrids;

    for(int i=0;mode != ExtractionDone ;++i){
      
      cfg.setPrefix(str("config.") + ((mode == ExtractGrids) ? "grid-" : "marker-")+str(i)+".");  
      if(!cfg.contains("offset")) {
        mode = (Mode)(mode+1);
        i = -1;
        continue;
      }

      Vec3 o,dx,dy,dx1,dy1;
      Size s(1,1);
      Size32f ms;
      Range32s r;
      MarkerType t;
      bool haveCorners;
      try{
        t = (MarkerType)(cfg["marker-type"].as<std::string>() == "amoeba");
        o = parse<Vec3>(cfg["offset"]);
        if(mode == ExtractGrids){
          s = parse<Size>(cfg["dim"]);
          dx = parse<Vec3>(cfg["x-direction"]);
          dy = parse<Vec3>(cfg["y-direction"]);
          dx1 = dx.normalized();
          dy1 = dy.normalized();
          r = parse<Range32s>(cfg["marker-ids"]);
          ICLASSERT_THROW(r.getLength()+1 == s.getDim(), ICLException("error loading configuration file at given grid " + str(i)
                                                                      + ": given size " +str(s) + " is not compatible to "
                                                                      + "given marker ID range " +str(r) ));
        }else{
          r.minVal = r.maxVal = cfg["marker-id"].as<int>();
        }

        if(!fds[t]) fds[t] = create_new_fd(t,configurables,iin);
        try{ 
          ms = parse<Size32f>(cfg["marker-size"]); 
        } catch(...){}

        fds[t]->loadMarkers(r,t==AMOEBA ? ParamList() : ParamList("size",ms));

        if(mode == ExtractGrids){
          std::cout << "** registering grid with " << (t?"amoeba":"bch") << " marker range " << r << std::endl; 
        }else{
          std::cout << "** registering single " << (t?"amoeba":"bch") << " marker with id " << r.minVal << std::endl; 
        }
        
        int id = r.minVal;
        std::vector<PossibleMarker> &lut = possible[t];
        //  std::vector<Vec> vertices;
        
        haveCorners = (mode==ExtractGrids) && (ms != Size32f::null) && (t==BCH);

        for(int y=0;y<s.height;++y){
          for(int x=0;x<s.width;++x){
            Vec3 v = o+dx*x +dy*y;
            if(lut[id].loaded) throw ICLException("error loading configuration file at given grid " + str(i)
                                                 +" : the marker ID " + str(id) + " was already used before");
            if(haveCorners){
              Vec3 ul = v + dx1*(ms.width/2) - dy1*(ms.height/2);
              Vec3 ur = v + dx1*(ms.width/2) + dy1*(ms.height/2);
              Vec3 ll = v - dx1*(ms.width/2) + dy1*(ms.height/2);
              Vec3 lr = v - dx1*(ms.width/2) - dy1*(ms.height/2);
              
              lut[id++] = PossibleMarker(c,
                                         v.resize<1,4>(1),
                                         ul.resize<1,4>(1),
                                         ur.resize<1,4>(1),
                                         ll.resize<1,4>(1),
                                         lr.resize<1,4>(1));
            }else{
              lut[id++] = PossibleMarker(c,Vec(v[0],v[1],v[2],1));
            }
            //            vertices.push_back(cf.transforms[0].transform*Vec(v[0],v[1],v[2],1));
          }
        }
      }catch(ICLException &e){
        ERROR_LOG("Error parsing xml configuration file: '" << *pa("-c",c) << "': " << e.what());
        continue;
      }
    }
    loadedFiles.push_back(cf);
  }
  
  grabber.init(pa("-i"));
  if(pa("-s")) grabber.useDesired(pa("-s").as<Size>());

#ifdef OLD_GUI  
  gui << "draw3D()[@handle=draw@minsize=32x24]";
#endif
  gui << Draw3D().handle("draw").minSize(32,24);
  
  //std::ostringstream tabs;
  //for(unsigned int i=0;i<configurables.size();++i){
  // tabs << configurables[i] << (i<configurables.size()-1 ? "," 
  //}
  //tabstr[tabstr.length()-1] = ')';
  markerDetectionOptionGUI = Tab(cat(configurables,","));
  for(unsigned int i=0;i<configurables.size();++i){
    markerDetectionOptionGUI << Prop(configurables[i]);
  }
#ifdef OLD_GUI
  gui << ( GUI("vbox[@minsize=16x1@maxsize=16x100]") 
           << ( GUI("hbox[@maxsize=100x3@minsize=1x3]") 
                << "combo(" +iin + ")[@handle=iin@label=visualized image]"
                << "slider(0,255,128)[@out=objAlpha@label=object-alpha]"
                )
           << ( GUI("hbox[@maxsize=100x3]") 
                << "checkbox(use corners,checked)[@out=useCorners]"
                << "checkbox(show CS,checked)[@out=showCS]"
                << "label( )[@handle=error@label=error]"
                )
           << ( GUI("hbox[@label=more options@minsize=1x3@maxsize=100x3]")
                << "button(plane)[@handle=show-plane-options]" 
                << "button(markers)[@handle=show-marker-detection-options]" 
                << "button(rel. Transf.)[@handle=showRelTransGUI]"
              )
           << (GUI("vscroll[@label=calibration objects]") << objGUI)
           << "label(ready..)[@minsize=1x3@maxsize=100x3@label=detection status@handle=status]"
           << ( GUI("hbox[@maxsize=100x3@minsize=1x3]") 
                << "button(save camera)[handle=save]"
                << "button(save best of 10)[handle=save10]"
               )
           )
      << "!show";
  
  planeOptionGUI << ( GUI("hbox")
                     << "combo(none,x,y,z)[@label=normal@handle=planeDim]"
                     << "float(-10000,10000,0)[@label=offset mm@handle=planeOffset]"
                     )
                << ( GUI("hbox")
                     << "combo(100,200,500,!1000,2000,3000,5000,10000)"
                        "[@label=radius mm@handle=planeRadius]"
                     << "float(1,1000,10)[@label=tic distance mm@handle=planeTicDist]"
                     )
                << ( GUI("hbox")
                     << "label( )[@handle=planeStatus@label=status]"
                     << "color(40,40,40,255)[@handle=planeColor@label=color]"
                     )
                 << "!create";


  relTransGUI << ( GUI("vbox[@label=rel-transformation]") 
                   << ( GUI("hbox")
                        << "spinner(0,8,0)[@label=x-rotation *pi/4@out=rx]"
                        << "spinner(0,8,0)[@label=y-rotation *pi/4@out=ry]"
                        << "spinner(0,8,0)[@label=z-rotation *pi/4@out=rz]"
                        )
                   << ( GUI("hbox")
                        << "float(-100000,100000,0)[@label=x-offset@out=tx]"
                        << "float(-100000,100000,0)[@label=y-offset@out=ty]"
                        << "float(-100000,100000,0)[@label=z-offset@out=tz]"
                        )
                   )
              << "button(show transformation matrix)[@handle=showRelTrans]" << "!create";
#endif
  
  gui << ( VBox().minSize(14,28).maxSize(14,100)
           << (VBox().label("visualization").minSize(1,5)
               << ( HBox().maxSize(100,3).minSize(1,3) 
                    << Combo(iin).handle("iin").label("visualized image")
                    << Slider(0,255,128).out("objAlpha").label("object-alpha")
                    )
               << ( HBox().maxSize(100,3) 
                    << CheckBox("use corners",true).out("useCorners")
                    << CheckBox("show CS",false).out("showCS")
                    )
               )
           << ( HBox().label("more options").minSize(1,3).maxSize(100,3)
                << Button("plane").handle("show-plane-options")
                << Button("markers").handle("show-marker-detection-options")
                << Button("rel. Transf.").handle("showRelTransGUI")
                )
           << (VScroll().label("calibration objects") 
               << objGUI
               )
           << (HBox().maxSize(100,3)
               << Label().handle("error").label("error").tooltip("The error is given by the mean square distance\n"
                                                                 "of the actually detected points and the points\n"
                                                                 "that are projected into the scene using the\n"
                                                                 "current camera calibration result\n"
                                                                 "If 'normalized error' is checked, the sum of the\n"
                                                                 "is not normalized by the number of points N\n"
                                                                 "but by N^2. To make the error comparable, it is\n"
                                                                 "also mutiplied by 100 in this case.")
               << CheckBox("nomalized error",true).out("errNormalized").tooltip("if checked, the total calibration error\n"
                                                                                "is not devided by the number of calibration points N,\n"
                                                                                "but by N^2 in order to avoid favoring frames where\n"
                                                                                "only few markers were found.")
               )
           << Label("ready..").minSize(1,2).maxSize(100,2).label("detection status").handle("status")
           << ( VBox().maxSize(100,6).minSize(1,6) 
                << ( HBox()
                     << Spinner(1,10000,10).out("save_num_frames").label("# frames").tooltip("When you press save, the system will\ncapture that many frames to find\nan optimal calibration result")
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
  gui["show-marker-detection-options"].registerCallback(function(&markerDetectionOptionGUI,&GUI::switchVisibility));
  gui["show-plane-options"].registerCallback(function(&planeOptionGUI,&GUI::switchVisibility));
           

  gui["save"].registerCallback(function(bestOfNSaver,&BestOfNSaver::init));
  gui["save_stop"].registerCallback(function(bestOfNSaver,&BestOfNSaver::stop));

  scene.addCamera(Camera());
  scene.getCamera(0).setResolution(grabber.grab()->getSize());
  
  gui["showRelTransGUI"].registerCallback(function(&relTransGUI,&GUI::switchVisibility));
  
  planeOptionGUI["planeOffset"].disable();
  planeOptionGUI["planeRadius"].disable();
  planeOptionGUI["planeTicDist"].disable();
  planeOptionGUI["planeColor"].disable();
  planeOptionGUI.registerCallback(change_plane,"planeOffset,planeRadius,planeTicDist,planeDim,planeColor");
  
  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(new MouseHandler(mouse));
  
 
}

struct FoundMarker{
  FoundMarker(){}
  FoundMarker(Fiducial fid, const Point32f &imagePos, const Vec &worldPos, int cfgFileIndex):
    fid(fid),imagePos(imagePos),worldPos(worldPos),hasCorners(false),cfgFileIndex(cfgFileIndex){}
  FoundMarker(Fiducial fid, const Point32f &imagePos, const Vec &worldPos,
              const Point32f imageCornerPositions[4],
              const Vec worldCornerPositions[4],
              int cfgFileIndex):
    fid(fid),imagePos(imagePos),worldPos(worldPos),hasCorners(true),cfgFileIndex(cfgFileIndex){
    std::copy(imageCornerPositions,imageCornerPositions+4,this->imageCornerPositions);
    std::copy(worldCornerPositions,worldCornerPositions+4,this->worldCornerPositions);
  }
  
  Fiducial fid;
  Point32f imagePos;
  Vec worldPos;
  bool hasCorners;
  Point32f imageCornerPositions[4];  
  Vec worldCornerPositions[4];
  int cfgFileIndex;
};


void run(){
  scene.lock();
  scene.setDrawCoordinateFrameEnabled(gui["showCS"]);
  scene.unlock();

  const Mat Trel = create_hom_4x4<float>(relTransGUI["rx"].as<float>()*M_PI/4,
                                         relTransGUI["ry"].as<float>()*M_PI/4,
                                         relTransGUI["rz"].as<float>()*M_PI/4,
                                         relTransGUI["tx"],relTransGUI["ty"],relTransGUI["tz"]);

  std::vector<Mat> Ts(loadedFiles.size());
  std::vector<bool> enabled(Ts.size());

  for(unsigned int i=0;i<loadedFiles.size();++i){
    int tidx = gui.get<ComboHandle>("transform-obj-"+str(i)).getSelectedIndex();
    Ts[i] = loadedFiles[i].transforms[tidx].transform;
    enabled[i] = gui.get<bool>("enable-obj-"+str(i));

    SceneObject *calibObj = loadedFiles[i].obj;
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

  ButtonHandle showRelTrans = relTransGUI["showRelTrans"];

  const ImgBase *image = grabber.grab();
  
  std::vector<FoundMarker> markers;
  for(int x=0;x<2;++x){
    if(!fds[x]) continue;
    const std::vector<Fiducial> &fids = fds[x]->detect(image);
    for(unsigned int i=0;i<fids.size();++i){

      const PossibleMarker &p = possible[x][fids[i].getID()];
      if(p.loaded){
        if(p.hasCorners && fids[i].getKeyPoints2D().size() == 4){
         const std::vector<Fiducial::KeyPoint> &kps = fids[i].getKeyPoints2D();
          Point32f imagePositions[4] = { 
            kps[0].imagePos, 
            kps[1].imagePos, 
            kps[2].imagePos,
            kps[3].imagePos 
          };
          
          markers.push_back(FoundMarker(fids[i],fids[i].getCenter2D(),p.center,
                                        imagePositions,p.corners,p.cfgFileIndex));
        }else{
          markers.push_back(FoundMarker(fids[i],fids[i].getCenter2D(),p.center,p.cfgFileIndex));
        }
      }
    }
  }


  if(showRelTrans.wasTriggered()){
    std::cout << "current relative transformation is:" << std::endl << Trel << std::endl;
    for(unsigned int i=0;i<loadedFiles.size();++i){
      std::cout << " * combined transformation matrix for calibration object '" << pa("-c",i) 
                << "' is:" << std::endl << (Trel * Ts[i]) << std::endl;
    }
  }

  
  std::vector<Vec> xws,xwsCentersOnly;
  std::vector<Point32f> xis,xisCentersOnly;
 
  const bool useCorners = gui["useCorners"];
  for(unsigned int i=0;i<markers.size();++i){
    const int idx = markers[i].cfgFileIndex;
    if(!enabled[idx]) continue;
    Mat T = Trel * Ts[idx];
    xws.push_back(T *markers[i].worldPos);
    xis.push_back(markers[i].imagePos);

    xwsCentersOnly.push_back(xws.back());
    xisCentersOnly.push_back(xis.back());

    if(useCorners && markers[i].hasCorners){
      for(int j=0;j<4;++j){
        xws.push_back(T * markers[i].worldCornerPositions[j]);
        xis.push_back(markers[i].imageCornerPositions[j]);
      }
    }
  }
  
  std::vector<Vec> *W[2] = { &xws, &xwsCentersOnly };
  std::vector<Point32f> *I[2] = { &xis, &xisCentersOnly };
  int idx = 0;
  bool deactivatedCenters = false;
  while(true){
    try{
      Camera &cam = scene.getCamera(0);

      {
        Mutex::Locker lock(save_lock);
        cam = Camera::calibrate_pinv(*W[idx], *I[idx]);
        cam.getRenderParams().viewport = Rect(Point::null,image->getSize());
        cam.getRenderParams().chipSize = image->getSize();
      }

      //      scene.setDrawCoordinateFrameEnabled(true);
      
      float error = 0;
      
      for(unsigned int i=0;i<W[idx]->size();++i){
        error += I[idx]->operator[](i).distanceTo(cam.project(W[idx]->operator[](i)));
      }
      if(gui["errNormalized"]){
        error /= sqr(W[idx]->size());
        error *= 100;
      }else{
        error /= W[idx]->size();
      }

      gui["error"] = error;
      gui["status"] = str("ok") + ((idx&&useCorners) ? "(used centers only)" : "");
      if(idx) deactivatedCenters = true;
      
      bestOfNSaver->next_hook(cam,error);
      
      if(error > 0 && !haveAnyCalibration){
        haveAnyCalibration = true;
        for(int i=0;i<scene.getObjectCount();++i){
          scene.getObject(i)->setVisible(true);
        }
      }
      break;
    }catch(ICLException &e){
      if(idx == 0){
        ++idx;
      }else{
        gui["status"] = str(e.what());
        bestOfNSaver->next_hook(Camera(),-1);
        break;
      }
    }
  }

  
  
  static DrawHandle3D draw = gui["draw"];
  
  draw = lastFD->getIntermediateImage(gui["iin"].as<std::string>());
  draw->symsize(7);
  for(unsigned int i=0;i<markers.size();++i){
    FoundMarker &m = markers[i];
    draw->linewidth(2);

    const int idx = markers[i].cfgFileIndex;
    if(enabled[idx]){
      draw->color(0,100,255,255);
      draw->fill(0,100,255,100);
    }else{
      draw->color(200,200,200,255);
      draw->fill(100,100,100,100);
    
    }
    draw->polygon(m.fid.getCorners2D());
    if(enabled[idx]){
      draw->color(255,0,0,255);
    }else{
      draw->color(200,200,200,255);
    }

    draw->linewidth(1);
    draw->sym(m.imagePos,'x');
    if(useCorners && m.hasCorners){
      if(deactivatedCenters) draw->color(255,0,0,100);
      draw->sym(m.imageCornerPositions[0],'x');
      draw->sym(m.imageCornerPositions[1],'x');
      draw->sym(m.imageCornerPositions[2],'x');
      draw->sym(m.imageCornerPositions[3],'x');
    }
    if(enabled[idx]){
      draw->color(0,255,0,255);
    }else{
      draw->color(200,200,200,255);
    }
    draw->text(str(m.fid.getID()),m.imagePos.x, m.imagePos.y+12, -10);
  }

  if(havePlane){
    draw->linewidth(1);
    const Point32f p = currentMousePos;
    const std::string t = planeOptionGUI["planeDim"];
    const float o = planeOptionGUI["planeOffset"];
    const float x=t=="x",y=t=="y",z=t=="z";
    PlaneEquation pe(Vec(o*x,o*y,o*z,1),Vec(x,y,z,1));

    const Vec w = scene.getCamera(0).getViewRay(p).getIntersection(pe);
    const Vec wx = x ? Vec(w[0],o,w[2],1) : Vec(o,w[1],w[2],1);
    const Vec wy = y ? Vec(w[0],w[1],o,1) : Vec(w[0],o,w[2],1);
    
    draw->color(0,100,255,255);
    draw->line(p,scene.getCamera(0).project(wx));
    draw->line(p,scene.getCamera(0).project(wy));
    
    const std::string tx = str("(")+str(w[0])+","+str(w[1])+","+str(w[2])+")";
    draw->color(0,0,0,255);
    draw->text(tx,p.x+1,p.y-11,8);
    draw->text(tx,p.x,p.y-12,8);
    draw->color(255,255,255,255);
    draw->text(tx,p.x-1,p.y-13,8);
  }


  draw.render();
}



int main(int n, char **ppc){
  std::vector<std::string> args(ppc+1,ppc+n);
  if(std::find(args.begin(),args.end(),"-cc") != args.end() ||
     std::find(args.begin(),args.end(),"-create-emtpy-config-file") != args.end()){
    std::cout << sample << std::endl;
    return 0;
  }
  
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-config|-c(...) "
                "-camera|-cam(camera_file_to_load) "
                "-create-empty-config-file|-cc "
                "-force-size|-s|-size(WxH) "
                "-output|-o(output-xml-file-name) "
                ,init,run).exec();
}

