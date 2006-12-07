#include <Img.h>
#include <FileReader.h>
#include <FileWriter.h>
#include <LocalThreshold.h>
#include <TestImages.h>

#include <stdio.h>
#include <Timer.h>

using namespace icl;
using namespace std;


int main (int argc, char **argv) {
  Size s(640,480);
  Img8u  *I = new Img8u(s,formatRGB), *R = new Img8u(s,formatRGB);
  
  if(argc != 2){
    printf("working with testimage tree: \n");
    TestImages::create("tree")->deepCopy(I);
  }else{
    printf("input file %s \n",argv[1]);
    FileReader(argv[1]).grab(I);
  }
  
  ImgBase *RBase =(ImgBase*)R;
  LocalThreshold(10,0).apply(I,&RBase);
  
  printf("writing output file out.jpg \n");
  FileWriter("out.jpg").write(R);

}
