#include <iclQuick.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>
#include <iclCCLUT.h>

int main(){
  Img8u a = cvt8u(scale(create("parrot"),32,24));
  
  system("if [ ! -d tmp_imaages ] ; then mkdir tmp_images ; fi");
  system("rm -rf tmp_images/*.ppm");
  
  FileWriter w("tmp_images/image_#####.ppm");
  progress_init("writing images");
  for(unsigned int i=0;i<5000;++i){
    progress(i,5000);
    w.write(&a);
  }
  progress_finish();
  
  FileGrabber g("tmp_images/*.ppm");
  
  progress_init("reading images");
  for(unsigned int i=0;i<5000;++i){
    ImgBase *image = g.grab()->deepCopy();
    progress(i,5000);
    delete image;
  }
  progress_finish();
  
  
  
  
}
