/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/apps/crop/crop.cpp                               **
** Module : ICLCV                                                  **
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

#include <ICLQt/Common.h>
#include <ICLFilter/ImageRectification.h>
#include <ICLFilter/RotateOp.h>
#include <ICLQt/DefineRectanglesMouseHandler.h>
#include <ICLIO/GenericImageOutput.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLIO/FileList.h>

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QLabel>

GenericGrabber grabber;
HSplit gui;
Img8u curr;
Mutex currMutex;
Rect lastRect;

struct Mouse1 : public MouseHandler{
  std::vector<Point> ps;
  Rect bounds;
  int handles[4];
  int dragged;
  Mouse1(const Size &maxSize):ps(4),bounds(Point::null,maxSize-Size(1,1)){

    Rect r = bounds.enlarged(-iclMin(50,iclMax(maxSize.width,maxSize.height)/2));
    ps[0] = r.ul();
    ps[1] = r.ur();
    ps[2] = r.lr();
    ps[3] = r.ll();
    std::fill(handles,handles+4,0);
    dragged = -1;
  }
  virtual void process(const MouseEvent &e){
    if(gui["rect"].as<bool>()) return;
    Point p = e.getPos();
    
    if(!bounds.contains(p.x,p.y)){
      return;
    }

    if(e.isReleaseEvent()){
      dragged = -1;
      std::fill(handles,handles+4,0);
    }
    if(e.isPressEvent()){
      for(int i=0;i<4;++i){
        if(ps[i].distanceTo(p) < 8){
          dragged = i;
          handles[i] = 2;
        }
      }
    }
    if(e.isDragEvent()){
      if(dragged != -1){
        ps[dragged] = p;
      }
    }else if(e.isMoveEvent()){
      for(int i=0;i<4;++i){
        if(ps[i].distanceTo(p) < 8){
          handles[i] = 1;
        }else{
          handles[i] = 0;
        }
      }
    }
  }

  VisualizationDescription vis() const{
    VisualizationDescription d;
    d.color(255,0,0,255);
    d.linewidth(2);
    d.fill(255,0,0,40);
    std::vector<Point> pso = ps;
    d.polygon(pso);
    d.linewidth(1);
    for(int i=0;i<4;++i){
      d.fill(255,0,0,1+127*handles[i]);
      d.rect(pso[i].x-5,pso[i].y-5,11,11);
    }
    return d;
  }
} *mouse_1 = 0;


struct Mouse2 : public DefineRectanglesMouseHandler{
  Mouse2(const Size &size) : DefineRectanglesMouseHandler(1,5){
    getOptions().handleWidth = 10;
    addRect(Rect(Point::null,size).enlarged(-5));
  }
  
  virtual void process(const MouseEvent &e){
    if(gui["rect"].as<bool>() && !e.isRight()){
      DefineRectanglesMouseHandler::process(e);
    }
  }
} *mouse_2 = 0;

void rectangular_changed(){
  if(gui["rect"]){
    gui["s1"].disable();
    gui["s2"].disable();
  }else{
    gui["s1"].enable();
    gui["s2"].enable();
  }
}

void save_as(){
  Mutex::Locker lock(currMutex);
  try{
    std::string filename = saveFileDialog();
    save(curr,filename);
  }catch(...){}
}

void overwrite(){
  Mutex::Locker lock(currMutex);
  std::string filename = pa("-i", 1).as<std::string>();
  save(curr,filename);
}

static GUI *batchGUI = 0;

template<bool performCrop>
void batch_pattern_changed(){
  BoxHandle box = (*batchGUI)["matches"];
  
  QList<QWidget*> widgets = box.getScroll()->widget()->findChildren<QWidget*>();
  foreach(QWidget * widget, widgets){
    delete widget;
  }
  // while(box.getScroll()->widget()->layout()->count()){
  //  std::cout << "deleting entry" << std::endl;
  //  delete box.getScroll()->widget()->layout()->takeAt(0);
  //}
  std::vector<utils::Rect> rs = mouse_2->getRects();
  Rect r = rs[0];
  try{
    StringHandle shi = (*batchGUI)["ipat"];
    StringHandle sho = (*batchGUI)["opat"];
    FileList f(shi.getValue());
    GenericImageOutput out("file","file="+sho.getValue());
    for(int i=0;i<f.size();++i){
      try{
        ImgQ image = qt::load(f[i]);
        if(!image.getDim()) throw 42;
        box.add(new QLabel(f[i].c_str()));
        ImgBase *dst = 0;
        if(performCrop){
          GenericGrabber g("file","file="+f[i]);
          const ImgBase *image = g.grab()->shallowCopy(r);
          image->deepCopyROI(&dst);
          out.send(dst);
          std::cout << "converted file " << f[i] << " successfully" << std::endl;
          delete image;
        }
        ICL_DELETE(dst);
      }catch(...){
        //        WARNING_LOG("referenced file " << f[i] << " could not be loaded [skipped]");
      }
    }
  }catch(...){}
}

