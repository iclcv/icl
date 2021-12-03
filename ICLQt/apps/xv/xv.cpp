/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/apps/xv/xv.cpp                                   **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <iostream>
#include <iterator>
#include <ICLIO/FileGrabber.h>
#include <ICLQt/Common.h>
#include <QDesktopWidget>

//#include <QScreen>

GUI gui;

Size compute_image_size(const std::vector<const ImgBase*> &is, QDesktopWidget *desktop){
  Size s;
  for(unsigned int i=0;i<is.size();++i){
    s.width = iclMax(s.width,is[i]->getWidth());
    s.height = iclMax(s.height,is[i]->getHeight());
  }
  QRect r = desktop->availableGeometry();

  return Size(iclMin(s.width,r.width()-20),iclMin(s.height,r.height()-20));
}

int main (int n, char **ppc){
  pa_set_help_text("icl-xv is ICL's simple image viewer.\n"
                   "It can display images of all supported file formats.\n"
                   "Furthermore, icl-xv is used for ICLQuick's\n"
                   "global icl::show(image) function. Here, the -delete\n"
                   "flag is used in order to delete a temporary image\n"
                   "after loading it.");
  ICLApplication app(n,ppc);
  pa_explain
  ("-input","define image to read")
  ("-delete","delete image file after reading")
  ("-roi","if set, image roi is visualized")
  ("-fs","shows the widget in fullscreen mode on the given\n"
   "screen ID (or any screen if the passed id is -1).\n"
   "The -fs flag only works when explicitly defining the input using the -input|-i arg!");
  pa_init(n,ppc,"-input|-i(filename) -delete|-d -roi|-r -fullscreen|-fs(screen)",true);

  const ImgBase *image = 0;
  if(pa("-input")){
    std::string imageName = pa("-input").as<std::string>();

    try{
      static FileGrabber w(imageName);
      image = w.grab();
      if(pa("-delete")){
        if(imageName.length()){
          int errorCode = system((std::string(ICL_SYSTEMCALL_RM) + imageName).c_str());
          if ( errorCode != 0 )
            WARNING_LOG( "Error code of system call unequal 0!" );
        }
      }
    }catch(const ICLException& e){
      static ImgQ o = ones(320,240,1)*100;
      fontsize(15);
      text(o, 90,90,"image not found!");
      image = &o;
    }
    if(pa_get_count()){
      std::cout << "Warning if called with -input, all extra given filenames are omitted!" << std::endl;
    }

    Size size = compute_image_size(std::vector<const ImgBase*>(1,image),QApplication::desktop());
    gui << Draw().handle("draw").size(size/20);
    gui.show();

    DrawHandle draw = gui["draw"];
    draw = image;
    draw.render();

    if(pa("-fs")){
      draw->setFullScreenMode(true,pa("-fs"));
    }
  }else if(pa_get_count()){
    if(pa("-delete")){
      std::cout << "-delete flag is not supported when running in multi image mode" << std::endl;
      std::cout << "call iclxv -input ImageName -delete instead (for single images only)" << std::endl;
    }
    std::string imageList = "";
    std::vector<const ImgBase*> imageVec;
    std::vector<std::string> imageVecStrs;

    Size maxSize;
    for(unsigned int i=0;i<pa_get_count();++i){
      std::string s = pa(i).as<std::string>();
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
    Tab t(imageList);

    for(size_t i=0;i<imageVecStrs.size();++i){
      t << Image().handle("image-"+str(i)).size(size/20);
    }

    gui << t << Show();

    for(size_t i=0;i<imageVecStrs.size();++i){
      gui["image-"+str(i)] = imageVec[i];
      ICL_DELETE(imageVec[i]);
    }
  }else{
    throw ICLException("no image given, or none of the given images was found!");
  }

  if(pa("-roi")){
    if(pa("-input")){
      static DrawHandle draw = gui["draw"];
      draw->color(255,0,0);
      draw->fill(0,0,0,0);
      draw->rect(image->getROI().x,image->getROI().y,image->getROI().width,image->getROI().height);
      draw.render();
    }else{
      std::cout << "roi visualization is not supported in multi image mode!" << std::endl;
    }
  }





  return app.exec();
}
