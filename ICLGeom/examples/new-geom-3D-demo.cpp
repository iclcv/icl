#include <iclQuick.h>
#include <iclGeom.h>

#include <iclScene2.h>
#include <iclCamera.h>
#include <iclCommon.h>
#include <iclFPSLimiter.h>
#include <iclPrimitive.h>

GUI gui("hsplit");

ImgQ image;
Scene2 scene;
ICLDrawWidget *w;
ICLWidget *w2;
ICLDrawWidget3D *w3D_1;
ICLDrawWidget3D *w3D_2;

void cuboid(const std::string &id,
            float x, float y, float z,
            float dx, float dy, float dz,
            float r, float g, float b, float alpha=255){
  (void)id;
  float data[] = {x,y,z,dx,dy,dz};
  Object2 * o = new Object2("cuboid",data);
  GeomColor c(r,g,b,alpha);
  o->setColor(Primitive::line,c);
  o->setColor(Primitive::triangle,c);
  o->setColor(Primitive::quad,c);
  //o->setVisible(Primitive::vertex,false);
  o->setVisible(Primitive::line,false);
  //o->setVisible(Primitive::triangle,false);
  //o->setVisible(Primitive::quad,false);
  scene.addObject(o);
}

struct ColorCube : public Object2{
  ColorCube(float x, float y, float z, float d){
    addVertex(Vec(x-d,y-d,z-d,1),GeomColor(255,0,0,255));
    addVertex(Vec(x-d,y+d,z-d,1),GeomColor(255,255,0,255));
    addVertex(Vec(x+d,y+d,z-d,1),GeomColor(0,255,0,255));
    addVertex(Vec(x+d,y-d,z-d,1),GeomColor(0,255,255,255));

    addVertex(Vec(x-d,y-d,z+d,1),GeomColor(255,0,255,255));
    addVertex(Vec(x-d,y+d,z+d,1),GeomColor(255,255,255,255));
    addVertex(Vec(x+d,y+d,z+d,1),GeomColor(0,0,0,255));
    addVertex(Vec(x+d,y-d,z+d,1),GeomColor(0,0,255,255));

    addLine(0,1);
    addLine(1,2);
    addLine(2,3);
    addLine(3,0);

    addLine(0,4);
    addLine(1,5);
    addLine(2,6);
    addLine(3,7);

    addLine(4,5);
    addLine(5,6);
    addLine(6,7);
    addLine(7,4);

    addQuad(0,1,2,3);
    addQuad(7,6,5,4);//4,5,6,7);
    addQuad(4,5,1,0);//(0,1,5,4);
    addQuad(5,6,2,1);//(1,2,6,5);
    addQuad(6,7,3,2);//(2,3,7,6);
    addQuad(7,4,0,3);//(3,0,4,7);

    setColorsFromVertices(Primitive::quad,true);
    setColorsFromVertices(Primitive::line,true);
  }
};

void init(){
  gui << "draw[@minsize=16x12@handle=left@label=Rendered into DrawWidget]";
  //gui << "image[@minsize=16x12@handle=right@label=Rendered into ImgQ]";
  gui << "draw3D[@minsize=16x12@handle=gl-1@label=Rendered into GL-Context]";
  gui << "draw3D[@minsize=16x12@handle=gl-2@label=Rendered into GL-Context]";

  GUI con("vbox[@maxsize=6x100]");
  con << "fslider(0.1,10,1.7,vertical)[@label=F@out=f]";
  con << "fps(10)[@handle=fps]";
  
  gui << con;
  gui.show();
  
  w = *gui.getValue<DrawHandle>("left");
  //w2 = *gui.getValue<ImageHandle>("right");  
  w3D_1 = *gui.getValue<DrawHandle3D>("gl-1");
  w3D_2 = *gui.getValue<DrawHandle3D>("gl-2");
  
  image = ImgQ(Size(640,480),formatRGB);
  
  w->setImage(&image);
  //w2->setImage(&image);
  w3D_1->setImage(&image);
  w3D_2->setImage(&image);
  
  Vec pos(0,-200,100);
  Vec fp(0,85,50);
  Vec up(0,0,1);
  scene.addCamera(Camera(pos,normalize(fp-pos),up,Size::VGA));

  Vec pos2(0,-50,150);
  Vec fp2(0,0,0);
  Vec up2(0,1,0);
  scene.addCamera(Camera(pos2,normalize(fp2-pos2),up2,Size::VGA));

  cuboid("worktop",
         0,80,-5,
         160,160,10,
         100,100,100);
  
  cuboid("column-1",
         -75,5,50,
         10,10,100,
         150,150,150);

  cuboid("column-2",
         75,5,50,
         10,10,100,
         150,150,150);

  cuboid("column-3",
         -75,155,50,
         10,10,100,
         150,150,150);

  cuboid("column-4",
         75,155,50,
         10,10,100,
         150,150,150);

  scene.addObject(new ColorCube(0,50,50,30));

  w3D_1->install(scene.getMouseHandler(0));
  w3D_2->install(scene.getMouseHandler(1));
  w->install(scene.getMouseHandler(0));
}


void run(){
  Camera &cam0 = scene.getCamera(0);
  Camera &cam1 = scene.getCamera(1);
  cam0.setViewPort(Rect(0,0,640,480));  
  cam1.setViewPort(Rect(0,0,640,480));  

  FPSLimiter limiter(25);
  
  while(1){
    gui["fps"].update();
    cam0.setFocalLength(gui["f"].as<float>()); /// 1 equals 90° view arc !
    cam1.setFocalLength(gui["f"].as<float>()); /// 1 equals 90° view arc !
    

    w->lock();
    w->reset();
    
    scene.render(*w,0);
    
    w->unlock();
    w->update();
        
    /*
        image.clear();
        
        //scene.render(image,0);
        //w2->setImage(&image);
        //w2->update();
    */

    w3D_1->lock();
    w3D_1->reset3D();
    //w3D->reset();
    w3D_1->callback(scene.getGLCallback(0));
    //scene.render(*w3D,0);
    w3D_1->unlock();
    w3D_1->update();


    w3D_2->lock();
    w3D_2->reset3D();
    //w3D->reset();
    w3D_2->callback(scene.getGLCallback(1));
    //scene.render(*w3D,0);
    w3D_2->unlock();
    w3D_2->update();

    limiter.wait();
  }
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

