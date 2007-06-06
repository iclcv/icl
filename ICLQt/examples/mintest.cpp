#include <iclPWCGrabber.h>
#include <iclQuick.h>



Size s(320,240);

int main(){

  PWCGrabber a(s,24,0);
  PWCGrabber b(s,24,1);

  usleep(1000*1000*1);
  return 0;
  
  //a.setDesiredSize(s);
  //b.setDesiredSize(s);

  printf("grabbing some images \n");

  for(int i=0;i<10;i++){
    printf("loop %d started \n",i);
    const ImgBase *ima = a.grab();
    save(cvt(ima),"imagea.ppm");

    const ImgBase *imb = b.grab();
    save(cvt(imb),"imageb.ppm");

    printf("loop %d done \n",i);
  }
  
  printf("starting to wait! \n");
  
  ImgQ xa=load("imagea.ppm");
  ImgQ xb=load("imageb.ppm");

  save(scale((xa,xb),0.5),"theimage.ppm");
  system("rm -rf imagea.ppm imageb.ppm");
  return 0;

                                     

}
