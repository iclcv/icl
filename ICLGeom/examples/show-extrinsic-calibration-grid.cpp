#include <ICLQuick/Common.h>
#include <ICLGeom/Camera.h>

GUI gui;
std::vector<Point32f> grid;
int Nx,Ny;
Camera cam;
GenericGrabber *grabber = 0;

void init(){
  
  if(!pa_defined("-input")){
    pa_usage("-input arg is mandatory");
    exit(-1);
  }
  gui << "draw[@handle=draw@minsize=32x24]";
  gui.show();


  cam = Camera(pa_subarg<std::string>("-input",2,""));
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setDesiredSize(cam.getViewPort().getSize());
  
  Vec p(pa_subarg<float>("-grid",0,-600),
        pa_subarg<float>("-grid",1,0),
        pa_subarg<float>("-grid",2,0));
  
  Vec v1(pa_subarg<float>("-grid",3,100),
         pa_subarg<float>("-grid",4,0),
         pa_subarg<float>("-grid",5,0));
  
  Vec v2(pa_subarg<float>("-grid",6,0),
         pa_subarg<float>("-grid",7,100),
         pa_subarg<float>("-grid",8,0));
  
  Nx = pa_subarg<float>("-grid",9,12)+1;
  Ny = pa_subarg<float>("-grid",10,8)+1;


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
  pa_explain("-input","like default ICL -input argument, but 3rd subargument: camera-calibrtion-xml file\n"
             " which can be created with icl-extrinsic-camera-calibration tool");
  pa_explain("-grid","receives a long list of numbers, syntax:\n"
             "\t -grid Px Py Pz V1x V1y V1z V2x V2y V2z NXCells NYCells\n"
             "\t all value in mm\n"
             "\t P = plane's position vector (points to the center of the grid)\n"
             "\t V1 first plane direction vector\n"
             "\t V2 second plane direction vector\n"
             "\t NXCells and NYCells grid cell count\n"
             "\t CellW and CellH grid cell size\n"
             "\t default args are [0 300 0 0 0 1 12 8 100 100]"
             );
  return ICLApplication(n,ppc,"-input(3) -grid(11)",init,run).exec();
}
