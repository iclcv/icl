#include "Img.h"
#include "File.h"
#include "Timer.h"

using namespace std;
using namespace icl;

int main() 
{
  //---- Allocate variables ----
  File ioIn("./demoImages/mask", "pgm", 1, 1, 1, 2);
  File ioOut("./demoImages/outMask", "pgm", 1, 1, 1, 2);
  ImgI* poImg = imgNew(depth8u,Size(400,400));
  ImgI* poImg2;
  Timer measure(0);
  
  //---- Grab image to the given size of poImg ----
  try 
  {
    measure.startTimer();
    ioIn.grab(poImg);
    measure.stopSubTimer("Grab image finished");
  }
  catch (ICLException& e)
  {
    e.report();
  }
  
  //---- Write data to file ----
  ioOut.write(poImg);
  measure.stopSubTimer("Write image finished");

  //---- Grab image in original size ----
  poImg2 = ioIn.grab();
  measure.stopSubTimer("2nd grab image finished");
  
  //---- Write data to file ----
  ioOut.write(poImg2);
  measure.stopTimer("2nd write finished");
}
