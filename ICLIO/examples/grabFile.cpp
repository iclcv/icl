#include "Img.h"
#include "FileRead.h"
#include "FileWriter.h"

using namespace std;
using namespace icl;

int main() 
{
  //---- Allocate variables ----
  ImgBase* poImg = imgNew(depth8u,Size(144,144));

  FileRead  ioRead("demoImages/mask*.pgm");
  FileWriter ioWrite("demoImages/outmask##.pgm");
  
  ioRead.grab(poImg);
  ioWrite.write(poImg);
  
  ioRead.grab(poImg);
  ioWrite.write(poImg);

  ioRead.grab(poImg);
  ioWrite.setFileName ("demoImages/outmask##.jpg");
  ioWrite.write(poImg);

  ioRead = FileRead ("demoImages/*.jpg");
  ioRead.grab(poImg);

  FileRead io (ioRead);
  io.grab (poImg);
  ioWrite.write(poImg);

  delete poImg; poImg = 0;
}
