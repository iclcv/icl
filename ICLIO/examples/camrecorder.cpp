#include <iclGenericGrabber.h>
#include <iclQt.h>
#include <iclGUI.h>
#include <iclQuick.h>
#include <iclFileWriter.h>
#include <QPushButton>
#include <iclProgArg.h>

GUI gui("vbox");
Size size;

void init(){
  gui << "image[@minsize=32x24@handle=image]" 
      << "combo(pwc,dc,unicap,file)[@handle=combo@out=xxx]"
      << ( GUI("hbox[@maxsize=100x4]") 
           << "togglebutton(record,stop)[@handle=rsbh@out=record-stop-value]"
           << "string(images/image_#####.ppm,20)[@handle=file-name-handle@out=file-name-str]"
           << "label(written images)[@handle=image-index-label@minsize=5x2@label=written images]"
         );
  
  gui.show();
}

void loop(){
  
  static ComboHandle &combo = gui.getValue<ComboHandle>("combo");
  static ImageHandle &image = gui.getValue<ImageHandle>("image");
  static ButtonHandle &recordStopButtonHandle = gui.getValue<ButtonHandle>("rsbh");
  static LabelHandle &writtenImageLabelHandle = gui.getValue<LabelHandle>("image-index-label");
  static string &filename = gui.getValue<std::string>("file-name-str"); 
  
  static GenericGrabber *grabber = 0;
  static std::map<string,string> params;
  params["pwc"] = "pwc=0";
  params["dc"] = "dc=0";
  params["unicap"] = "unicap=";
  params["file"] = "file=images/*.ppm";
  
  while(1){
    static std::string lastComboItem = "";
    std::string type = combo.getSelectedItem();

    if(lastComboItem != type && (!grabber || grabber->getType() != type)){
      ICL_DELETE( grabber );
      grabber = new GenericGrabber(type,params[type],false);
      if(grabber->isNull()){
        ICL_DELETE( grabber );
      }else{
        grabber->setDesiredSize(size);
      }
    }
    
    static FileWriter *writer = 0;
    static int indexCount = 0;
    if((*recordStopButtonHandle)->isChecked()){
      if(!writer){
        try{
          writer = new FileWriter(filename);
          indexCount = 0;
        }catch(icl::FileOpenException &ex){
          ERROR_LOG("illegal filename");
          writer = 0;
        }
      }
    }else{
      ICL_DELETE(writer);
    }
    
    const ImgBase *grabbedImage = 0;
    if(grabber){
      grabbedImage = grabber->grab();
      image = grabbedImage;
    }else{
      ImgQ buf = zeros(640,480,3);
      color(255,0,0);
      text(buf,150,200,string("no ")+type+"-grabber available");
      image = &buf;
    }
    if(writer && grabbedImage){
      writer->write(grabbedImage);
      writtenImageLabelHandle = ++indexCount;
    }else{
      writtenImageLabelHandle = "...";
    }
    
    
    image.update();
    Thread::msleep(20);  
    lastComboItem = type;  
  }
}



int main(int n, char **ppc){
  QApplication app(n,ppc);
  pa_explain("-size","image size");
  
  pa_init(n,ppc,"-size(1)");
  size = translateSize(pa_subarg<string>("-size",0,"640x480"));
  
  init();

  exec_threaded(loop);  
  
  return app.exec();
}
