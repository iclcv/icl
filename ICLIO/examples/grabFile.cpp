#include "Img.h"
#include "FileReader.h"
#include "FileWriter.h"

using namespace std;
using namespace icl;

int main() 
{
  //---- Allocate variables ----
  ImgBase* poImg = imgNew(depth8u,Size(144,144));

  FileReader  ioRead("demoImages/mask*.pgm");
  FileWriter ioWrite("demoImages/outmask##.pgm");
  
  ioRead.grab(poImg);
  ioWrite.write(poImg);
  
  ioRead.grab(poImg);
  ioWrite.setFileName ("demoImages/outmask##.pgm.gz");
  ioWrite.write(poImg);

  ioRead.grab(poImg);
  ioWrite.setFileName ("demoImages/outmask##.jpg");
  ioWrite.write(poImg);

  ioRead = FileReader ("demoImages/*.jpg");
  ioRead.grab(poImg);

  FileReader io (ioRead);
  io.grab (poImg);
  ioWrite.write(poImg);

  delete poImg; poImg = 0;
}
