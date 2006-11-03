#include "Threshold.h"
#include "FileRead.h"
#include "TestImages.h"

using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
   ImgI *src, *dst=0;
   string srcName("");
   string dstName("wiener.ppm.gz");
   if (nArgs > 2) dstName = ppcArg[2];
   if (nArgs > 1) {
      // read image from file
      FileRead reader(ppcArg[1]);
      src = reader.grab();
   } else src = TestImages::create("women");
  
  // Perform binarization
  Threshold().binarize(src,&dst,127);
  
  // write and display the image
  TestImages::xv (dst, dstName);

  return 0;
}
