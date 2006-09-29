#include "Threshold.h"
#include "FileWrite.h"
#include "FileRead.h"

using namespace icl;

void xv(string s){
  system(string("xv ").append(s).append("&").c_str());
}

int main(int nArgs, char **ppcArg){
  if(nArgs < 3){
    printf("to few arguments \n");
    exit(-1);
  }
  string srcName = ppcArg[1];
  string dstName = ppcArg[2];
  
  // READ the image
  ImgI *image = FileRead(srcName).grab();
  ImgI *dst   = 0;

  // Perform binarization
  Threshold().binarize(image,&dst,127);
  
  // WRITE the image
  FileWrite(dstName).write(dst);

  // show images using xv
  xv(srcName);
  xv(dstName);  

  return 0;
}
