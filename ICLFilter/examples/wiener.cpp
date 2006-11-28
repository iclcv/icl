#include <Wiener.h>
#include <FileReader.h>
#include <TestImages.h>

using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
   ImgBase *src, *dst=0;
   string srcName("");
   string dstName("wiener.ppm.gz");
   if (nArgs > 2) dstName = ppcArg[2];
   if (nArgs > 1) {
      // read image from file
      FileReader reader(ppcArg[1]);
      src = reader.grab();
   } else src = TestImages::create("women");

   // apply wiener filter
   Size mask (3,3);
   Wiener(mask).apply (src,&dst,0.5);
  
   // write and display the image
   TestImages::xv (dst, dstName);

   return 0;
}
