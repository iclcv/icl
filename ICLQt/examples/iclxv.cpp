#include <iclQuick.h>
#include <QApplication>
#include <iclDrawWidget.h>
#include <iclProgArg.h>
#include <iostream>
#include <iterator>
#include <iclFileGrabber.h>
#include <iclGUI.h>

GUI gui;
ICLDrawWidget *w=0;


int main (int n, char **ppc){
  QApplication app(n,ppc);
  pa_explain("-input","define image to read");
  pa_explain("-delete","delete image file after reading");
  pa_explain("-roi","if set, image roi is visualized");
  pa_init(n,ppc,"-input(1) -delete -roi",true);
  
  if(pa_defined("-input")){
    w = new ICLDrawWidget;
    w->show();
  }

  
  
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
      fontsize(15);  w->show();
      text(image, 90,90,"image not found!");
    }
    if(pa_dangling_args().size()){
      std::cout << "Warning if called with -input, all extra given filenames are omitted!" << std::endl;
      std::cout << "This are the filenames omitted:";
      std::copy(pa_dangling_args().begin(),pa_dangling_args().end(),std::ostream_iterator<std::string>(std::cout,"\n\t"));
      std::cout << "\n";
    }
  }else if(pa_dangling_args().size()){
    if(pa_defined("-delete")){
      std::cout << "-delete flag is not supported when running in multi image mode" << std::endl;
      std::cout << "call iclxv -input ImageName -delete instead (for single images only)" << std::endl;
    }
    std::string imageList = "";
    std::vector<ImgBase*> imageVec;
    std::vector<std::string> imageVecStrs;
    Size maxSize;
    for(unsigned int i=0;i<pa_dangling_args().size();++i){
      std::string s = pa_dangling_args()[i];
      try{
        FileGrabber grabber(s);
        const ImgBase *image = grabber.grab();
        if(!image) throw ICLException("");
        maxSize.width = iclMax(image->getWidth(),maxSize.width);
        maxSize.height = iclMax(image->getHeight(),maxSize.height);
        imageVec.push_back(image->deepCopy());
        
        std::replace_if(s.begin(),s.end(),std::bind2nd(std::equal_to<char>(),','),'-');
        imageList += (imageList.length() ? ",": "") +s;
        imageVecStrs.push_back(s);
      }catch(const ICLException &ex){
        std::cout << "unable to load image: " << s << std::endl;
        std::cout << "(skipping!)" << std::endl;
      }
    }
    gui << std::string("multidraw(")+imageList+",!all,!deepcopy)[@minsize="+icl::translateSize(maxSize/20)+"@handle=image]";
    gui.show();
    
    MultiDrawHandle &h = gui.getValue<MultiDrawHandle>("image");
    for(unsigned int i=0;i<imageVecStrs.size();++i){
      h[imageVecStrs[i]] = imageVec[i];
      ICL_DELETE(imageVec[i]);
    }    
  }else{
    image = ones(320,240,1)*100;
    fontsize(15);
    text(image, 110,90,"no image set!");
  }

  if(pa_defined("-roi")){
    if(pa_defined("-input")){
      w->lock();
      w->reset();
      w->color(255,0,0);
      w->fill(0,0,0,0);
      
      w->rect(image.getROI().x,image.getROI().y,image.getROI().width,image.getROI().height);
      w->unlock();
    }else{
      std::cout << "roi visualization is not supported in multi image mode!" << std::endl;
    }
  }
  
  if(image.getDim()){
    w->setImage(&image);
    w->resize(QSize(image.getWidth(),image.getHeight()));
  }
  
  app.exec();
}
