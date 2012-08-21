/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/examples/interactive-hough-line-demo.cpp  **
** Module : ICLAlgorithms                                          **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLAlgorithms/HoughLineDetector.h>

typedef FixedColVector<float,2> Pos;

HSplit gui;
HoughLineDetector *h = 0;

std::vector<Point32f> points;
Mutex points_mutex;

struct Mouse : public MouseHandler {
  virtual void process(const MouseEvent &e){
    if(e.isLeft()){
      Mutex::Locker l(points_mutex);
      points.push_back(e.getPos());
    }else if(e.isRight()){
      Mutex::Locker l(points_mutex);
      points.clear();
   } 
  }
};

void init(){

  gui << Draw().handle("view").minSize(32,24)
      << ( VSplit()
           << Image().handle("lut").label("hough space").minSize(16,12)
           << ( VBox() 
                << Image().handle("inhibit").label("local inhibition image").minSize(8,6)
                << Slider(0,100,1).out("maxlines").label("max lines")
                << FSlider(0.01,1,0.03).out("dRho").label("rho sampling step")
                << FSlider(0.1,10,2).out("dR").label("r sampling step")
                << FSlider(2,100,10).out("rInhib").label("r inhibition radius")
                << FSlider(0.05,2,0.3).out("rhoInhib").label("rho inhibition radius")
                << ( HBox() 
                     << Button("off","!on").out("gaussianInhibition").label("gaussian inhib")
                     << Button("off","!on").out("blurHoughSpace").label("blur hough space")
                     )
                << ( HBox() 
                     << Button("off","!on").out("dilateEntries").label("dilate entries")
                     << Button("off","!on").out("blurredSampling").label("blurred sampling")
                     )
                )

           )
      << Show();

  gui["view"].install(new Mouse);
  gui["view"] = Img8u(Size::VGA,1);
  gui["view"].render();
}

void run(){
  int maxlines = gui["maxlines"];
  float dRho = gui["dRho"];
  float dR = gui["dR"];
  float rInhib = gui["rInhib"];
  float rhoInhib = gui["rhoInhib"];
  ImageHandle lut = gui["lut"];
  ImageHandle inhibit = gui["inhibit"];
  DrawHandle view = gui["view"];
  bool gaussianInhibition = gui["gaussianInhibition"];
  bool blurHoughSpace = gui["blurHoughSpace"];
  bool dilateEntries = gui["dilateEntries"];
  bool blurredSampling = gui["blurredSampling"];
  
  HoughLineDetector h(dRho,dR,Range32f(0,sqrt(640*640+480*480)),rInhib,rhoInhib,
                      gaussianInhibition,blurHoughSpace,dilateEntries,blurredSampling);
  points_mutex.lock();
  h.add(points);

  std::vector<float> sig;
  std::vector<StraightLine2D> ls = h.getLines(maxlines,sig);

  (*lut)->setRangeMode(ICLWidget::rmAuto);
  (*lut)->setFitMode(ICLWidget::fmFit);
  lut = h.getImage();
  lut.render();
  
  
  if(gaussianInhibition){
    (*inhibit)->setRangeMode(ICLWidget::rmAuto);
    (*inhibit)->setFitMode(ICLWidget::fmFit);
    inhibit = h.getInhibitionMap();
  }
  

  (*view)->color(255,255,255,255);
  (*view)->fill(255,255,255,255);
  for(unsigned int i=0;i<points.size();++i){
    Point32f p = points[i];
    (*view)->point(p.x,p.y);
  }

  (*view)->fill(255,255,255,0);
  (*view)->color(255,0,0,255);
  for(unsigned int i=0;i<ls.size();++i){
    StraightLine2D &l = ls[i];
    Pos a = l.o+l.v*1000;
    Pos b = l.o-l.v*1000;
    (*view)->line(a[0],a[1],b[0],b[1]);
    (*view)->text(str(sig[i]),l.o[0],l.o[1],-1,-1,8);
  }
  
  points_mutex.unlock();  
  view->render();
  Thread::msleep(10);
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
