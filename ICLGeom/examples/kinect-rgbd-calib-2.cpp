/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2011 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/kinect-rgbd-calib.cpp                 **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann, Christof Elbrechter                  **
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
#include <ICLUtils/ConfigFile.h>
#include <ICLMarkers/FiducialDetector.h>

#include <ICLCore/Random.h>
#include <ICLUtils/ConsoleProgress.h>
#include <ICLUtils/SimplexOptimizer.h>

#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>

#include <ICLCC/CCFunctions.h>
#include <ICLFilter/MedianOp.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/Geom.h>

#include <ICLGeom/RGBDImageSceneObject.h>
#include <fstream>

static float sprod(const Vec &a, const Vec &b){
  return a[0]*b[0] +a[1]*b[1] +a[2]*b[2];
}


ImageUndistortion udistRGB;
ImageUndistortion udistIR;

GUI gui("hsplit");
GenericGrabber grabDepth, grabColor;

SmartPtr<FiducialDetector> fid,fid2;

struct Correspondence{
  float x,y,d,  a,b;
  std::string name;
};

std::vector<Correspondence> correspondences;

//RGBDMapping mapping;

Img32f matchImage(Size(320,240), formatRGB);

GUI fidDetectorPropertyGUI("hsplit");



Scene scene;
#if 0


struct Bla : public SceneObject{
  std::vector<Vec> m_normedViewRayDirs; // (x,y,z)normed and norm
  Vec m_viewRayOffset;
  
  inline float getNormFactor(const ViewRay &a, const ViewRay &b){
    return sprod(a.direction,b.direction)/(norm3(a.direction)*norm3(b.direction));
  }

  Bla(){
    m_vertices.resize(640*480, Vec(0,0,0,1));
    m_vertexColors.resize(640*480);
    setLockingEnabled(true);
    setVisible(Primitive::vertex,true);
    setPointSize(3);
    setPointSmoothingEnabled(false);
    
    Camera cam = pa("-cam") ? Camera(*pa("-cam")) : Camera();

    Array2D<ViewRay> viewRays = cam.getAllViewRays();
    m_viewRayOffset = viewRays(0,0).offset;
    m_normedViewRayDirs.resize(640*480);
    
    for(int y=0;y<480;++y){
      for(int x=0;x<640;++x){
        const int idx = x + 640 * y;
        const ViewRay &v = viewRays[idx];
        Vec &n = m_normedViewRayDirs[idx];
        
        float norm = 1.0/getNormFactor(v,viewRays(319,239));
        
        n[0] = v.direction[0];
        n[1] = v.direction[1];
        n[2] = v.direction[2];
        n[3] = norm;
      }
    }
  }

  static inline float depth_to_distance_mm(int d){
    return 1.046 * (d==2047 ? 0 : 1000. / (d * -0.0030711016 + 3.3309495161));
  }
  
  inline float getDepth(int d, float norm) const{
    return depth_to_distance_mm(d) * norm;
  }
  inline float getDepth(int d, int x, int y) const{ 
    return getDepth(d,m_normedViewRayDirs[x+640*y][3]);
  }
  
  void update(const Channel32f &D, 
              const Channel32f &r,
              const Channel32f &g,
              const Channel32f &b){
    lock();
    static const Rect imageRect(0,0,640,480);
    
    Time now = Time::now();
    
    for(int y=0;y<480;++y){
      // too much page misses if we add parallellism
#pragma omp parallel num_threads(4)
      
      {
#pragma omp for 
        for(int x=0;x<640;++x){
          int idx = x + 640 * y;
          const Vec &dir = m_normedViewRayDirs[idx];
          const float depthValue = getDepth( D[idx], dir[3]);
          
          Vec &v = m_vertices[idx];
          v[0] = m_viewRayOffset[0] + depthValue * dir[0];
          v[1] = m_viewRayOffset[1] + depthValue * dir[1];
          v[2] = m_viewRayOffset[2] + depthValue * dir[2];
          
          Point p = mapping.apply(x,y,depthValue);
          
          //          const float phInv = 1.0/ (Q(0,3) * x + Q(1,3) * y + Q(2,3) * depthValue + Q(3,3));
          // const int px = phInv * ( Q(0,0) * x + Q(1,0) * y + Q(2,0) * depthValue + Q(3,0) );
          // const int py = phInv * ( Q(0,1) * x + Q(1,1) * y + Q(2,1) * depthValue + Q(3,1) );
          
          GeomColor &c = m_vertexColors[idx];        
          if(imageRect.contains(p.x, p.y)){
            static const float FACTOR = 1.0/255;
            c[0] = r(p.x,p.y) * FACTOR;
            c[1] = g(p.x,p.y) * FACTOR;
            c[2] = g(p.x,p.y) * FACTOR;
            c[3] = 1;
          }else{
            c[3] = 0;
          }
        }
      }
    }
    SHOW( (Time::now()-now).toMilliSecondsDouble() );
    unlock();

  }
} *obj = 0;

