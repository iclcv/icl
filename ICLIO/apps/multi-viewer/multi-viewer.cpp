/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/apps/multi-viewer/multi-viewer.cpp               **
** Module : ICLIO                                                  **
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

VSplit gui;

struct Input{
  std::string a,b;
  GenericGrabber grabber;
  ImageHandle handle;
  std::string id;
  void operator()(){
    handle = grabber.grab();
  }
};

int nInputs = -1;
Array2D<Input> inputs;
bool asyncMode = false;

void init(){
  if(pa("-r")) GenericGrabber::resetBus("dc");

  ProgArg p = pa("-i");
  Size layout = pa("-l") ? pa("-l").as<Size>() : Size(ceil(nInputs*0.5),2);

  ICLASSERT_THROW(layout.getDim() >= nInputs, 
                   ICLException("given layout has less cells than the given number of inputs"));

  std::vector<HSplit> rows(layout.height);

  inputs  = Array2D<Input>(layout);
  
  for(int i=0;i<nInputs;++i){
    Input &in = inputs[i];

    in.id = str(i);
    in.a = p[2*i];
    in.b = p[2*i+1];

    in.grabber.init(in.a,in.a+"="+in.b);
    
    if(pa("-s")) in.grabber.useDesired(pa("-s").as<Size>());
    if(pa("-f")) in.grabber.useDesired(pa("-f").as<format>());
    if(pa("-d")) in.grabber.useDesired(pa("-d").as<depth>());
    
    rows[i/layout.width] << Image().label(in.id + ": "+in.a+" "+in.b).handle(in.id);
  }

  for(int i=0;i<layout.height;++i){
    gui << rows[i];
  }
  gui << ( HBox().minSize(0,2).maxSize(99,2) 
           << CamCfg() 
           << Fps(10).handle("fps")
         ) 
      <<   Show();

  for(int i=0;i<nInputs;++i){
    Input &in = inputs[i];
    try{
      in.handle = gui.get<ImageHandle>(in.id);
    }catch(...){};
  }
}

template<int N>
void run(){
  if(!N){
    gui["fps"].render();
    if(!asyncMode){
      for(int i=0;i<nInputs;++i){
        inputs[i]();
      }
    }
  }
  if(N || asyncMode){
    inputs[N]();
  }
}


int main(int n, char **ppc){
  pa_explain
  ("-size","grabbing size")
  ("-i","list of inputs, two tokens per input, e.g. -i dc 0 dc 1 dc 2 file '*.jpg'");

  pa_init(n,ppc,
          "-size|-s(size) "
          "-format|-f(format) "
          "-depth|-d(depth) "
          "-reset-bus|-r "
          "-layout|-l(size) " 
          "-asynchronous|-a "
          "-i(...)");
  ICLApp app(n,ppc,"",init);

  ProgArg p = pa("-i");
  nInputs = p.n()/2;

  if(pa("-a")){
    asyncMode = true;
    if(nInputs > 8) throw ICLException("asynchronous mode is only supported for up to 8 inputs!");
    if(nInputs > 0) app.addThread(run<0>);
    if(nInputs > 1) app.addThread(run<1>);
    if(nInputs > 2) app.addThread(run<2>);
    if(nInputs > 3) app.addThread(run<3>);
    if(nInputs > 4) app.addThread(run<4>);
    if(nInputs > 5) app.addThread(run<5>);
    if(nInputs > 6) app.addThread(run<6>);
    if(nInputs > 7) app.addThread(run<7>);
  }else{
    app.addThread(run<0>);
  }
  return app.exec();
}
