#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLIO/GenericImageOutput.h>
Scene scene;
GUI gui("hsplit");
GUI prevGUI("hbox");

GenericImageOutput colorOut, depthOut;
SceneObject *obj = 0;

SmartPtr<Mat> colorDepthTM;

void init(){
  bool cOut = pa("-c"), dOut = pa("-d");
  
  if(cOut){
    colorOut.init(pa("-c"));
  }
  if(dOut){
    depthOut.init(pa("-d"));
  }
  
  if(cOut || dOut){
    prevGUI << (cOut ? "image[@handle=color]" : "")
            << (dOut ? "image[@handle=depth]" : "")
            << "!create";
  }

  gui << "draw3D[@handle=draw]" 
      << ( GUI("vbox[@minsize=10x2]")
           << "fslider(-10,10,0)[@out=x@label=x]"
           << "fslider(-10,10,0)[@out=y@label=y]"
           << "fslider(1.5,10,0)[@out=z@label=z]"
           << ((cOut||dOut) ? "togglebutton(show,hide)[@label=preview@handle=preview]" : "")
         )
      << "!show";
  
  if(cOut || dOut){
    gui["preview"].registerCallback(function(&prevGUI,&GUI::switchVisibility));
    if(dOut){
      ImageHandle d = prevGUI["depth"];
      d->setRangeMode(ICLWidget::rmAuto);
    }
  }
  
  Camera defaultCam(Vec(4.73553,-3.74203,8.06666,1),
                    Vec(-0.498035,0.458701,-0.735904,1),
                    Vec(0.787984,-0.116955,-0.604486,1));
  scene.addCamera( !pa("-cam").as<bool>() ? defaultCam : Camera(*pa("-cam")));
  
  if(pa("-different-color-camera")){
    scene.addCamera(*pa("-different-color-camera"));
    Mat D=scene.getCamera(0).getCSTransformationMatrix();
    Mat C=scene.getCamera(1).getCSTransformationMatrix();
    
    colorDepthTM = new Mat(C*D.inv());
  }
  SceneObject* ground = SceneObject::cuboid(0,0,0,200,200,3);
  ground->setColor(Primitive::quad,GeomColor(100,100,100,255));
  
  scene.addObject(ground);
  if(pa("-object")){
    scene.addObject( (obj = new SceneObject(*pa("-object"))) );
  }else{
    scene.addObject( (obj = SceneObject::cube(0,0,3, 3) ) );
  }
  obj->setColor(Primitive::quad, GeomColor(0,100,255,255));
  obj->setColor(Primitive::triangle, GeomColor(0,100,255,255));
  obj->setColor(Primitive::polygon, GeomColor(0,100,255,255));
  obj->setVisible(Primitive::line | Primitive::vertex, false);
  
  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
  

}

void run() {
  bool cOut = pa("-c"), dOut = pa("-d");
  
  obj->removeTransformation();
  obj->translate(Vec(gui["x"],gui["y"],gui["z"],1));

  static Img32f depthImage;
  if(cOut || dOut){
    const Img8u colorImage = scene.render(0,0,dOut ? &depthImage : 0);
    
    if(colorDepthTM){
      Camera &d = scene.getCamera(0);
      Camera &c = scene.getCamera(1);
      c.setTransformation(*colorDepthTM * d.getCSTransformationMatrix());
      const Img8u colorImage2 = scene.render(1);
      if(cOut) colorOut.send(&colorImage2);
      if(prevGUI.isVisible()){
        if(cOut) prevGUI["color"] = colorImage2;
        if(dOut) prevGUI["depth"] = depthImage;
      }
    }else{
      if(cOut) colorOut.send(&colorImage);
      if(prevGUI.isVisible()){
        if(cOut) prevGUI["color"] = colorImage;
        if(dOut) prevGUI["depth"] = depthImage;
      }
    }
    if(dOut) depthOut.send(&depthImage);
    
  }
  gui["draw"].render();
}  


int main(int n, char **v){
  return ICLApp(n,v,"-depth-out|-d(2) -color-out|-c(2) "
		"-object|-o(obj-filename) -initial-camera|-cam(camerafile) "
                "-different-color-camera(camerafile)",init,run).exec();
}
