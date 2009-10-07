#include <iclQuick.h>
#include <iclRandom.h>

int main(){
  /// create image to draw something into
  ImgQ canvas(Size::VGA,formatRGB);
  
  /// set fill color (rects, circles and triangles
  /// will be drawn with this fill color (note:
  /// if no alpha value is given, it is set to 255
  icl::fill(0,100,255); 
  
  /// color is used for rects, circles and triangles
  /// boundaries as well as for lines, text and for
  /// single points (using the 'pix' function)
  icl::color(0,0,0,0);
  
  /// draws a rect angle into given image
  /// note, image origin is upper left corner
  icl::rect(canvas,0,0,640,480);
  icl::fill(50,255,0);
  icl::rect(canvas,0,400,640,280);

  icl::fill(255,255,255,200);
  icl::rect(canvas,250,253,200,150);
  
  icl::fill(50,255,0,100);
  icl::rect(canvas,0,380,640,23); 

  icl::fill(255,50,20,255);
  icl::color(200,30,10,255);
  /// triangles work as well
  icl::triangle(canvas,245,253,455,253,350,100);

  /// set alpha to 0 -> no boundaries
  icl::color(0,0,0,0);
  
  /// initialize random-generator
  randomSeed();
  
  /// draw some clouds at the top
  for(int i=0;i<1000;++i){
    icl::fill(255,255,255,random(1u,30u));
    int x = (int)random(-20.,660.);
    int y = (int)random(-20.,(i>900)*50+50.);
    int r = (int)random(5.,20.);
    icl::circle(canvas,x,y,r);
  }
  
  /// color is used also for text
  color(200,200,200,255);
  
  /// setup text size and font
  font(30,"Arial");
  
  /// render text into the image
  text(canvas,300,300,"home");
  
  /// show result image using icl-xv
  show(canvas);
}
