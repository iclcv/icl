#include "Img.h"
#include "File.h"

using namespace std;
using namespace icl;

int main() 
{
  //---- Allocate variables ----
  File ioIn("./demoImages/mask", "pgm", 1, 1, 1, 2);
  File ioOut("./demoImages/outMask", "pgm", 1, 1, 1, 2);
  ImgI* poImg = imgNew(depth8u,Size(400,400));
  ImgI* poImg2;

  //---- Grab image to the given size of poImg ----
  ioIn.grab(poImg);

  //---- Write data to file ----
  ioOut.write(poImg);
  
  //---- Grab image in original size ----
  poImg2 = ioIn.grab();
  
  //---- Write data to file ----
  ioOut.write(poImg2);
}
