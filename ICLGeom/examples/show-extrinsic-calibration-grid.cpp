/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/show-extrinsic-calibration-grid.cpp   **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
  ("-input","like default ICL -input argument,\n"
   "but 3rd subargument is camera-calibration-xml file\n"
   "which can be created with the icl-cam-calib tool")
  ("-grid","receives a long list of numbers, syntax:\n"
   "\t -grid Px Py Pz V1x V1y V1z V2x V2y V2z NXCells NYCells\n"
   "\t all value in mm\n"
   "\t P = plane's position vector (points to the center of the grid)\n"
   "\t V1 first plane direction vector\n"
   "\t V2 second plane direction vector\n"
   "\t NXCells and NYCells grid cell count\n"
   "\t CellW and CellH grid cell size\n"
   );
  return ICLApplication(n,ppc,"-input|-i(device,device-prams,camera-xml-file) "
                        "-grid|-g(Px=-800,Py=1600,Pz=0,V1x=50,V1y=0,V1z=0,V2x=0,V2y=-50,V2z=0,NXCells=32,NYCells=32)",init,run).exec();
}
