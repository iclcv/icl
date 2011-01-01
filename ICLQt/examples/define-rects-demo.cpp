#include <ICLQuick/Common.h>
#include <ICLGeom/GeomDefs.h>
#include <QtGui/QMenu>
#include <QtGui/QActionEvent>
#include <ICLQt/DefineRectanglesMouseHandler.h>
GUI gui;
GenericGrabber grabber;
DefineRectanglesMouseHandler mouse;

void init(){
  grabber.init(FROM_PROGARG("-i"));
  grabber.setDesiredSize(Size::VGA);
  
  gui << "draw[@minsize=32x24@handle=draw]" << "!show";
  gui["draw"].install(&mouse);
}

void run(){
  gui_DrawHandle(draw);
  draw = grabber.grab();

  draw->lock();
  draw->reset();
  mouse.visualize(**draw);
  draw->unlock();

  draw.update();

  
  Thread::msleep(10);
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(2)",init,run).exec();
}
