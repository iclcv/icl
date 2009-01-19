#include <iclUsefulFunctions.h>
#include <iclQuick.h>
#include <iclProgArg.h>

int main(int n, char **ppc){
  //  640x480@(5,10)
  pa_explain("-roi","specify template roi like 640x480@(5,10)");
  pa_explain("-s","specify significance level range [0,1]");
  pa_init(n,ppc,"-roi(1) -s(1)");
  
  Img8u image = cvt8u(create("parrot"));
  image.scale(Size(640,480));
  
  Rect roi = translateRect(pa_subarg<string>("-roi",0,"100x120@(200,400)"));
  roi &= image.getImageRect();
  
  image.setROI(roi);
  Img8u templ = cvt8u(copyroi(cvt(image)));
  image.setFullROI();
  

  float s = pa_subarg<float>("-s",0,0.9);
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



