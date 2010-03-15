/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
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
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLAlgorithms/HoughLineDetector.h>

typedef FixedColVector<float,2> Pos;

GUI gui("hsplit"),con("vsplit");
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
  con << "image[@handle=lut@label=hough space@minsize=16x12]";
  con << ( GUI("vbox") 
           << "image[@handle=inhibit@label=local inhibition image@minsize=8x6]"
           << "slider(0,100,1)[@out=maxlines@label=max lines]"
           << "fslider(0.01,1,0.03)[@out=dRho@label=rho sampling step]"
           << "fslider(0.1,10,2)[@out=dR@label=r sampling step]"
           << "fslider(2,100,10)[@out=rInhib@label=r inhibition radius]"
           << "fslider(0.05,2,0.3)[@out=rhoInhib@label=rho inhibition radius]"
           << ( GUI("hbox") 
                << "togglebutton(off,!on)[@out=gaussianInhibition@label=gaussian inhib]"
                << "togglebutton(off,!on)[@out=blurHoughSpace@label=blur hough space]"
                )
           << ( GUI("hbox") 
                << "togglebutton(off,!on)[@out=dilateEntries@label=dilate entries]"
                << "togglebutton(off,!on)[@out=blurredSampling@label=blurred sampling]"
                )
           );

  gui << "draw[@handle=view@minsize=32x24]" 
      << con;
  
  gui.show();

  gui_DrawHandle(view);
  (*view)->install(new Mouse);
  view = Img8u(Size::VGA,1);
  view.update();
}

void run(){
  gui_int(maxlines);
  gui_float(dRho);
  gui_float(dR);
  gui_float(rInhib);
  gui_float(rhoInhib);
  gui_ImageHandle(lut);
  gui_ImageHandle(inhibit);
  gui_DrawHandle(view);
  gui_bool(gaussianInhibition);
  gui_bool(blurHoughSpace);
  gui_bool(dilateEntries);
  gui_bool(blurredSampling);
  
  HoughLineDetector h(dRho,dR,Range32f(0,sqrt(640*640+480*480)),rInhib,rhoInhib,
                      gaussianInhibition,blurHoughSpace,dilateEntries,blurredSampling);
  points_mutex.lock();
  h.add(points);

  std::vector<float> sig;
  std::vector<StraightLine2D> ls = h.getLines(maxlines,sig);

  (*lut)->setRangeMode(ICLWidget::rmAuto);
  (*lut)->setFitMode(ICLWidget::fmFit);
  lut = h.getImage();
  lut.update();
  
  
  if(gaussianInhibition){
    (*inhibit)->setRangeMode(ICLWidget::rmAuto);
    (*inhibit)->setFitMode(ICLWidget::fmFit);
    inhibit = h.getInhibitionMap();
    inhibit.update();
  }
  

  (*view)->lock();
  (*view)->reset();
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

  (*view)->unlock();
  points_mutex.unlock();  
  
  Thread::msleep(10);
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
