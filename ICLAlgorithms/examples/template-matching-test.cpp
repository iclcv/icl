#include <ICLAlgorithms/UsefulFunctions.h>
#include <ICLQuick/Quick.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>

int main(int n, char **ppc){
  //  640x480@(5,10)
  paex
  ("-roi|-r","specify template roi like (X,Y)WIDTHxHEIGHT")
  ("-significance-level|-s","specify significance level range [0,1]");
  painit(n,ppc,"-roi(Rect=(200,400)100x120) -s(float=0.9)");
  
  Img8u image = cvt8u(create("parrot"));
  image.scale(Size(640,480));
  
  Rect roi = pa("-roi");
  roi &= image.getImageRect();
  
  image.setROI(roi);
  Img8u templ = cvt8u(copyroi(cvt(image)));
  image.setFullROI();
  

  float s = pa("-s");
  printf("using significance: %f \n",s);
  Img8u *buffer = new Img8u;
  RegionDetector rd;
  vector<Rect> rs = iclMatchTemplate(image,templ,s,buffer,false,&rd);
  std::cout << "Estimating Time for 100 Iterations ..." << std::endl;
  tic();
  for(int i=0;i<100;i++){
    rs = iclMatchTemplate(image,templ,s,buffer,false,&rd);
  }
  toc();
  
  
  ImgQ resultImage = cvt(image);
  
  color(255,255,255);
  for(unsigned int i=0;i<rs.size();++i){
    rect(resultImage,rs[i]);
  }
  
  show(label(resultImage,"results"));

  return 0;
}



