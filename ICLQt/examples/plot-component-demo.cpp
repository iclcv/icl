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
#include <ICLUtils/FPSLimiter.h>

GUI gui("vbox");

void init(){
  std::string gl = pa("-gl") ? "gl" : "noGL";
  gui << (GUI("hbox") 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot1@minsize=16x12]"
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot2@minsize=16x12]"
          )
      << (GUI("hbox") 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot3@minsize=16x12]" 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot4@minsize=16x12]" 
          ) << "!show";
}

void run(){
  static PlotHandle plots[] = { 
    gui["plot1"], gui["plot2"],
    gui["plot3"], gui["plot4"]
  };

  static std::vector<float> sinData(100);
  static std::vector<float> cosData(100);
  static std::vector<float> tanData(100);

  static Time t = Time::now();
  float dtSec = (Time::now()-t).toSecondsDouble();
  for(unsigned int i=0;i<sinData.size();++i){
    float relI = float(i)/sinData.size();
    sinData[i] = sin(relI * 2*M_PI + dtSec);
    cosData[i] = cos(relI * 2*M_PI + dtSec);
    tanData[i] = cosData[i] > 1.E-10 ? sinData[i]/cosData[i] : 1.E30;
  }
  for(int i=0;i<4;++i){
    PlotHandle &plot = plots[i];
    plot->lock();
    plot->clear();
    plot->setPropertyValue("tics.y-distance",0.25);
    if(i==0 || i==3){
      plot->addSeriesData(sinData.data(), sinData.size(), 
                          new AbstractPlotWidget::Pen(QColor(255,0,0),Qt::NoPen,' ',5, QColor(255,0,0,100)),
                          "sin(x)");
      if(i==0){
        plot->setPropertyValue("enable fill",true);
      }
    }
    if(i==1 || i == 3){
      plot->addSeriesData(cosData.data(), cosData.size(), 
                          new AbstractPlotWidget::Pen(QColor(0,255,0),QColor(0,255,0),'o',2, QColor(0,255,0,100)),
                          "cos(x)");
      if(i==3) plot->setPropertyValue("enable symbols",false);
    }
    if(i==2 || i == 3){
      plot->addSeriesData(tanData.data(), tanData.size(), 
                          new AbstractPlotWidget::Pen(QColor(0,100,255),Qt::NoPen,' ',2, QColor(0,100,255,100)),
                          "tan(x)");
    }

    plot->unlock();
    plot.update();
  }

  static FPSLimiter fpsLimit(20,10);
  fpsLimit.wait();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-use-opengl|-gl",init,run).exec();
}
