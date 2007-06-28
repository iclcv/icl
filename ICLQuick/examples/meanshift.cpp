#include <iclQuick.h>
#include <cstdlib>
#include <math.h>

//tx, ty coords of a pixel in the local window
//h kernel bandwidth
double kernelMagic(double tx, double ty, int h){
  double returnV;
  //normalizing and computing the norm
  returnV = sqrt((tx*tx)/(h*h)+(ty*ty)/(h*h));
  //cutting of
  if (returnV > 1) {
    returnV = 0;
  } else {
    returnV = 1 - returnV * returnV;
  }
  return returnV;
}

int main(){

  
  //  ImgQ myImage = load("red_gradient.jpg");
  ImgQ myImage = zeros(1000,1000,1);
  
  color(255);
  fill(255);
  rect(myImage,180,180,40,45);
  
  fill(0);
  myImage.setFormat(formatGray);
  save(myImage,"bla.jpg");
		       	  
  //just a gray image for the weights, testing
  ImgQ wImage = gray(myImage);
  
  //h kernel bandwidth=half window size; 50 for test
  int h = 80;
  //x,y coords of center of kernel window, 50 for test
  int cx = 180;
  int cy = 180;

  
  //drawing rect at current kernel pos
  //  rect(myImage, cx, cy, h*2, h*2);
  
  int repeat = 0;

  static int END =100;  
  while(repeat < 2 && END){

    END--;
    //msx, msy the meanshift vectors for x and y directions
    double msx = 0;
    double msy = 0;
    //for the upper and lower sums of both vectors
    double msux = 0;
    double mslx = 0;
    double msuy = 0;
    double msly = 0;

    //loop; x, y coords of all pixels in local window
    for (int x = cx-h; x <= cx+h; x++){
      for (int y = cy-h; y <= cy+h; y++){
	//checking: x, y have to be in the picture
	if (x < 0) x = 0;
	if (x >= myImage.getWidth()) break;
	if (y < 0) y = 0;
	if (y >= myImage.getHeight()) break;
	//tx, ty relative coords
	double tx = x - cx;
	double ty = y - cy;
	//ps part sum, equal in both sums
	double ps = kernelMagic(tx, ty, h)*(double)wImage(x, y, 0);
	msux += (ps*x);
	mslx += ps;
	msuy += (ps*y);
	msly += ps;
	//if (kernelMagic(tx, ty, h)>0) printf("debug: tx %f, ty %f, ps %f, magic %f\n", tx, ty, ps, kernelMagic(tx, ty, h));
      }
    }
    //checking for zeros in the divisor
    if (mslx != 0){
      msx = msux / mslx;
    } else {
      printf("mslx %f, msux %f error\n", mslx, msux);
    }
    if (msly != 0){
      msy = msuy / msly;
    }else {
      printf("msly %f, msuy %f error\n", msly, msuy);
    }
  
    //debugging print
    //printf ("after run %d msux %f, mslx %f\n", repeat, msux, mslx);
  
    //if there is no shift, moving the window manuel; CHANGE THIS!
    //if the window does not return, repeat has to be set back to 0
    
    if (((int)msx == cx) && ((int)msy == cy) && (repeat < 2)){
      repeat++;
      msx = cx + h/10;
      msy = cy + h/10;
    }
    printf("msx %f, mxy %f \n", msx, msy);
    //setting new center
    
    float dx = msx-cx;
    float dy = msy-cy;

    if( fabs(dx) + fabs(dy) < 2) break;
    cx -= dx;
    cy -= dy;

    //cx = (int)msx;
    //cy = (int)msy;
    printf("cx %d, cy %d\n", cx, cy);
  
    //drawing another rect
    static float alpha = 0;
    
    alpha +=20;
    alpha = std::min(float(255.0),alpha);
    color(255-alpha,2*alpha,0,alpha);
    
    rect(myImage, cx-h/4, cy-h/4, h/2, h/2);  
    
  }  
  
  //  show(myImage);
  save(myImage,"result.jpg");
  
  return 0;
}
