#include "Threshold.h"
#include "FileWrite.h"
#include "FileRead.h"

using namespace std;
using namespace icl;
using namespace std;

void xv(string s){
  system(string("xv ").append(s).append("&").c_str());
}

int main(int nArgs, char **ppcArg){
  if(nArgs < 3){
    printf("too few arguments \n");
    printf("usage: %s srcfile dstfile\n", ppcArg[0]);
    exit(-1);
  }

  string srcName = ppcArg[1];
  string dstName = ppcArg[2];
  
  // READ the image
  FileRead reader(srcName);
  ImgI *image = reader.grab();
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
