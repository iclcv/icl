#include <ICLQuick/Common.h>
#include <ICLGeom/Camera.h>

GUI gui;
std::vector<Point32f> grid;
int Nx,Ny;
Camera cam;
GenericGrabber *grabber = 0;

void init(){
  
 
  gui << "draw[@handle=draw@minsize=32x24]";
  gui.show();


  cam = Camera(*pa("-input",2));
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setDesiredSize(cam.getRenderParams().viewport.getSize());
  
  Vec p(pa("-grid",0),
        pa("-grid",1),
        pa("-grid",2));
  
  Vec v1(pa("-grid",3),
         pa("-grid",4),
         pa("-grid",5));
  
  Vec v2(pa("-grid",6),
         pa("-grid",7),
         pa("-grid",8));
  
  Nx = pa("-grid",9).as<int>()+1;
  Ny = pa("-grid",10).as<int>()+1;


  grid.resize(Nx*Ny);
  for(int x=0;x<Nx;++x){
    for(int y=0;y<Ny;++y){
      Vec v = p + v1*x + v2*y;
      v[3]=1;
      grid.at(x+Nx*y) = cam.project(v);
    }
  }

}
  

void run(){
  gui_DrawHandle(draw);
  
  draw=grabber->grab();

  draw->lock();
  draw->reset();
  draw->color(0,100,255,200);
  draw->grid(grid.data(),Nx,Ny);
  draw->unlock();
  
  draw.update();
  Thread::msleep(10);
}

int main(int n, char **ppc){
  paex
  ("-input","like default ICL -input argument, but 3rd subargument: camera-calibrtion-xml file\n"
   " which can be created with icl-extrinsic-camera-calibration tool")
  ("-grid","receives a long list of numbers, syntax:\n"
   "\t -grid Px Py Pz V1x V1y V1z V2x V2y V2z NXCells NYCells\n"
   "\t all value in mm\n"
   "\t P = plane's position vector (points to the center of the grid)\n"
   "\t V1 first plane direction vector\n"
   "\t V2 second plane direction vector\n"
   "\t NXCells and NYCells grid cell count\n"
   "\t CellW and CellH grid cell size\n"
   "\t default args are [0 300 0 0 0 1 12 8 100 100]"
   );
  return ICLApplication(n,ppc,"-input|-i(device,device-prams,camera-xml-file) "
                        "-grid|-g(Px,Py,Pz,V1x,V1y,V1z,V2x,V2y,V2z,NXCells,NYCells)",init,run).exec();
}
