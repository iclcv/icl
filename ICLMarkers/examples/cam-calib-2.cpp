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



typedef FixedColVector<float,3> Vec3;

std::map<std::string,SmartPtr<FiducialDetector> > fds;
std::map<std::string,std::map<int,Vec> > posLUT;
FiducialDetector *lastFD = 0; // used for visualization

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
                     "      <data id=\"pos\" type=\"string\">x,y,z</data>\n"
                     "      <data id=\"marker-id\" type=\"int\">id</data>\n"
                     "   </section>\n"
                     "   <!-- more markers -->\n"
                     "   <!-- more markers -->\n"
                     "   <!-- more markers -->\n"
                     "   <data id=\"world-transform\" type=\"string\">4x4-matrix</data>      <!-- optional -->\n"
                     "</config>\n");

void create_new_fd(const std::string &t, std::vector<std::string> &cfgs, std::string &iin){
  fds[t] = new FiducialDetector(t);
  fds[t]->setConfigurableID(t);
  cfgs.push_back(t);
  iin = fds[t]->getIntermediateImageNames();
  lastFD = fds[t].get();
  lastFD->setPropertyValue("css.angle-threshold",180);
  lastFD->setPropertyValue("css.curvature-cutoff",30);
  lastFD->setPropertyValue("css.rc-coefficient",1);
}
                   

