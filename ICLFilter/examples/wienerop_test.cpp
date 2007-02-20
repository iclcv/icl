#include <WienerOp.h>
#include <FileReader.h>
#include <TestImages.h>

using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
  ImgBase *src = TestImages::create("parrot",formatRGB,depth32f);
  ImgBase *dst = 0;


  src->setROI(Rect(20,20,600,800));
  

  WienerOp wo(Size(5,5),0.2);
  wo.setClipToROI(true);
  wo.apply (src,&dst);
  
  // write and display the image
  TestImages::xv (dst);
  
  return 0;
}
