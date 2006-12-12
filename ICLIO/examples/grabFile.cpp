#include <Img.h>
#include <FileReader.h>

using namespace std;
using namespace icl;

int main() 
{
  //---- Allocate variables ----
  ImgBase* poImg = imgNew(depth8u,Size(144,144));

  FileReader ioRead("/home/migoe/lehre/SeminarOnlineComputerVisionInRobotics/images/tomatoes/tomatoes_sequence_18.ppm");

  FileReader io (ioRead);
  io.grab (poImg);

  delete poImg; poImg = 0;
}
