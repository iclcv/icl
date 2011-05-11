#include <ICLQuick/Common.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

#include <ICLMarkers/FiducialDetector.h>

GUI gui("hsplit");

GenericGrabber grabber;
FiducialDetector *fid = 0;
Scene scene;

struct Obj : public SceneObject{
  Obj(){
    addChild(new ComplexCoordinateFrameSceneObject);
    addCube(0,0,48,96);
  }
} *obj = 0;

void init(){
  fid = new FiducialDetector(pa("-m").as<std::string>(), 
                             pa("-m",1).as<std::string>(), 
                             ParamList("size",(*pa("-m",2)) ) );
  
  fid->setConfigurableID("fid");
  gui << "draw3D("+*pa("-size")+")[@handle=draw@minsize=16x12]"
      << (GUI("vbox[@maxsize=16x100]") 
          << ("combo(" + fid->getIntermediateImageNames() + ")"
              "[@maxsize=100x2@handle=vis@label=visualization]")
          << "prop(fid)"
          << "togglebutton(running,pause)[@out=pause]"
         )
      << "!show";

  grabber.init(pa("-input"));
  grabber.useDesired<Size>(pa("-size"));
  grabber.useDesired(formatGray);
  
  //fid->loadMarkers("[0,10]",ParamList("size",Size(96,96)));
  try{
    fid->setPropertyValue("regions.minimum region size",400);
    fid->setPropertyValue("thresh.global threshold",-11);
    fid->setPropertyValue("thresh.mask size",45);
    fid->setPropertyValue("thresh.algorithm","tiled linear");
  }catch(ICLException &e){
    WARNING_LOG("exception caught while setting initial parameters: " << e.what());
  }
  

  obj = new Obj;
  if(pa("-c")){
    scene.addCamera(Camera(*pa("-c")));
  }else{
    scene.addCamera(Camera());
  }
  scene.getCamera(0).setResolution(pa("-size"));
  scene.addObject(obj);
  // scene.setDrawCoordinateFrameEnabled(true);
  fid->setCamera(scene.getCamera(0));
  //  gui["draw"].install(scene.getSceneMouseHandler(0));
}



// working loop
void run(){
  while(gui["pause"]) Thread::msleep(100);
  const ImgBase *image = grabber.grab();
  //save(*image->asImg<icl8u>(),"image.ppm");
  
  const std::vector<Fiducial> &fids = fid->detect(image);
  gui_DrawHandle3D(draw);
  draw = fid->getIntermediateImage(gui["vis"]);

  draw->lock();
  draw->reset();
  draw->reset3D();
  draw->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    //if(fids[i].getID() == 3){
    //  obj->setTransformation(fids[i].getPose3D());
    //}
    
    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());
    draw->color(0,100,255,255);
    draw->text(fids[i].getName(),fids[i].getCenter2D().x, fids[i].getCenter2D().y,9);
    draw->color(0,255,0,255);
    float a = fids[i].getRotation2D();
    draw->line(fids[i].getCenter2D(), fids[i].getCenter2D() + Point32f( cos(a), sin(a))*100 );
    /*
        const std::vector<Fiducial::KeyPoint> &kp = fids[i].getKeyPoints2D();
        for(unsigned int j=0;j<kp.size();++j){
        const Fiducial::KeyPoint &p = kp[j];
        draw->color(255,0,255,255);
        draw->sym(p.imagePos, '+');
        draw->text(str(p.ID), p.imagePos.x,p.imagePos.y,9);
        }
    draw->callback(scene.getGLCallback(0));
   */
  }
  draw->unlock();

  draw.update();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2) -camera|-c(camerafile) -size|-s(size=VGA) "
                "-marker-type|-m(type=art,whichToLoad=art/*.png,size=50x50)",init,run).exec();
}
