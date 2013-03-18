#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>

// static application data
HSplit gui;
GenericGrabber grabber;

// the global detector class ("bch" markers)
// here, using marker ids {100,101, .. 200}
FiducialDetector fid("bch","[100-200]","size=30x30");

// initialization function (called once after start)
void init(){
  // set the configurable ID (see icl::Configurable class documentation)
  fid.setConfigurableID("fid");

  // create the GUI
  gui << Draw().handle("draw")          // create drawing component
      << Prop("fid").maxSize(18,100)    // create the property widged for 'fid'
      << Show();                        // show the main widget

  // initialize the grabber from given program argument
  grabber.init(pa("-input")); 
}


// working loop (automatically looped in the working thread)
void run(){
  // get a handle to the "draw" component
  static DrawHandle draw = gui["draw"];

  // grab the next image
  const ImgBase *image = grabber.grab();
    
  // detect markers
  const std::vector<Fiducial> &fids = fid.detect(image);

  // visualize the image
  draw = image;
  
  // draw marker detection results
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
  // update the visualization
  draw.render();
}

// main method 
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
