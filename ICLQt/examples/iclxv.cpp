#include <iclQuick.h>
#include <QApplication>
#include <iclWidget.h>
#include <iclProgArg.h>

int main (int n, char **ppc){
  QApplication app(n,ppc);
  pa_init(n,ppc,"-input(1) -delete");
  
  ICLWidget w;
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
  w.setImage(&image);
  w.resize(QSize(image.getWidth(),image.getHeight()));
  app.exec();
  
 
  
  return 0;
}
