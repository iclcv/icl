/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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
          << "plot(-5,5,-5,5,"+gl+")[@handle=plot9@minsize=15x12]"
          )
      << (GUI("vbox") 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot3@minsize=15x12]" 
          << "plot(0,6.5,-1,1,"+gl+")[@handle=plot4@minsize=15x12]"
          << "plot(0,0,0,0,"+gl+")[@handle=plot10@minsize=15x12]" 
          )
      << (GUI("vbox") 
          << "plot(-9,9,-9,9,"+gl+")[@handle=plot5@minsize=15x12]" 
          << "plot(0,0,0,0,"+gl+")[@handle=plot6@minsize=15x12]" 
          << "plot(0,0,0,0,"+gl+")[@handle=plot11@minsize=15x12]"
          )
      << (GUI("vbox") 
          << "plot(0,0,0,0,"+gl+",something [pi])[@handle=plot7@minsize=15x12]" 
          << "plot(0,0,0,0,"+gl+")[@handle=plot8@minsize=15x12]" 
          << "plot(0,0,0,0,"+gl+")[@handle=plot12@minsize=15x12]"
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
    gui["plot7"], gui["plot8"],
    gui["plot9"], gui["plot10"],
    gui["plot11"], gui["plot12"],
  };

  static std::vector<float> sinData(100);
  static std::vector<float> cosData(100);
  static std::vector<float> tanData(100);
  static std::vector<Point32f> scatterData1(5000);
  static std::vector<Point32f> scatterData2(5000);
  static std::vector<Point32f> scatterData3(1000);
  static Time t = Time::now();
  float dtSec = (Time::now()-t).toSecondsDouble();

  static Array2D<Point32f> grid(30,30);
  static bool first = true;
  if(first){
    first = false;
    for(int x=0;x<grid.getWidth();++x){
      for(int y=0;y<grid.getHeight();++y){
        grid(x,y) = Point32f(x,y);
      }
    }
  }
  Range32f rxGrid = Range32f::limits(), ryGrid=Range32f::limits();
  std::swap(rxGrid.minVal, rxGrid.maxVal);
  std::swap(ryGrid.minVal, ryGrid.maxVal);
  
  for(int i=0;i<grid.getDim();++i){
    GRand gr(0,0.01*Point32f(grid.getWidth()/2-1, grid.getHeight()/2-1).distanceTo(Point32f(i%grid.getWidth(),i/grid.getWidth())));
    grid[i] += Point32f(gr,gr);
    rxGrid.extend(grid[i].x);
    ryGrid.extend(grid[i].y);
  }
  
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
  
  static std::vector<float> fData(100);
  for(size_t xi=0;xi<fData.size();++xi){
    float x = xi/float(fData.size()-1);
    fData[xi] = (200-dtSec)*x*x*x*x - (60+dtSec)*x*x*x - (dtSec/10)*5*x*x - 32*x - 8;
  }
  static std::vector<Point32f> fDataSamples(100);
  for(size_t i=0;i<fDataSamples.size();++i){
    URand r(0,1);
    GRand gr(0,4);
    float x = r;
    float fx = (200-dtSec)*x*x*x*x - (60+dtSec)*x*x*x - (dtSec/10)*5*x*x - 32*x - 8 + gr;
    fDataSamples[i] = Point32f(x,fx);
  }
  /// try polynomial regression here!
  DynMatrix<float> X(fDataSamples.size(), 5);
  DynMatrix<float> Y(fDataSamples.size(), 1);
  
  for(size_t i=0;i<fDataSamples.size();++i){
    const float x = fDataSamples[i].x, y = fDataSamples[i].y;
    X(i,0) = 1; 
    X(i,1) = x;
    X(i,2) = x*x;
    X(i,3) = x*x*x;
    X(i,4) = x*x*x*x;
    Y[i] = y;
  }
  DynMatrix<float> R = Y*X.pinv(false);
  static std::vector<float> fApprox(fData.size());
  for(size_t xi=0;xi<fApprox.size();++xi){
    float x = xi/float(fData.size()-1);
    fApprox[xi] = R[0] + R[1]*x + R[2] *x*x + R[3]*x*x*x + R[4]*x*x*x*x;
  }
  /// xxx

  

  for(int i=0;i<12;++i){
    PlotHandle &plot = plots[i];
    plot->lock();
    if(i!=8 ){
      plot->clear();
    }

    if(i < 4){
      plot->setPropertyValue("tics.y-distance",0.25);
    }else if(i<6 || i>=8){
      plot->setPropertyValue("tics.y-distance",3);
      plot->setPropertyValue("tics.x-distance",3);
    }else{
      plot->setPropertyValue("tics.y-distance",0.5);
      plot->setPropertyValue("tics.x-distance",1);
    }
    if(i==0 || i==3){
      plot->linewidth(1);
      plot->color(255,0,0);
      if(i==0){
        plot->fill(255,0,0,100);
        plot->setPropertyValue("enable fill",true);
      }
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
      plot->setDataViewPort(Range32f(0,4),Range32f(-.8,.8));
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

    if(i == 8){
      static Time last = Time::now();
      float dtS = (Time::now() -last).toSecondsDouble();
      if( dtS > 5){
        plot->clear();
        last = Time::now();
      }
      
      plot->color(dtS*50,0,255-dtS*50);
      plot->linestrip(scatterData3);
      plot->color(0,0,0);
      plot->title("shape drawn as linestrip");
      plot->setPropertyValue("borders.top",18);
    }
    if(i == 9){
      plot->color(255,0,0);
      plot->grid(grid);
      plot->setDataViewPort(rxGrid,ryGrid);
      plot->setPropertyValue("labels.x-precision",0);
      plot->setPropertyValue("labels.y-precision",0);
      plot->title("some distorted grid");
      plot->setPropertyValue("borders.top",18);
    }
    if(i == 10){
      plot->fill(255,0,0,100);
      plot->setDataViewPort(Range32f(0,1),Range32f(-50,50));
      plot->setPropertyValue("enable fill",true);
      plot->setPropertyValue("tics.x-distance",0.2);
      plot->setPropertyValue("tics.y-distance",5);
      plot->setPropertyValue("labels.x-precision",1);
      plot->setPropertyValue("labels.y-precision",0);
      plot->label("orig. function");
      plot->series(fData);

      plot->sym('x',2);
      plot->color(0,100,255);
      plot->label("samples");
      plot->scatter(fDataSamples);

      plot->nosym();
      
      plot->color(0,255,0);
      plot->fill(0,255,0,100);
      plot->label("regression result");
      plot->series(fApprox);
    }
    if(i==11){
      plot->setPropertyValue("enable fill",true);
      plot->setDataViewPort(Range32f(0,2*M_PI),Range32f(-1,1));
      plot->nocolor();
      plot->label("sin");
      plot->fill(255,0,0);
      plot->bars(sinData.data(), sinData.size()/5,5);

      plot->label("cos");
      plot->fill(0,255,0);
      plot->bars(cosData.data(), cosData.size()/5,5);

      plot->label("tan");
      plot->fill(0,50,255);
      plot->bars(tanData.data(), tanData.size()/5,5);

    }
    plot->unlock();
    plot.render();
  }

  static FPSLimiter fpsLimit(20,10);
  fpsLimit.wait();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-use-opengl|-gl",init,run).exec();
}
