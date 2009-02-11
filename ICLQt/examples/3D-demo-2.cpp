#include <iclCommon.h>
#include <iclDrawWidget3D.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>
#include <iclConsoleProgress.h>

Size size;
ICLDrawWidget3D *widget = 0;
Img8u faces[6];
ImgBase *facesB[6];

void init(){
  size = translateSize(pa_subarg<std::string>("-size",0,"VGA"));
  
  FileGrabber g(pa_subarg<std::string>("-input",0,"no_files_yspecified"));

  for(int i=0;i<6;++i){
    g.grab()->convert(&faces[i]);
    facesB[i]=&faces[i];
  }
  
  widget = new ICLDrawWidget3D(0);
  widget->setGeometry(200,200,size.width,size.height);
  widget->show();

}
void run(){
  std::string defFormat = "__isel_%3.2f_amtec_%2.2f_lights_undef_left.tiff.jpeg";
  // isel -> ry, amtec = rx
  SteppingRange<icl32f> rxrange(0,350,10);
  SteppingRange<icl32f> ryrange(-25,75,10);
  
  Img8u bg(Size(10,10),formatMatrix);
  widget->setImage(&bg);
  
  bool output = pa_defined("-output");
  std::string fmt;
  if(output){
    fmt = pa_subarg<std::string>("-output",0,"def");
    if(fmt == "def") fmt = defFormat;
  }
  
  while(1){
    progress_init("Capturing images ...");
    for(float rx=rxrange.minVal;rx<=rxrange.maxVal;rx+=rxrange.stepping){
      for(float ry=ryrange.minVal;ry<=ryrange.maxVal;ry+=ryrange.stepping){
        widget->lock();
        
        widget->reset3D();
        widget->rotate3D(rx,ry,0);
        widget->color3D(1, 1, 1, 1);
        widget->imagecube3D(0,0,0,0.5,(const ImgBase**)facesB);
        widget->unlock();
        if(output){
          static char filename[1024];
          sprintf(filename,fmt.c_str(),rx,ry);
          widget->updateAndSaveFrameBuffer(filename);
          Thread::msleep(1);
        }else{
          widget->updateFromOtherThread();
          Thread::msleep(10);
          
        }
      }
      if(output){
        progress((int)(rx-rxrange.minVal),(int)(rxrange.maxVal-rxrange.minVal));
      }
    }
    
    if(output){
      progress_finish();
      ::exit(0);
    }
  }
}



int main(int n, char **ppc){
  
  pa_explain("-input","file pattern to put on the cube");
  pa_explain("-output","output file pattern\n"
             "\tmust contain two float patterns like %f or %2.2f\n"
             "\tif set to \"def\" a default pattern is used!");
  pa_init(n,ppc,"-input(1) -output(1)");
  ExecThread x(run);
  QApplication a(n,ppc);
  init();
  x.run();
  
  return a.exec();
  
}
