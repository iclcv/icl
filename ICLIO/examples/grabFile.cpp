#include <iclImg.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>
#include <sstream>

using namespace std;
using namespace icl;

int main(int argc, char* argv[]) 
{
   ostringstream oss;
   if (argc <= 1) oss << "demoImages*.p?m";
   else {
      for (int i=1; i < argc; ++i) oss << argv[i] << " ";
   }

   FileGrabber ioRead(oss.str());
   FileWriter ioWrite("file__##.icl.gz");

   // grab 20 images and write 5 of them
   for (int n=0; n < 20; n++) {
      const ImgBase* poImg = ioRead.grab ();
      if (n % 4 == 0) ioWrite.write(poImg);
   }
}
