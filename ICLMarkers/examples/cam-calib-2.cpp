/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/cam-calib-2.cpp                       **
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

#include <ICLQuick/Common.h>
#include <ICLGeom/Scene.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLGeom/GridSceneObject.h>

GUI gui("hsplit");
GUI relTransGUI;
Scene scene;
GenericGrabber grabber;

Point32f currentMousePos;
bool havePlane = false;

void mouse(const MouseEvent &e){
  currentMousePos = e.getPos();
}

typedef FixedColVector<float,3> Vec3;

struct PossibleMarker{
  PossibleMarker():loaded(false){}
  PossibleMarker(const Vec &v):loaded(true),center(v),hasCorners(false){}
  PossibleMarker(const Vec &v, const Vec &a, const Vec &b, const Vec &c, const Vec &d):
    loaded(true),center(v),hasCorners(true){
    corners[0] = a;
    corners[1] = b;   
    corners[2] = c;
    corners[3] = d;
  }
  PossibleMarker &operator=(const Vec &v){
    center = v;
    loaded = true;
    hasCorners = false;
    return *this;
  }
  bool loaded;
  Vec center;
  bool hasCorners;
  Vec corners[4];
};

enum MarkerType {
  BCH,
  AMOEBA
};

SmartPtr<FiducialDetector> fds[2];
std::vector<PossibleMarker> possible[2] = {
  std::vector<PossibleMarker>(4096), 
  std::vector<PossibleMarker>(4096)
};

FiducialDetector *lastFD = 0; // used for visualization
std::vector<std::pair<SceneObject*,Mat> > calibObjs;
//std::vector<SceneObject*> grids;

std::string sample= ("<config>\n"
                     "  <section id=\"grid-0\">\n"
                     "      <data id=\"dim\" type=\"string\">(NumXCells)x(NumYCells)</data>\n"
                     "      <data id=\"offset\" type=\"string\">x,y,z</data>\n"
                     "      <data id=\"x-direction\" type=\"string\">dx1,dy1,dz1</data>\n"
                     "      <data id=\"y-direction\" type=\"string\">dx2,dy2,dz2</data>\n"
                     "      <data id=\"marker-type\" type=\"string\">amoeba|bch</data>\n"
                     "      <data id=\"marker-ids\" type=\"string\">[minID,maxID]</data>\n"
                     "   </section>\n"
                     "    <!-- more grids -->\n"
                     "    <!-- more grids -->\n"
                     "    <!-- more grids -->\n"
                     "   <section id=\"marker-0\">\n"
                     "      <data id=\"marker-type\" type=\"string\">amoeba|bch</data>\n"
                     "      <data id=\"offset\" type=\"string\">x,y,z</data>\n"
                     "      <data id=\"marker-id\" type=\"int\">id</data>\n"
                     "   </section>\n"
                     "   <!-- more markers -->\n"
                     "   <!-- more markers -->\n"
                     "   <!-- more markers -->\n"
                     "   <data id=\"world-transform\" type=\"string\">4x4-matrix</data>      <!-- optional -->\n"
                     "   <data id=\"obj-file\" type=\"string\">\n"
                     "      <!-- optional .obj file content that describes the visual shape of the calibration object -->\n"
                     "   </data>\n"
                     "</config>\n");

FiducialDetector *create_new_fd(MarkerType t, std::vector<std::string> &configurables, std::string &iin){
  static const std::string ts[2] = {"bch","amoeba"};
  FiducialDetector *fd = new FiducialDetector(ts[t]);
  fd->setConfigurableID(ts[t]);
  configurables.push_back(ts[t]);
  iin = fd->getIntermediateImageNames();
  fd->setPropertyValue("css.angle-threshold",180);
  fd->setPropertyValue("css.curvature-cutoff",30);
  fd->setPropertyValue("css.rc-coefficient",1);
  fd->setPropertyValue("thresh.global threshold",-10);
  fd->setPropertyValue("thresh.mask size",30);
  lastFD = fd;
  return fd;
}

void save(){
  std::string filename;
  if(pa("-o")){
    filename = pa("-o").as<std::string>();
  }else{ 
    try{
      filename = saveFileDialog("*.xml","save current camera","./");
    }catch(...){}
  }
  if(filename.length()){
    std::ofstream s(filename.c_str());
    s << scene.getCamera(0);
    std::cout << "current camera is " << scene.getCamera(0) << std::endl;
  }
}                   