void batch_crop(){
  if(!batchGUI){
    batchGUI = new HSplit;
    (*batchGUI) << ( VBox()
                     << String("").handle("ipat").label("input file pattern")
                     << String("converted-####.png").handle("opat").label("output file pattern")
                     << Button("do it!").handle("do it")
                   )
                << VScroll().handle("matches").label("matching files")
                << Create();
    (*batchGUI)["ipat"].registerCallback(batch_pattern_changed<false>);
    (*batchGUI)["do it"].registerCallback(batch_pattern_changed<true>);
  }
  (*batchGUI).show();
}

void init(){
  grabber.init(pa("-i"));

  bool c_arg = pa("-c");

  gui << Draw().handle("draw").label("input image")
      << Image().handle("cropped").label("cropped")
      << ( VBox().maxSize(c_arg ? 0 : 12,99).minSize(c_arg ? 0 : 12,1)
           << Button("save as ..").handle("saveAs")
           << Button("overwrite input").handle("overwrite")
           << Combo("0,90,180,270").handle("rot").label("rotation")
           << CheckBox("rectangular",!pa("-r")).handle("rect")
           << Button("Batch crop...").handle("batch")
           << ( HBox().label("rectification size")
                << Spinner(1,4096,640).handle("s1")
                << Label(":")
                << Spinner(1,4096,480).handle("s2")
                )
           << (HBox() 
               << Fps().handle("fps")
               << CamCfg()
               )
           )
      << Show();


  if(!c_arg){
    gui["batch"].registerCallback(batch_crop);
  }
  const ImgBase *image = grabber.grab();
  if(!c_arg){
    mouse_1 = new Mouse1(image->getSize());
    gui["draw"].install(mouse_1);
  }
  mouse_2 = new Mouse2(image->getSize());
  gui["draw"].install(mouse_2);
  
  DrawHandle draw = gui["draw"];
  draw->setImageInfoIndicatorEnabled(false);
  
  if(!c_arg){
    gui["rect"].registerCallback(rectangular_changed);
    rectangular_changed();
    if(*pa("-i",0) != "file" || FileList(*pa("-i",1)).size() != 1){
      gui["overwrite"].disable();
    }else{
      gui["overwrite"].registerCallback(overwrite);
    }
    gui["saveAs"].registerCallback(save_as);
  }

}

void run(){
  bool c_arg = pa("-c");
  
  static FPSLimiter fpsLimit(30);
  fpsLimit.wait();

  const ImgBase *image = grabber.grab();
  DrawHandle draw = gui["draw"];
  ImageHandle cropped = gui["cropped"];

  draw = image;

  static RotateOp rot;
  if(c_arg){
    rot.setAngle(0);
  }else{
    rot.setAngle(parse<int>(gui["rot"]));
  }

  const ImgBase *cro = 0;
  if(c_arg || gui["rect"].as<bool>()){
    static Img8u roi;
    std::vector<utils::Rect> rs = mouse_2->getRects();
    ICLASSERT_THROW(rs.size() == 1, ICLException("expected exactly one rectangle"));
    lastRect = rs[0];
    mouse_2->visualize(**draw);
    SmartPtr<const ImgBase> tmp = image->shallowCopy(rs[0] & image->getImageRect());
    roi.setChannels(tmp->getChannels());
    roi.setFormat(tmp->getFormat());
    roi.setSize(tmp->getROISize());
    tmp->convertROI(&roi);
    cro = rot.apply(&roi);

    draw->color(0,255,0,255);
    draw->text(str(rs[0]), rs[0].x, rs[0].y);

  }else{
    draw->draw(mouse_1->vis());
    Size32f s(gui["s1"],gui["s2"]);
    Point32f ps[4] = { mouse_1->ps[0],  mouse_1->ps[1],  mouse_1->ps[2],  mouse_1->ps[3] };
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                \
      case depth##D:{                                           \
        static ImageRectification<icl##D> ir;                   \
        try{                                                    \
          cro = rot.apply(&ir.apply(ps,*image->as##D(),s));     \
        }catch(...){}                                           \
        break;                                                  \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
  if(cro){
    cropped = cro;
    currMutex.lock();
    cro->convert(&curr);
    currMutex.unlock();
  }

  gui["draw"].render();
  gui["fps"].render();
}


int main(int n, char **args){
  pa_init(n,args,"[m]-input|-i(2) -rectification|-r -create-crop-rect-output-only|-ccroo|-c "
          "-estimate-image-size-only -estimate-image-ar-only "
          "-compute-optimal-scaling-size(target-width) -compute-optimal-scaling-size-input-size(Size=0x0)");
  if(pa("-estimate-image-size-only") || pa("-estimate-image-ar-only") || pa("-compute-optimal-scaling-size")){
    GenericGrabber g(pa("-input"));
    Size s = g.grab()->getSize();
    if(pa("-estimate-image-size-only")){
      std::cout << s << std::endl;
    }
    if(pa("-estimate-image-ar-only")){
      std::cout << float(s.width)/float(s.height) << std::endl;
    }
    if(pa("-compute-optimal-scaling-size")){
      int W = pa("-compute-optimal-scaling-size");
      Size sinput = pa("-compute-optimal-scaling-size-input-size");
      if(sinput == Size(0,0)) sinput = s;
      float f = float(W)/float(sinput.width);
      std::cout << Size(W,round(sinput.height * f)) << std::endl;
    }

    return 0;
  }
  ICLApp app(n,args,"",init,run);
  int result = app.exec();
  std::cout << lastRect << std::endl;
  return result;
}
