#include <ICLQuick/Common.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

#include <ICLMarkers/FiducialDetector.h>

GUI gui("hsplit");

GenericGrabber grabber;
FiducialDetector fid("bch","[0-100]",ParamList("size",Size(30,30)));


void init(){
  fid.setConfigurableID("fid");
  gui << "draw[@handle=draw@minsize=16x12]"
      << "prop(fid)[@maxsize=18x100]"
      << "!show";

  grabber.init(pa("-input"));
}



// working loop
void run(){
  const ImgBase *image = grabber.grab();
  const std::vector<Fiducial> &fids = fid.detect(image);
  DrawHandle draw = gui["draw"];

  draw = image;
  
  draw->lock();
  draw->reset();
  draw->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());
    draw->color(0,100,255,255);
    draw->text(fids[i].getName(),fids[i].getCenter2D().x, fids[i].getCenter2D().y,9);
    draw->color(0,255,0,255);
    float a = fids[i].getRotation2D();
    draw->line(fids[i].getCenter2D(), fids[i].getCenter2D() + Point32f( cos(a), sin(a))*100 );
  }
  draw->unlock();

  draw.update();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