SceneObject *planeObj = 0;

Vec set_3_to_1(Vec a){
  a[2] += 1;
  a[3] = 1;
  return a;
}

void change_plane(const std::string &handle){
  if(handle == "planeDim"){
    if(gui["planeDim"].as<std::string>() == "none"){
      gui["planeOffset"].disable();
      gui["planeRadius"].disable();
      gui["planeTicDist"].disable();
      gui["planeColor"].disable();
      gui["planeStatus"] = str("removed");
      scene.removeObject(planeObj);
      planeObj = 0;
      havePlane = false;
      return;
    }else{
      havePlane = true;
      gui["planeOffset"].enable();
      gui["planeRadius"].enable();
      gui["planeTicDist"].enable();
      gui["planeColor"].enable();
    }
  }
  if(planeObj){
    scene.removeObject(planeObj);
    ICL_DELETE(planeObj);
  }
  
  const std::string t = gui["planeDim"];
  const float offset = gui["planeOffset"];
  const float radius = parse<float>(gui["planeRadius"]);
  const float ticDist = gui["planeTicDist"];
  const Color4D c = gui["planeColor"];

  int n = (2*radius) / ticDist;
  if(n * n > 1000000){
    gui["planeStatus"] = str("too many nodes");
    return;
  }else{
    gui["planeStatus"] = str("ok (") + str(n*n) + " nodes)";
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

void init(){
 
  if(!pa("-c")) { pausage("-c is mandatory!"); ::exit(0); } 

  std::vector<std::string> configurables;
  std::string iin;

  for(int c = 0; c <pa("-c").n(); ++c){
    ConfigFile cfg(*pa("-c",c));
    std::cout << "* parsing given configuration file '" << *pa("-c",c) << "'" << std::endl;
    
    Mat T = Mat::id();
    try{
      T = parse<Mat>(cfg["config.world-transform"]);
    }catch(...){}
    
    
    try{
      std::string s;
      try{
        std::string s2 = cfg["config.obj-file"];
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
      o->setTransformation(T);
      scene.addObject(o);
      calibObjs.push_back(std::make_pair(o,T));
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
        std::vector<Vec> vertices;
        
        haveCorners = (mode==ExtractGrids) && (ms != Size32f::null) && (t==BCH);
        
        for(int y=0;y<s.height;++y){
          for(int x=0;x<s.width;++x){
            Vec3 v = o+dx*x +dy*y;
            if(lut[i].loaded) throw ICLException("error loading configuration file at given grid " + str(i)
                                                 +" : the marker ID " + str(id) + " was already used before");
            if(haveCorners){
              Vec3 ul = v + dx1*(ms.width/2) - dy1*(ms.height/2);
              Vec3 ur = v + dx1*(ms.width/2) + dy1*(ms.height/2);
              Vec3 ll = v - dx1*(ms.width/2) + dy1*(ms.height/2);
              Vec3 lr = v - dx1*(ms.width/2) - dy1*(ms.height/2);
              
              lut[id++] = PossibleMarker(T*v.resize<1,4>(1),
                                         T*ul.resize<1,4>(1),
                                         T*ur.resize<1,4>(1),
                                         T*ll.resize<1,4>(1),
                                         T*lr.resize<1,4>(1));
            }else{
              lut[id++] = PossibleMarker(T*Vec(v[0],v[1],v[2],1));
            }
            vertices.push_back(T*Vec(v[0],v[1],v[2],1));
          }
        }
        /*
            if(mode == ExtractGrids){
            SceneObject *so = new GridSceneObject(s.width,s.height,vertices,true,false);
            grids.push_back(so);
            so->setColor(Primitive::line,GeomColor(255,0,0,180));
            scene.addObject(so);
            }
        */
      }catch(ICLException &e){
        ERROR_LOG("Error parsing xml configuration file: '" << *pa("-c",c) << "': " << e.what());
        continue;
      }
    }
  }
  
  grabber.init(pa("-i"));
  
  gui << "draw3D()[@handle=draw@minsize=32x24]";
  
  std::string tabstr = "tab(";
  for(unsigned int i=0;i<configurables.size();++i){
    tabstr += configurables[i]+',';
  }
  tabstr[tabstr.length()-1] = ')';
  GUI tab(tabstr);
  for(unsigned int i=0;i<configurables.size();++i){
    tab << "prop(" + configurables[i] + ")";
  }
  gui << ( GUI("vbox[@minsize=16x1@maxsize=16x100]") 
           << ( GUI("hbox[@maxsize=100x3]") 
                << "combo(" +iin + ")[@handle=iin@label=visualized image]"
                << "slider(0,255,128)[@out=objAlpha@label=object-alpha]"
                )
           << ( GUI("hbox[@maxsize=100x3]") 
                << "checkbox(use corners,checked)[@out=useCorners]"
                << "checkbox(show CS,checked)[@out=showCS]"
                << "label( )[@handle=error@label=error]"
                )
           << ( GUI("vbox[@label=plane@maxsize=100x10]")
                << ( GUI("hbox")
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
                )

           << tab 
           << ( GUI("hbox[@maxsize=100x3]") 
                << "button(chage relative tranformation)[@handle=showRelTransGUI]"
                << "button(save camera)[handle=save]"
                )
           )
      << "!show";
  
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
  
           

  gui["save"].registerCallback(save);

  scene.addCamera(Camera());
  scene.getCamera(0).setResolution(grabber.grab()->getSize());
  
  DrawHandle3D draw = gui["draw"];
  draw->lock();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  
  gui["showRelTransGUI"].registerCallback(function(&relTransGUI,&GUI::switchVisibility));
  
  gui["planeOffset"].disable();
  gui["planeRadius"].disable();
  gui["planeTicDist"].disable();
  gui["planeColor"].disable();
  gui.registerCallback(change_plane,"planeOffset,planeRadius,planeTicDist,planeDim,planeColor");
  
  gui["draw"].install(new MouseHandler(mouse));
}

struct FoundMarker{
  FoundMarker(){}
  FoundMarker(Fiducial fid, const Point32f &imagePos, const Vec &worldPos):
    fid(fid),imagePos(imagePos),worldPos(worldPos),hasCorners(false){}
  FoundMarker(Fiducial fid, const Point32f &imagePos, const Vec &worldPos,
              const Point32f imageCornerPositions[4],
              const Vec worldCornerPositions[4]):
    fid(fid),imagePos(imagePos),worldPos(worldPos),hasCorners(true){
    std::copy(imageCornerPositions,imageCornerPositions+4,this->imageCornerPositions);
    std::copy(worldCornerPositions,worldCornerPositions+4,this->worldCornerPositions);
  }
  
  Fiducial fid;
  Point32f imagePos;
  Vec worldPos;
  bool hasCorners;
  Point32f imageCornerPositions[4];  
  Vec worldCornerPositions[4];  
};

void run(){
  scene.lock();
  scene.setDrawCoordinateFrameEnabled(gui["showCS"]);
  scene.unlock();

  for(unsigned int i=0;i<calibObjs.size();++i){
    SceneObject *calibObj = calibObjs[i].first;
    const int calibObjAlpha = gui["objAlpha"];
    if(calibObjAlpha){
      calibObj->setVisible(Primitive::quad,true);
      calibObj->setVisible(Primitive::triangle,true);
      calibObj->setVisible(Primitive::polygon,true);
      calibObj->setColor(Primitive::quad,GeomColor(0,100,255,calibObjAlpha));
      calibObj->setColor(Primitive::triangle,GeomColor(0,100,255,calibObjAlpha));
      calibObj->setColor(Primitive::polygon,GeomColor(0,100,255,calibObjAlpha));
    }else{
      calibObj->setVisible(Primitive::quad,false);
      calibObj->setVisible(Primitive::triangle,false);
      calibObj->setVisible(Primitive::polygon,false);
    }
  } 

  ButtonHandle showRelTrans = relTransGUI["showRelTrans"];

  const ImgBase *image = grabber.grab();
  
  std::vector<FoundMarker> markers;
  for(int x=0;x<2;++x){
    if(!fds[x]) continue;
    const std::vector<Fiducial> &fids = fds[x]->detect(image);
    for(unsigned int i=0;i<fids.size();++i){
      //FoundMarker m = { fids[i], Point32f::null, Vec() };
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
                                        imagePositions,p.corners));
        }else{
          markers.push_back(FoundMarker(fids[i],fids[i].getCenter2D(),p.center));
        }
      }else{
        ERROR_LOG("the fiducial detector detected a marker with ID " 
                  << fids[i].getID() << " which was not registered "
                  << "in this tool (this should actually not happen)");
      }
    }
  }
  
  Mat T = create_hom_4x4<float>(relTransGUI["rx"].as<float>()*M_PI/4,
                                relTransGUI["ry"].as<float>()*M_PI/4,
                                relTransGUI["rz"].as<float>()*M_PI/4,
                                relTransGUI["tx"],relTransGUI["ty"],relTransGUI["tz"]);
  
  for(unsigned int i=0;i<calibObjs.size();++i){
    calibObjs[i].first->setTransformation(T * calibObjs[i].second);
  }
  //  for(unsigned int i=0;i<grids.size();++i){
  //  grids[i]->setTransformation(T);
  //}
  
  
  if(showRelTrans.wasTriggered()){
    std::cout << "current relative transformation is:" << std::endl << T << std::endl;
    for(unsigned int i=0;i<calibObjs.size();++i){
      std::cout << "combined transformation matrix for calibration object '" << pa("-c",i) 
                << "' is:" << std::endl << (T * calibObjs[i].second) << std::endl;
    }
  }
  
  std::vector<Vec> xws;
  std::vector<Point32f> xis;
  bool useCorners = gui["useCorners"];
  for(unsigned int i=0;i<markers.size();++i){
    xws.push_back(T * markers[i].worldPos);
    xis.push_back(markers[i].imagePos);


    if(useCorners && markers[i].hasCorners){
      for(int j=0;j<4;++j){
        xws.push_back(T*markers[i].worldCornerPositions[j]);
        xis.push_back(markers[i].imageCornerPositions[j]);
      }
    }


  }
  try{
    Camera &cam = scene.getCamera(0);
    cam = Camera::calibrate_pinv(xws, xis);
    cam.getRenderParams().viewport = Rect(Point::null,image->getSize());
    cam.getRenderParams().chipSize = image->getSize();
    scene.setDrawCoordinateFrameEnabled(true);
    
    float error = 0;
    
    for(unsigned int i=0;i<xws.size();++i){
      error += xis[i].distanceTo(cam.project(xws[i]));
    }
    gui["error"] = error/xws.size();

  }catch(ICLException &e){
    SHOW(e.what());
  }

  
  
  static DrawHandle3D draw = gui["draw"];
  
  draw = lastFD->getIntermediateImage(gui["iin"].as<std::string>());
  draw->lock();
  draw->reset();
  draw->symsize(7);
  for(unsigned int i=0;i<markers.size();++i){
    FoundMarker &m = markers[i];
    draw->linewidth(2);
    draw->color(0,100,255,255);
    draw->fill(0,100,255,100);
    draw->polygon(m.fid.getCorners2D());
    draw->color(255,0,0,255);

    draw->linewidth(1);
    draw->sym(m.imagePos,'x');
    if(m.hasCorners){
      draw->sym(m.imageCornerPositions[0],'x');
      draw->sym(m.imageCornerPositions[1],'x');
      draw->sym(m.imageCornerPositions[2],'x');
      draw->sym(m.imageCornerPositions[3],'x');
    }
    draw->color(0,255,0,255);
    draw->text(str(m.fid.getID()),m.imagePos.x, m.imagePos.y+12, -10);
  }

  if(havePlane){
    draw->linewidth(1);
    const Point32f p = currentMousePos;
    const std::string t = gui["planeDim"];
    const float o = gui["planeOffset"];
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


  draw->unlock();
  draw.update();
}



int main(int n, char **ppc){
  std::vector<std::string> args(ppc+1,ppc+n);
  if(std::find(args.begin(),args.end(),"-cc") != args.end() ||
     std::find(args.begin(),args.end(),"-create-emtpy-config-file") != args.end()){
    std::cout << sample << std::endl;
    return 0;
  }
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "[m]-config|-c(...) "
                "-create-empty-config-file|-cc "
                "-force-size|-s(WxH) "
                "-output|-o(output-xml-file-name) "
                ,init,run).exec();
}

