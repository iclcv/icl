#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>

HSplit gui;
GenericGrabber grabber;

// the global detector class
// here, using the first 100 "bch"-markers
FiducialDetector fid("bch","[0-100]","size=30x30");

void init(){
  fid.setConfigurableID("fid");
  gui << Draw().handle("draw")
      << Prop("fid").maxSize(18,99)
      << Show();
  grabber.init(pa("-input")); 
}

void run(){
  static DrawHandle draw = gui["draw"];
  const core::ImgBase *image = grabber.grab();
    
  const std::vector<Fiducial> &fids = fid.detect(image);

  draw = image;

  draw->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    utils::Point32f c = fids[i].getCenter2D();
    float rot = fids[i].getRotation2D();
    
    draw->color(0,100,255,255);
    draw->text(fids[i].getName(),c.x,c.y,10);
    draw->color(0,255,0,255);
    draw->line(c,c+utils::Point32f( cos(rot), sin(rot))*100 );

    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());
  }
  draw.render();
}
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
