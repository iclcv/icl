#include <iclQuick.h>
#include <iclRandom.h>

int main(){
  ImgQ canvas(Size::VGA,formatRGB);
  icl::fill(0,100,255); 
  icl::color(0,0,0,0);
  icl::rect(canvas,0,0,640,480);
  icl::fill(50,255,0);
  icl::rect(canvas,0,400,640,280); 

  icl::fill(255,255,255,200);
  icl::rect(canvas,250,253,200,150);
  
  icl::fill(50,255,0,100);
  icl::rect(canvas,0,380,640,23); 

  icl::fill(255,50,20,255);
  icl::color(200,30,10,255);
  icl::triangle(canvas,245,253,455,253,350,100);

  icl::color(0,0,0,0);
  randomSeed();
  for(int i=0;i<1000;++i){
    icl::fill(255,255,255,random(1u,30u));
    icl::circle(canvas,random(-20.,660.),random(-20.,(i>900)*50+50.),random(5.,20.));
  }
  color(200,200,200,255);
  font(30,"Arial");
  text(canvas,300,300,"home");
  
  show(canvas);
}
