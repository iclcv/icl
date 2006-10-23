#include "Img.h"
#include "PWCGrabber.h"
#include "FileWrite.h"

using namespace std;
using namespace icl;

int main() 
{
  
  PWCGrabber cam(Size(320,240));
  ImgI* poImg = imgNew(depth8u,Size(320,240),formatRGB);

  FileWrite ioWrite("demoImages/outImg##.ppm");
  
  cam.grab(poImg);
  ioWrite.write(poImg);
  
  cam.grab(poImg);
  ioWrite.write(poImg);

  cam.grab(poImg);
  ioWrite.write(poImg);
}
