#include <Img.h>
#include <FileReader.h>
#include <FileWriter.h>

using namespace std;
using namespace icl;

int main() 
{
  //---- Allocate variables ----
  ImgBase* poImg = imgNew(depth8u,Size(144,144));

  FileReader ioRead(FileReader::hashPattern("/vol/bilddaten/share/vampire/slam/coil-100/sub1/ppm/obj1__##.ppm"));
  FileReader io (ioRead);
  FileWriter ioWrite("obj1__##.ppm.gz");

  for (int n=0; n < 20; n++) {
     io.grab (poImg);
     if (n % 4 == 0) ioWrite.write(poImg);
  }

  delete poImg; poImg = 0;
}
