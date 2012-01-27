/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/plot-compoment-demo.cpp                 **
** Module : ICLQt                                                  **
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

GUI gui;

void init(){
  gui << "plot(0,6.5,-1,1,GL)[@handle=plot@minsize=32x24]" << "!show";
}

void run(){
  static PlotHandle plot = gui["plot"];
  static std::vector<float> sinData(1000);
  static Time t = Time::now();
  float dtSec = (Time::now()-t).toSecondsDouble();
  for(unsigned int i=0;i<sinData.size();++i){
    float relI = float(i)/sinData.size();
    sinData[i] = sin(relI * 2*M_PI + dtSec);
  }
  plot->lock();
  plot->clear();
  //plot->addSeriesData(sinData.data()+10, sinData.size()-10, new AbstractPlotWidget::Pen(QColor(0,255,0)));
  //plot->addSeriesData(sinData.data()+20, sinData.size()-20, new AbstractPlotWidget::Pen(QColor(0,100,255)));
  plot->addSeriesData(sinData.data(), sinData.size(), new AbstractPlotWidget::Pen(QColor(255,0,0)));

  plot->unlock();
  plot.update();

  Thread::msleep(25);
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
}
