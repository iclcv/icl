#include <Img.h>
#include <FileReader.h>
#include <FileWriter.h>
#include <LocalThreshold.h>
#include <TestImages.h>
#include <ProgArg.h>

#include <stdio.h>

using namespace icl;
using namespace std;


int main (int argc, char **argv) {
  pa_init(argc, argv,"-input(1) -output(1) -masksize(1) -globalthreshold(1)");

  Size s(640,480);
  Img8u  *I = new Img8u(s,formatRGB), *R = new Img8u(s,formatRGB);
  

  if(pa_defined("-input")){
    printf("input file %s \n",pa_subarg<string>("-input",0,"").c_str());    
    FileReader(pa_subarg<string>("-input",0,"")).grab(I);
  }
  else{
    printf("working with testimage tree: \n");
    TestImages::create("tree")->deepCopy(I);
  }
  
  unsigned int maskSize = pa_subarg<unsigned int>("-masksize",0,10);
  int gt = pa_subarg<int>("-globalthreshold",0,0);
  printf("mask size was set to %d \n",maskSize);
  printf("global threshold was set to  %d \n",gt);
  
  ImgBase *RBase =(ImgBase*)R;
  
  LocalThreshold t(maskSize,gt);
  t.apply(I,&RBase);
  
  string outputName = pa_subarg<string>("-output",0,"./out.jpg");
  printf("writing output file %s \n", outputName.c_str());
  FileWriter(outputName).write(R);

}
