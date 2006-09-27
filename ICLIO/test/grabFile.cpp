#include "Img.h"
#include "FileRead.h"
#include "FileWrite.h"

using namespace std;
using namespace icl;

int main() 
{
  //---- Allocate variables ----
  ImgI* poImg = imgNew(depth8u,Size(144,144));
  
  FileRead ioRead("mask","./demoImages","pgm",0);
  FileWrite ioWrite("outMask","./demoImages","pgm");
  
  ioRead.grab(poImg);
  ioWrite.write(poImg);
  
  ioRead.grab(poImg);
  ioWrite.write(poImg);

  ioRead.grab(poImg);
  ioWrite.write(poImg);
}
