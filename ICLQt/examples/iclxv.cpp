#include <iclQuick.h>
#include <QApplication>
#include <iclDrawWidget.h>
#include <iclProgArg.h>

int main (int n, char **ppc){
  QApplication app(n,ppc);
  pa_explain("-input","define image to read");
  pa_explain("-delete","delete image file after reading");
  pa_explain("-roi","if set, image roi is visualized");
  pa_init(n,ppc,"-input(1) -delete -roi");
  
  ICLDrawWidget w;
  w.show();
 
  
  ImgQ image;
  if(pa_defined("-input")){
    try{
      image = load(pa_subarg<string>("-input",0,"no image"));
      if(pa_defined("-delete")){
        string imageName = pa_subarg<string>("-input",0,"");
        if(imageName.length()){
          system((string("rm -rf ")+imageName).c_str());
        }
      }
    }catch(ICLException e){
      image = ones(320,240,1)*100;
      fontsize(15);
      text(image, 90,90,"image not found!");
    }
  }else{
    image = ones(320,240,1)*100;
    fontsize(15);
    text(image, 110,90,"no image set!");
  }

  if(pa_defined("-roi")){
    w.lock();
    w.reset();
    w.color(255,0,0);
    w.fill(0,0,0,0);

    w.rect(image.getROI().x,image.getROI().y,image.getROI().width,image.getROI().height);
    w.unlock();
  }
  
  
  w.setImage(&image);
  w.resize(QSize(image.getWidth(),image.getHeight()));
  app.exec();
  
 
  
  return 0;
}
