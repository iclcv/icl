#include <ICLQuick/Common.h>
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
  static DrawHandle draw = gui["draw"];

  const ImgBase *image = grabber.grab();
  const std::vector<Fiducial> &fids = fid.detect(image);

  draw = image;
  
  draw->lock();
  draw->reset();
  draw->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    Point32f c = fids[i].getCenter2D();
    float rot = fids[i].getRotation2D();
    
    draw->color(0,100,255,255);
    draw->text(fids[i].getName(),c.x,c.y,10);
    draw->color(0,255,0,255);
    draw->line(c,c+Point32f( cos(rot), sin(rot))*100 );

    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());

  }
  draw->unlock();

  draw.update();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
