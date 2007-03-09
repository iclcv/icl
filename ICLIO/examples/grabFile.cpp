#include <iclImg.h>
#include <iclFileReader.h>
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

   FileReader ioRead(oss.str());
   FileReader io (ioRead);
   FileWriter ioWrite("file__##.ppm.gz");

   // grab 20 images and write 5 of them
   for (int n=0; n < 20; n++) {
      const ImgBase* poImg = io.grab ();
      if (n % 4 == 0) ioWrite.write(poImg);
   }
}
