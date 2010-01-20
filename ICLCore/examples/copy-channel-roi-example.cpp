#include <ICLCore/Img.h>

using namespace icl;

int main(){
  Img8u a(Size(10,10),1);
  a.clear(0,255);
  
  
  Img8u b(a.getParams());
  
  a.setROI(Rect(5,5,2,2));
  b.setROI(Rect(2,2,2,2));
  
  a.deepCopyROI(&b);
  
  b.printAsMatrix("3.0",true);
}
