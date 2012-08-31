#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>

HSplit gui;
GenericGrabber grabber;
FiducialDetector fid("bch","[100-200]","size=30x30");

void init(){
  fid.setConfigurableID("fid");

  gui << Draw().handle("draw")
      << Prop("fid").maxSize(18,100)
      << Show();

  grabber.init(pa("-input")); 
}

void run(){
  static DrawHandle draw = gui["draw"];
  const ImgBase *image = grabber.grab();
  const std::vector<Fiducial> &fids = fid.detect(image);

  draw = image;
  draw->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    Point32f c = fids[i].getCenter2D();
    float rot = fids[i].getRotation2D();

    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());
    draw->text(fids[i].getName(),c.x,c.y,10);
    draw->color(0,255,0,255);
    draw->fill(0,255,0,255);
    draw->arrow(c, c+Point32f( cos(rot), sin(rot))*25, 5);
  }

  draw.render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
