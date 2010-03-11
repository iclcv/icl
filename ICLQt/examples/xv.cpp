/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <iostream>
#include <iterator>
#include <ICLIO/FileGrabber.h>
#include <ICLQuick/Common.h>
#include <QDesktopWidget>

//#include <QScreen>

GUI gui;

Size compute_image_size(const std::vector<ImgBase*> &is, QDesktopWidget *desktop){
  Size s;
  for(unsigned int i=0;i<is.size();++i){
    s.width = iclMax(s.width,is[i]->getWidth());
    s.height = iclMax(s.height,is[i]->getHeight());
  }
  QRect r = desktop->availableGeometry();
  
  return Size(iclMin(s.width,r.width()-20),iclMin(s.height,r.height()-20));
}

int main (int n, char **ppc){
  QApplication app(n,ppc);
  paex
  ("-input","define image to read")
  ("-delete","delete image file after reading")
  ("-roi","if set, image roi is visualized");
  painit(n,ppc,"-input|-i(filename) -delete|-d -roi|-r",true);

  ImgQ image;  
  if(pa("-input")){
    string imageName = pa("-input");
  
    try{
      image = load(imageName);
      if(pa("-delete")){
        if(imageName.length()){
          system((string("rm -rf ")+imageName).c_str());
        }
      }
    }catch(ICLException e){
      image = ones(320,240,1)*100;
      fontsize(15); 
      text(image, 90,90,"image not found!");
    }
    if(pacount()){
      std::cout << "Warning if called with -input, all extra given filenames are omitted!" << std::endl;
    }

    Size size = compute_image_size(std::vector<ImgBase*>(1,&image),QApplication::desktop());
    gui = GUI("image[@handle=draw@size="+str(size/20)+"]");
    gui.show();
    
    gui["draw"] = image;
    gui["draw"].update();
  
  }else if(pacount()){
    if(pa("-delete")){
      std::cout << "-delete flag is not supported when running in multi image mode" << std::endl;
      std::cout << "call iclxv -input ImageName -delete instead (for single images only)" << std::endl;
    }
    std::string imageList = "";
    std::vector<ImgBase*> imageVec;
    std::vector<std::string> imageVecStrs;
    Size maxSize;
    for(unsigned int i=0;i<pacount();++i){
      std::string s = pa(i);
      try{
        FileGrabber grabber(s,false,true);
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
    Size size = compute_image_size(imageVec,QApplication::desktop());
    gui << std::string("multidraw(")+imageList+",!all,!deepcopy)[@handle=image@size="+str(size/20)+"]";
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

  if(pa("-roi")){
    if(pa("-input")){
      gui_DrawHandle(draw);
      draw->lock();
      draw->reset();
      draw->color(255,0,0);
      draw->fill(0,0,0,0);
      
      draw->rect(image.getROI().x,image.getROI().y,image.getROI().width,image.getROI().height);
      draw->unlock();
    }else{
      std::cout << "roi visualization is not supported in multi image mode!" << std::endl;
    }
  }

  return app.exec();
}
