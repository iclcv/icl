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

#include <ICLUtils/Random.h>
#include <deque>

GUI gui("hbox");

void init(){
  std::string gl = pa("-gl") ? "gl" : "noGL";
  gui << (GUI("vbox") 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot1@minsize=15x12]"
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot2@minsize=15x12]"
          )
      << (GUI("vbox") 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot3@minsize=15x12]" 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot4@minsize=15x12]" 
          )
      << (GUI("vbox") 
          << "plot(-9,9,-9,9,"+gl+")[@handle=plot5@minsize=15x12]" 
          << "plot(0,0,0,0,"+gl+")[@handle=plot6@minsize=15x12]" 
          )
      << (GUI("vbox") 
          << "plot(0,0,0,0,"+gl+",something [pi])[@handle=plot7@minsize=15x12]" 
          << "plot(0,0,0,0,"+gl+")[@handle=plot8@minsize=15x12]" 
          << "checkbox(animate,checked)[@out=run]"
         ) << "!show";
}

void run(){
  while(!gui["run"].as<bool>()){
    Thread::msleep(100);
  }
  static PlotHandle plots[] = { 
    gui["plot1"], gui["plot2"],
    gui["plot3"], gui["plot4"],
    gui["plot5"], gui["plot6"],
    gui["plot7"], gui["plot8"]
  };

  static std::vector<float> sinData(100);
  static std::vector<float> cosData(100);
  static std::vector<float> tanData(100);
  static std::vector<Point32f> scatterData1(5000);
  static std::vector<Point32f> scatterData2(5000);
  static std::vector<Point32f> scatterData3(1000);
  static Time t = Time::now();
  float dtSec = (Time::now()-t).toSecondsDouble();


  static Point32f sinSeries[101];
  static int sinSeriesUsed = 0;
  sinSeries[sinSeriesUsed++] = Point32f(sin(dtSec), dtSec/M_PI);
  if(sinSeriesUsed == 101){
    for(int i=0;i<100;++i){
      sinSeries[i] = sinSeries[i+1];
    }
    sinSeriesUsed = 100;
  }
  
  static GRand grand;
  static const float C[] = { 1.9, 1.2, 
                             1.2, 2.8 };
  for(unsigned int i=0;i<scatterData1.size();++i){
    Point32f p(grand, grand);
    scatterData1[i].x = p.x * C[0] + p.y * C[1];
    scatterData1[i].y = p.x * C[2] + p.y * C[3];

    scatterData2[i].x = p.x * C[0] + p.y * -C[1] + 1;
    scatterData2[i].y = p.x * -C[2] + p.y * C[3] + 0.5;
  }
  for(unsigned int i=0;i<scatterData3.size();++i){
    float rel = float(i)/(scatterData3.size()-1) * 2 * M_PI;
    float r = 2 + 3 * sin(4*rel+dtSec);
    scatterData3[i].x  = r * cos(rel);
    scatterData3[i].y  = r * sin(rel);
  }

  for(unsigned int i=0;i<sinData.size();++i){
    float relI = float(i)/sinData.size();
    sinData[i] = sin(relI * 2*M_PI + dtSec);
    cosData[i] = cos(relI * 2*M_PI + dtSec);
    tanData[i] = cosData[i] > 1.E-10 ? sinData[i]/cosData[i] : 1.E30;
  }

  for(int i=0;i<8;++i){
    PlotHandle &plot = plots[i];
    plot->lock();
    plot->clear();

    if(i < 4){
      plot->setPropertyValue("tics.y-distance",0.25);
    }else if(i<6){
      plot->setPropertyValue("tics.y-distance",3);
      plot->setPropertyValue("tics.x-distance",3);
    }else{
      plot->setPropertyValue("tics.y-distance",0.5);
      plot->setPropertyValue("tics.x-distance",1);
    }
    if(i==0 || i==3){
      plot->linewidth(1);
      plot->color(255,0,0);
      if(i==0) plot->fill(255,0,0,100);
      plot->label("sin(x)");
      plot->series(sinData);
    }
    if(i==1 || i == 3){
      plot->color(0,255,0);
      if(i==1) plot->sym('o',2);
      plot->label("cos (x)");
      plot->series(cosData);
    }
    if(i==2 || i == 3){
      plot->color(0,100,255);
      plot->label("tan (x)");
      plot->series(tanData);
    }
    if(i == 4){
      plot->label("red noise");
      plot->color(255,0,0);
      plot->scatter(scatterData1);

      plot->label("dirty blue noise");
      plot->color(0,100,255);
      plot->scatter(scatterData2);
    }
    if(i == 5){
      plot->color(0,200,255);
      plot->label("some nice shape");
      plot->scatter(scatterData3,true);
      plot->color(255,0,0);
      plot->fill(255,0,0,100);
      plot->rect(-.2,-.2,.4,.4);
      plot->line(0,0,3,3);
      plot->text(3,3,"the center");

    }
    if( i == 6){
      plot->setDataViewPort(Range32f(sinSeries[0].y, sinSeries[sinSeriesUsed-1].y), Range32f(0,0));
      plot->color(255,0,0);
      plot->label("continous data");
      plot->series(&sinSeries[0].x, sinSeriesUsed, 2);
      //      plot->addSeriesData(&sinSeries[0].x, sinSeriesUsed,
      //                    new AbstractPlotWidget::Pen(QColor(255,0,0)), "continous data", 2);
    }

    if(i == 7){
      //float xs[] = { 1,2,3,4};
      //float ys[] = { 1,2,3,4};
      const float data[8] = {.1,.2,.3,.4,.5,.6,-.7,.3};
      const float data2[8] = {0.7,.1,.2,.3,.4,.5,.6,-.7};
      const float data3[10] = {.3,.4,.5,0.7,.1,.2,.3,.4,.5,.6};
      plot->color(255,0,0);
      plot->fill(255,0,0,100);
      plot->label("red bars");
      plot->bars(data,8);

      plot->color(0,255,0);
      plot->fill(0,255,0,100);
      plot->label("green bars");
      plot->bars(data2,8);

      plot->color(0,100,255);
      plot->fill(0,100,255,100);
      plot->label("blue bars");
      plot->bars(data3,10);
      //plot->addScatterData('x',xs,ys,4,"dummy data");
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
