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
#include <ICLGeom/GridSceneObject.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>
#include <ICLGeom/Geom.h>
#include <ICLMath/Homography2D.h>
#include <fstream>

struct Marker{
  int id;
  Point32f center;
  Point32f corners[4];
};

struct Grid{
  Grid(int idoffset, int nx, int ny,  Size32f size, float dxmm, float dymm):
    idoffset(idoffset),nx(nx),ny(ny),size(size),dxmm(dxmm),dymm(dymm){
  
    markers = Array2D<Marker>(nx,ny);

    obj = new SceneObject;
    
    Point32f d(dxmm,dymm);
    Point32f s(size.width/2,size.height/2);

    
    for(int y=0,k=0;y<ny;++y){
      for(int x=0;x<nx;++x,++k){
        Marker &m = markers(x,y);
        m.center = d.transform(x,y);
        m.id = idoffset + x + y*nx;
        m.corners[0] = m.center + s.transform(1,-1);
        m.corners[1] = m.center + s.transform(1,1);
        m.corners[2] = m.center + s.transform(-1,1);
        m.corners[3] = m.center + s.transform(-1,-1); //??
      
        obj->addVertex(Vec(m.corners[0].x, m.corners[0].y, 0, 1));
        obj->addVertex(Vec(m.corners[1].x, m.corners[1].y, 0, 1));
        obj->addVertex(Vec(m.corners[2].x, m.corners[2].y, 0, 1));
        obj->addVertex(Vec(m.corners[3].x, m.corners[3].y, 0, 1));
        
        obj->addLine(k*4,k*4+1, geom_blue());
        obj->addLine(k*4+1,k*4+2, geom_blue());
        obj->addLine(k*4+2,k*4+3, geom_blue());
        obj->addLine(k*4+3,k*4, geom_blue());
      }
    }
    
    plane = new SceneObject;
    
    Marker m = markers(0,0);
    plane->addVertex(Vec(m.corners[3].x, m.corners[3].y, 0,1));
    
    m = markers(nx-1,0);
    plane->addVertex(Vec(m.corners[0].x, m.corners[0].y, 0,1));
    
    m = markers(nx-1,ny-1);
    plane->addVertex(Vec(m.corners[1].x, m.corners[1].y, 0,1));
    
    m = markers(0,ny-1);
    plane->addVertex(Vec(m.corners[2].x, m.corners[2].y, 0,1));
    
    plane->addQuad(0,1,2,3,geom_blue(100));
    plane->addLine(0,1,geom_blue(200));  
    plane->addLine(1,2,geom_blue(200));  
    plane->addLine(2,3,geom_blue(200));  
    plane->addLine(3,0,geom_blue(200));  
    
  }
  
  Mat T;
  int idoffset;
  int nx;
  int ny;
  Size32f size;
  float dxmm;
  float dymm;
  SmartPtr<SceneObject> obj;
  SmartPtr<SceneObject> plane;
  
  void clear(){
    modelPoints.clear();
    imagePoints.clear();
    objPoints.clear();
  }
  
  bool contains(int id){
    return (id >= idoffset && id < idoffset+nx*ny);
  }
  
  Marker &getMarker(int id){
    if(!contains(id)) throw ICLException("wrong marker ID");
    id-=idoffset;
    return markers(id%nx,id/nx);
  }
  
  void add(const Fiducial &f){
    Marker &m = getMarker(f.getID());
    const std::vector<Fiducial::KeyPoint> &ks = f.getKeyPoints2D();
    if(ks.size() != 4) throw ICLException("markers with 4 keypoints needed!");
    for(int i=0;i<4;++i){
      modelPoints.push_back(m.corners[i]);
      objPoints.push_back(Vec(m.corners[i].x, m.corners[i].y, 0, 1));
      imagePoints.push_back(ks[i].imagePos);
    }
  }
  
  Array2D<Marker> markers;
  
  std::vector<Point32f> modelPoints;
  std::vector<Point32f> imagePoints;
  std::vector<Vec>      objPoints;
  
};

GenericGrabber grabber;
HSplit gui;
GUI rel;
Scene scene;
FiducialDetector fd("bch");
ComplexCoordinateFrameSceneObject *CS = 0;
CoplanarPointPoseEstimator cpe(CoplanarPointPoseEstimator::worldFrame,
                               CoplanarPointPoseEstimator::SamplingCoarse);
std::vector<Grid> grids;

