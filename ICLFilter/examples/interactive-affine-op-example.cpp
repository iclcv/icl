#include <iclCommon.h>
#include <iclAffineOp.h>

GUI gui;
Img8u image;

Point32f anchor;
Point32f rotAnchor(100,100);
Point32f movePos;
Point32f rotPos;

void update(){
  AffineOp op;
  //op.translate(-rotAnchor.x,-rotAnchor.y);
  //op.rotate(::atan2(rotPos.y-anchor.y,rotPos.x-anchor.x)*(180/M_PI));
  //op.translate(rotAnchor.x,rotAnchor.y);

  op.translate(movePos.x-anchor.x,movePos.y-anchor.y);
  
  static ImgBase *dst=0;
  op.apply(&image,&dst);
  gui["draw"] = dst;
  gui["draw"].update();
  Thread::msleep(10);
}


void mouse(const MouseEvent &evt){
  std::vector<bool> down = evt.getDownMask();
  Point32f p = evt.getPos();
  if(evt.isDragEvent()){
    if(down[0]){
      movePos = p;
    }else if(down[2]){
      rotPos = p;
    }
    update();
  }else if(evt.isPressEvent()){
    if(evt.isMiddle()){
      rotAnchor = p;
    }else{
      anchor = p;
    }
  }
}

void init(){
  gui << "draw[@handle=draw]";
  
  gui.show();
  

  
  image = cvt8u(scale(create("parrot"),0.4));

  gui["draw"].install(new MouseHandler(mouse));  
  gui["draw"] = image;
  gui["draw"].update();
}


int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
