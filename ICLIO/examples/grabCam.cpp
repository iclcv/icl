#include <Img.h>
#include <PWCGrabber.h>
#include <FileWriter.h>

using namespace std;
using namespace icl;

int main() 
{
  
  PWCGrabber cam(Size(320,240));
  ImgBase* poImg = imgNew(depth8u,Size(320,240),formatRGB);

  FileWriter ioWrite("demoImages/outImg##.ppm");
  
  cam.grab(poImg);
  ioWrite.write(poImg);
  
  cam.grab(poImg);
  ioWrite.write(poImg);

  cam.grab(poImg);
  ioWrite.write(poImg);
}