void init(){
  if(pa("-p")){
    fd.loadProperties(*pa("-p"));
  }

  std::ostringstream gridnames;
  ProgArg p = pa("-g");
  for(int i=0;i<p.n();++i){
    std::string g = p[i];
    for(size_t j=0;j<g.length();++j){
      if(g[j] == ',') g[j] = ' ';
    }
    gridnames << "Grid " << i << ": [" << g << "]"<< (i < p.n()-1 ? "," : "");
  }
  
  
  gui << Draw3D().handle("draw").minSize(32,24)
      << (VBox().maxSize(12,99).minSize(12,1)
          << Button("save cam").handle("save")
          << Button("use cam").handle("use").tooltip("aligns the world frame with the object frame")
          << (VBox().label("reference frame")
              << Combo(gridnames.str()).handle("ref").label("relative to grid")
              << CheckBox("centered at grid",true).handle("cen")
              << Button("relative transform").handle("rel")
              )
          << Disp(4,4).label("current object transform").handle("T")
          )
      << Show();

  rel << ( VBox().label("rel-transformation")
           << ( HBox()
                << Spinner(0,8,pa("-i",0)).label("x-rotation *pi/4").out("rx")
                << Spinner(0,8,pa("-i",1)).label("y-rotation *pi/4").out("ry")
                << Spinner(0,8,pa("-i",2)).label("z-rotation *pi/4").out("rz")
                )
           << ( HBox()
                << Float(-100000,100000,pa("-i",3)).label("x-offset").out("tx")
                << Float(-100000,100000,pa("-i",4)).label("y-offset").out("ty")
                << Float(-100000,100000,pa("-i",5)).label("z-offset").out("tz")
                )
           )
  //<< Button("show transformation matrix").handle("showRelTrans") 
      << Create();

  gui["rel"].registerCallback(function(rel,&GUI::switchVisibility)); 
  
  grabber.init(pa("-i"));
  
  scene.addCamera(*pa("-c"));
  
  for(int i=0;i<p.n();++i){
    std::vector<std::string> ts = tok(p[i],",");
    if(ts.size() != 6) {
      pa_show_usage("invalid token for -grid");
      ::exit(-1);
    }
    Grid g(parse<int>(ts[0]),
           parse<int>(ts[1]),
           parse<int>(ts[2]),
           parse<Size32f>(ts[3]),
           parse<float>(ts[4]),
           parse<float>(ts[5]) );
    grids.push_back(g);
    
    scene.addObject(g.obj.get());
    scene.addObject(g.plane.get());
    
    fd.loadMarkers(Range32s(g.idoffset,g.idoffset+g.nx*g.ny-1),"size="+str(g.size)); 
  }  

  CS = new ComplexCoordinateFrameSceneObject;
  scene.addObject(CS);

  scene.setBounds(1000);
  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
  
}

float r2(float x){
  return float(round(x*100))*0.01;
}
float r1(float x){
  return float(round(x*10))*0.1;
}

void run(){
  static DrawHandle3D draw = gui["draw"];
  const ImgBase *image = grabber.grab();
  
  draw = image;

  std::vector<Fiducial> fids = fd.detect(image);
  for(size_t i=0;i<grids.size();++i){
    grids[i].clear();
  }

  draw->linewidth(1);
  draw->color(255,0,0,255);
  draw->fill(255,0,0,20);
  for(size_t i=0;i<fids.size();++i){
    Fiducial &f = fids[i];
    int id = f.getID();
    draw->polygon(f.getCorners2D());
    for(size_t j=0;j<grids.size();++j){
      if(grids[j].contains(id)){
        grids[j].add(f);
      }
    }
  }
  for(size_t i=0;i<grids.size();++i){
    Grid &g = grids[i];
    if(!g.modelPoints.size()) continue;
    g.T = cpe.getPose(g.modelPoints.size(),g.modelPoints.data(), g.imagePoints.data(), scene.getCamera(0));
    //    g.T = optimize_pose(g.T, g.objPoints, g.imagePoints, camera);
    g.obj->setTransformation(g.T);
    g.plane->setTransformation(g.T);
  }
  
  Grid g = grids[gui["ref"].as<int>()];
  
  Mat T = g.T;
  if(gui["cen"]){
    float dx = (g.nx-1) * g.dxmm * 0.5;
    float dy = (g.ny-1) * g.dymm * 0.5;
    T = T * create_hom_4x4<float>(0,0,0, dx,dy,0);
  }

  const Mat R = create_hom_4x4<float>(rel["rx"].as<float>()*M_PI/4,
                                      rel["ry"].as<float>()*M_PI/4,
                                      rel["rz"].as<float>()*M_PI/4,
                                      rel["tx"],rel["ty"],rel["tz"]);
  T = T * R;

  CS->setTransformation(T);
  
  static DispHandle disp = gui.get<DispHandle>("T");
  static ButtonHandle save = gui["save"];
  static ButtonHandle use = gui["use"];
  
  for(int x=0;x<4;++x){
    for(int y=0;y<4;++y){
      disp(x,y) = x == 3 ? r1(T(x,y)) : r2(T(x,y));
    }
  }
  
  
  if(use.wasTriggered()){
    scene.getCamera(0).setWorldFrame(T);
  }
  if(save.wasTriggered()){
    struct Evt : public ICLApp::AsynchronousEvent{
      Mat T;
      Camera cam;
      Evt(const Mat &T, const Camera &cam):T(T), cam(cam){}
      virtual void execute() {
        try{
          std::string fn = saveFileDialog("XML-Files (*.xml)");
          cam.setWorldFrame(T);
          std::ofstream file(fn.c_str());
          file << cam;
        }catch(...){}
      }
    };
    
    ICLApp::instance()->executeInGUIThread(new Evt(T, scene.getCamera(0)));
  }
  
  draw.render();
}

int main(int n, char **a){
  pa_explain("-g","list of grid tokens 'ID-offset,nx,ny,marker-size,dx,dy' size units are in mm");
  pa_explain("-t","gives initial transform paramters (rotation is given in integer units of PI/2)");
  return ICLApp(n,a,"[m]-input|-i(2) "
                "[m]-initial-camera|-c(1) "
                "[m]-grids|-g(...) -fiducial-detector-props|-p(filename)"
                "-initial-relative-transform|-t(rx=0,ry=0,rz=0,tx=0,ty=0,tz=0)",init,run).exec();
}