#endif

RGBDImageSceneObject *obj = 0;


void init(){
  const Size size = pa("-size");
  
  RGBDMapping m = RGBDImageSceneObject::get_default_kinect_rgbd_mapping(size);
  Camera cam = RGBDImageSceneObject::get_default_kinect_camera(size);
  if(pa("-mapping")) m = RGBDMapping(*pa("-mapping"));
  if(pa("-cam")) cam = Camera(*pa("-cam"));
  
  obj = new RGBDImageSceneObject(size,m,cam);
  obj->setConfigurableID("obj");

  matchImage.setSize(size);
  grabDepth.init("kinectd","kinectd=0");
  grabColor.init("kinectc","kinectc=0");
  grabDepth.useDesired(depth32f, size, formatGray);
  grabColor.useDesired(depth32f, size, formatGray);
  
  fid = new FiducialDetector(pa("-m").as<std::string>(), 
                             pa("-m",1).as<std::string>(), 
                             ParamList("size",(*pa("-m",2)) ) );
  
  fid2 = new FiducialDetector(pa("-m").as<std::string>(), 
                              pa("-m",1).as<std::string>(), 
                              ParamList("size",(*pa("-m",2)) ) );
  fid2->setPropertyValue("thresh.global threshold","-5.4");
  fid2->setPropertyValue("thresh.mask size","60");
  
  
  fid->setConfigurableID("fid");
  fid2->setConfigurableID("fid2");    
      
  gui << (GUI("vsplit")
          << "draw[@handle=color@minsize=16x12@label=color image]"
          << "draw[@handle=ir@minsize=16x12@label=IR image]"
         )
      << (GUI("vsplit")
          << "draw[@handle=match@minsize=16x12@label=result visualization]"
          << "draw3D[@handle=depth@minsize=16x12@label=depth image]"
         )
      << ( GUI("vbox[@minsize=12x2]")
           << "togglebutton(pause, run)[@out=paused]"
           << ("combo(R/G mapped_RGB/depth,|mapped_RGB - IR|,!R/B mapped_RGB/IR)"
               "[@handle=resultVis@label=result visualization method@maxsize=99x3]")
           << "combo(least squares,ransac, ransac+simplex)[@handle=optMethod@label=optimization method]"
           << "checkbox(use corners,unchecked)[@out=useCorners]"
           << "button(add points)[@handle=addPoints]"
           << "button(calculate homography)[@handle=calcHomo]"
           << "button(save homography)[@handle=saveHomo]"
           << "button(clear points and reset)[@handle=clearPoints]"
           << "checkbox(view only,checked)[@out=viewOnly]"
           << "button(more ...)[@handle=more]" 
           << "fps(10)[@handle=fps]"
           )
      << "!show";
  
  fidDetectorPropertyGUI  << (GUI("vbox[@maxsize=16x100]") 
                              << ("combo(" + fid->getIntermediateImageNames() + ")"
                                  "[@maxsize=100x2@handle=vis@label=color image]")
                              << "prop(fid)")
                          << (GUI("vbox[@maxsize=16x100]") 
                              << ("combo(" + fid2->getIntermediateImageNames() + ")"
                                  "[@maxsize=100x2@handle=vis2@label=infrared image]")
                              << "prop(fid2)")
                          << "prop(obj)"
                          << "!create";
  gui["more"].registerCallback(function(&fidDetectorPropertyGUI,&GUI::switchVisibility));
        
  try{
    fid->setPropertyValue("thresh.algorithm","tiled linear");
    
    fid2->setPropertyValue("thresh.algorithm","region mean");
    fid2->setPropertyValue("matching algorithm","gray ncc");
    fid2->setPropertyValue("thresh.global threshold","-0.7");
    fid2->setPropertyValue("thresh.mask size","19");
    fid2->setPropertyValue("quads.minimum region size","360");

  }catch(ICLException &e){
    WARNING_LOG("exception caught while setting initial parameters: " << e.what());
  }
  
  gui_DrawHandle(match);
  match->setRangeMode(ICLWidget::rmAuto);
		
  if(pa("-rgb-udist")){
    string fn1 = pa("-rgb-udist");
    udistRGB=ImageUndistortion(fn1);
    grabColor.enableUndistortion(udistRGB);
    std::cout<<"RGB-UNDISTORTION: --- "<<fn1<<" --- "<<std::endl<<udistRGB<<std::endl;
  }
  if(pa("-ir-udist")){
    string fn2 = pa("-ir-udist");
    udistIR=ImageUndistortion(fn2);
    grabDepth.enableUndistortion(udistIR);
    std::cout<<"IR-UNDISTORTION: --- "<<fn2<<" --- "<<std::endl<<udistIR<<std::endl;
  }

 
  scene.addCamera(cam);
  scene.addObject(obj);
  
  float params[] = {0,0,0,1000};
  SceneObject *cube = new SceneObject("cube",params);
  scene.addObject(cube);
  
  DrawHandle3D draw = gui["depth"];
  draw->lock();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  
  draw->install(scene.getMouseHandler(0));  
  
  scene.removeObject(cube);
  
 
}