void init(){
 
  ConfigFile cfg(*pa("-c"));
  cfg.listContents();
  
  Mat T = Mat::id();
  try{
    T = parse<Mat>(cfg["config.world-transform"]);
  }catch(...){}

  std::vector<std::string> configurables;
  std::string iin;
  
  try{
    std::string s = cfg["config.obj-file"];
    {
      std::ofstream obj("/tmp/tmp-obj-file.obj");
      obj << s << std::endl;
    }
    SceneObject *obj = new SceneObject("/tmp/tmp-obj-file.obj");
    obj->setColor(Primitive::quad,GeomColor(0,100,255,100));
    obj->setColor(Primitive::line,GeomColor(255,0,0,255));
    obj->setVisible(Primitive::line,true);
    obj->setLineWidth(2);
    obj->setTransformation(T);
    scene.addObject(obj);

  }catch(ICLException &e){
    SHOW(e.what());
  }
  system("rm -rf /tmp/tmp-obj-file.obj");

  
  for(int i=0;true;++i){
    cfg.setPrefix("config.grid-"+str(i)+".");  
    try{
      Size s = parse<Size>(cfg["dim"]);
      Vec3 o = parse<Vec3>(cfg["offset"]);
      Vec3 dx = parse<Vec3>(cfg["x-direction"]);
      Vec3 dy = parse<Vec3>(cfg["y-direction"]);
      std::string t = cfg["marker-type"];
      Range32s r = parse<Range32s>(cfg["marker-ids"]);
      ICLASSERT_THROW(r.getLength()+1 == s.getDim(), ICLException("error loading configuration file at given grid " + str(i)
                                                                  + ": given size " +str(s) + " is not compatible to "
                                                                  + "given marker ID range " +str(r) ));
      if(fds.find(t) == fds.end()){
        create_new_fd(t,configurables,iin);
      }
      FiducialDetector &fd = *fds[t];
      fd.loadMarkers(r,t == "bch" ? ParamList("size",Size(50,50)) : ParamList());
      std::cout << "registering " << t << " marker range " << r << std::endl; 

      int id = r.minVal;
      std::map<int, Vec> &lut = posLUT[t];
      std::vector<Vec> vertices;
      for(int y=0;y<s.height;++y){

        for(int x=0;x<s.width;++x){
          Vec3 v = o+dx*x +dy*y;
          if(lut.find(id) != lut.end()) throw ICLException("error loading configuration file at given grid " + str(i)
                                                           +" : the marker ID " + str(id) + " was already used before");
          lut[id++] = T*Vec(v[0],v[1],v[2],1);
          vertices.push_back(T*Vec(v[0],v[1],v[2],1));
        }
      }
      GridSceneObject *go = new GridSceneObject(s.width,s.height,vertices,true,false);
      go->setColor(Primitive::line,GeomColor(255,0,0,180));
      scene.addObject(go);
    }catch(...){ break; }
  }
  
  for(int i=0;true;++i){
    cfg.setPrefix("config.marker-"+str(i)+".");  
    try{
      std::string t = cfg["marker-type"];
      Vec3 p = parse<Vec3>(cfg["pos"]);
      int id = cfg["marker-id"];
      if(fds.find(t) == fds.end()){
        create_new_fd(t,configurables,iin);
      }
      fds[t]->loadMarkers(str(id), t == "bch" ? ParamList("size",Size(50,50)) : ParamList());

      std::cout << "registering single " << t << " marker " << id << std::endl; 
      
      std::map<int, Vec> &lut = posLUT[t];
      if(lut.find(id) != lut.end()) throw ICLException("error loading configuration file at given grid " + str(i)
                                                       +" : the marker ID " + str(id) + " was already used before");
      
      lut[id] = T * Vec(p[0],p[1],p[2], 1);
    }catch(ICLException &e) { SHOW(e.what());  break; }
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
           << "combo(" +iin + ")[@handle=iin@label=visualized image]"
           << tab 
           << "button(chage relative tranformation)[@handle=showRelTransGUI]"
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
           


  scene.addCamera(Camera());
  scene.getCamera(0).setResolution(grabber.grab()->getSize());
  
  DrawHandle3D draw = gui["draw"];
  draw->lock();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  
  gui["showRelTransGUI"].registerCallback(function(&relTransGUI,&GUI::switchVisibility));
}

struct CalibrationMarker{
  Fiducial fid;
  Point32f imagePos;
  Vec worldPos;
};

void run(){
  ButtonHandle showRelTrans = relTransGUI["showRelTrans"];

  const ImgBase *image = grabber.grab();
  
  std::vector<CalibrationMarker> markers;
  for(std::map<std::string,SmartPtr<FiducialDetector> >::iterator it = fds.begin();
      it != fds.end();++it){
    const std::vector<Fiducial> &fids = it->second->detect(image);
    for(unsigned int i=0;i<fids.size();++i){
      
      CalibrationMarker m = { fids[i], Point32f::null, Vec() };
      std::map<int,Vec> &lut = posLUT[it->first];
      std::map<int,Vec>::iterator jt = lut.find(m.fid.getID());
      if(jt != lut.end()){
        m.imagePos = m.fid.getCenter2D();
        m.worldPos = jt->second;
        markers.push_back(m);
      }
    }
  }
  
  Mat T = create_hom_4x4<float>(relTransGUI["rx"].as<float>()*M_PI/4,
                                relTransGUI["ry"].as<float>()*M_PI/4,
                                relTransGUI["rz"].as<float>()*M_PI/4,
                                relTransGUI["tx"],relTransGUI["ty"],relTransGUI["tz"]);
  if(showRelTrans.wasTriggered()){
    std::cout << "current relative transformation is " << std::endl << T << std::endl << std::endl;
  }
  
  std::vector<Vec> xws(markers.size());
  std::vector<Point32f> xis(markers.size());
  for(unsigned int i=0;i<xws.size();++i){
    xws[i] = T * markers[i].worldPos;
    xis[i] = markers[i].imagePos;
  }
  try{
    scene.getCamera(0) = Camera::calibrate_pinv(xws, xis);
    scene.getCamera(0).getRenderParams().viewport = Rect(Point::null,image->getSize());
    scene.getCamera(0).getRenderParams().chipSize = image->getSize();
    scene.setDrawCoordinateFrameEnabled(true);
  }catch(...){}
  
  static DrawHandle3D draw = gui["draw"];
  
  draw = lastFD->getIntermediateImage(gui["iin"].as<std::string>());
  draw->lock();
  draw->reset();
  draw->symsize(5);
  for(unsigned int i=0;i<markers.size();++i){
    CalibrationMarker &m = markers[i];
    draw->linewidth(2);
    draw->color(0,100,255,255);
    draw->fill(0,100,255,100);
    draw->polygon(m.fid.getCorners2D());

    draw->color(255,0,0,255);

    draw->sym(m.imagePos,'*');
    
    draw->text(str(m.worldPos[0]) + " " + str(m.worldPos[1]) + " " + str(m.worldPos[2]), 
               m.imagePos.x, m.imagePos.y, -10);
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
                "[m]-config|-c(config-xml-file-name) "
                "-create-empty-config-file|-cc "
                "-force-size|-s(WxH) "
                "[m]-output|-o(output-xml-file-name) "
                ,init,run).exec();
}

