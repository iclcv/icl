#include <iclImg.h>
#include <iclPWCGrabber.h>
#include <iclFileWriter.h>

using namespace std;
using namespace icl;

int main() {
  
  PWCGrabber cam(Size(320,240));
  FileWriter ioWrite("testImg##.ppm");
  
  for(int i=0;i<3;i++){
    ioWrite.write(cam.grab());
  }
}