void visualizeMatches(DrawHandle &draw, const std::vector<Fiducial> &fids, 
                      FiducialDetector *fd, const std::string &imageName, 
                      bool showCorrespondences){
  //draw = fd->getIntermediateImage(imageName);
  draw->lock();
  draw->reset();
  draw->linewidth(1);
  draw->symsize(20);
  for(unsigned int i=0;i<fids.size();++i){
    draw->color(0,100,255,200);
    draw->linestrip(fids[i].getCorners2D());
    draw->color(255,0,0,255);
    draw->sym(fids[i].getCenter2D(),'x');
  }
  draw->color(0,255,0,255);
  draw->fill(0,0,0,0);
  
  if(showCorrespondences){
    for(unsigned int i=0;i<correspondences.size(); ++i){
      Correspondence &c = correspondences[i];
      draw->sym(Point32f(c.a,c.b), 'o');
    }
  }
  
  draw->unlock();
}


void run(){
  gui["fps"].update();

  bool viewOnly = gui["viewOnly"];
  gui_ButtonHandle(addPoints);
  gui_ButtonHandle(clearPoints);
  gui_ButtonHandle(calcHomo);
  gui_ButtonHandle(saveHomo);
    
  Size size = pa("-size");
  static Img8u C, IR, IRmed;
  static Img32f D;

  grabDepth.grab(bpp(D));
  grabDepth.grab();
  grabColor.disableUndistortion();
  grabColor.useDesired(formatRGB);
  grabColor.useDesired(depth8u);
  grabColor.setProperty("format","Color Image (24Bit RGB)");

  if(pa("-rgb-udist")){
    grabColor.enableUndistortion(udistRGB);
  }
  if(!viewOnly){
    grabColor.grab();
  }
  grabColor.grab(bpp(C));

  if(!viewOnly){
    grabColor.disableUndistortion();
    grabColor.useDesired(formatGray);
    grabColor.useDesired(depth8u);
    grabColor.setProperty("format","IR Image (8Bit)");
    
    
    if(pa("-ir-udist")){
      grabColor.enableUndistortion(udistIR);
    }
    grabColor.grab();
    grabColor.grab(bpp(IR));

    static MedianOp median(Size(5,5));
    median.setClipToROI(false);
    median.apply(&IR, bpp(IRmed));
        
    gui["ir"] = IRmed; 
  }
  


  gui["color"] = C;
        
  std::vector<Fiducial> fids, fids2;
  
  obj->update(D,C);
  const std::vector<Vec> & van = obj->getViewRaysAndNorms();
  const RGBDMapping M = obj->getMapping();
  
  const Channel32f dd = D[0];
  if(!viewOnly){
    static Img8u Cgray(C.getSize(),formatGray);
    cc(&C,&Cgray);
    
    fids = fid->detect(&Cgray);
    fids2 = fid2->detect(&IRmed);
    
    static DrawHandle color = gui["color"];
    static DrawHandle ir = gui["ir"];
    
    visualizeMatches(color,fids, fid.get(), fidDetectorPropertyGUI["vis"], true);
    visualizeMatches(ir,fids2, fid2.get(), fidDetectorPropertyGUI["vis2"], false);
    
    const Rect r = C.getImageRect();
    Channel8u c = C[0];
    
    { /// visualize the (CAM) homography ...
      //Mat T = H_cam.getCSTransformationMatrix();
      //Mat P = H_cam.getProjectionMatrix();
      //Mat Q = P*T;
     
      matchImage.setChannels(2);
      
      std::copy(IR.begin(0),IR.end(0),matchImage.begin(1));
      
      Channel32f miC0 = matchImage[0];
      Channel32f miC1 = matchImage[1];
      
      for(int y=0; y<size.height; y++){
        for(int x=0; x<size.width; x++){
          const int idx = x + size.width * y;
          float depthValue = dd(x,y) * van[idx][3]; //obj->getDepth(dd(x,y),x,y);
          if(!depthValue){
            miC0(x,y) = 0;
            miC1(x,y) = 0;
          }else{
            Point p = M.apply(x,y,depthValue); //H_cam.project(Vec(x,y,depthValue,1));
            //        Vec pp = Q*Vec(x,y,dd(x,y),1);
            //Point p(pp[0]/pp[3], pp[1]/pp[3]);
            
            //Point p = H.apply_int(Point32f(x,y)); TODO !!
            miC0(x,y) = r.contains(p.x,p.y) ? c(p.x,p.y) : 0.0f;
          }
        }
      }
    }
    
    matchImage.normalizeAllChannels(Range32f(0,255));
    
    gui["match"] = matchImage;
  }


  
  gui["depth"].update();
  gui["color"].update();
  gui["ir"].update();
  gui["match"].update();
        
  while(gui["paused"]){
    Thread::msleep(10);
  }
   
  if(clearPoints.wasTriggered()){
    correspondences.clear();
    //(FixedMatrix<float,3,3>&)H=FixedMatrix<float,3,3>::id();
  }
   
  if(!viewOnly){
    if(addPoints.wasTriggered()){
      
      bool useCorners = gui["useCorners"];  
      
      for(unsigned int a=0;a<fids.size(); a++){
        const Fiducial &fa = fids[a];
        for(unsigned int b=0; b<fids2.size(); b++){
          const Fiducial &fb = fids2[b];
          
          if(fa.getName()==fb.getName()){
            float bx = fb.getCenter2D().x, by = fb.getCenter2D().y;
            Correspondence c = { bx,
                                 by,
                                 dd(bx,by) * van[bx + size.width*by][3],// obj->getDepth(D(bx,by,0), bx, by),
                                 fa.getCenter2D().x, 
                                 fa.getCenter2D().y,
                                 fa.getName() };
            if(c.d) {
              correspondences.push_back(c);          
            }

            if(useCorners){
              const std::vector<Fiducial::KeyPoint> &ka = fa.getKeyPoints2D();
              const std::vector<Fiducial::KeyPoint> &kb = fb.getKeyPoints2D();
              for(unsigned int i=0;i<ka.size();++i){
                float ax = ka[i].imagePos.x, ay = ka[i].imagePos.y;
                float bx = kb[i].imagePos.x, by = kb[i].imagePos.y;
                
                Correspondence c = { bx,
                                     by,
                                     dd(bx,by) * van[bx + size.width*by][3],//obj->getDepth(D(bx,by,0), bx, by),
                                     ax,
                                     ay,
                                     fa.getName() + "key-point " + str(i)};
                if(c.d) {
                  correspondences.push_back(c);          
                }
              }
            }
            break;
          }
        }
      }
    }
    if(calcHomo.wasTriggered()){
      int N = correspondences.size();
      std::vector<Point32f> Xis(N);
      std::vector<Vec> Xws(N);
      for(int i=0;i<N;++i){
        const Correspondence &c = correspondences[i];
        Xis[i] = Point32f(c.a,c.b);
        Xws[i] = Vec(c.x,c.y,c.d,1);
      }
      obj->setMapping(RGBDMapping(Xws,Xis));
      obj->getMapping().save("last-mapping.xml");
    }
    
    if(saveHomo.wasTriggered()){
      obj->getMapping().save("saved-mapping.xml");
      std::cout << "wrote saved-mapping.xml" << std::endl;
    }
  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,
                "-size|-s(Size=VGA) "
                "-rgb-udist(fn1) "
                "-ir-udist(fn2) "
                "-marker-type|-m(type=bch,whichToLoad=[0-1000],size=50x50) "
                "-mapping(mapping-filename) "
                "-cam(camerafilename)",
                init,run).exec();
}
