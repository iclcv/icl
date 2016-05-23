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
#include <ICLIO/GenericImageOutput.h>

VSplit gui;

struct Input{
  std::string a,b;
  GenericGrabber grabber;
  ImageHandle handle;
  const ImgBase *lastImage;
  std::string id;
  void operator()(){
    lastImage = grabber.grab();
    handle = lastImage;
  }
  void save(){
    if(out) out->send(lastImage);
  }
  SmartPtr<GenericImageOutput> out;
};

int nInputs = -1;
Array2D<Input> inputs;
bool asyncMode = false;

void save_all(){
  std::string fn;
  bool ok = false;
  struct Evt : public ICLApp::AsynchronousEvent{
    std::string &fn;
    bool &ok;

    Evt(std::string &fn, bool &ok):fn(fn),ok(ok){}
    void execute(){
      try{
        static std::string lastDir;
        fn = saveFileDialog("","save files (# is replaced by the input-ID)",lastDir);
        if(fn.length()){
          lastDir = File(fn).getDir();
        }
      }catch(...){ }
      ok = true;
    }
  };
  ICLApp::instance()->executeInGUIThread(new Evt(fn,ok));
  while(!ok) Thread::msleep(10);
  if(fn.length()){
    MatchResult r = match(fn,"([^#]*)#([^#]*)",3);
    if(r.matched && r.submatches.size()>2){
      std::string pref  = r.submatches[1];
      std::string suff  = r.submatches[2];
      for(int i=0;i<nInputs;++i){
        Input &in = inputs[i];
        std::string part = in.a+"-"+in.b;
        for(size_t i=0;i<part.length();++i){
          char &c = part[i];
          if(c == '/') c = '-';
        }
        std::string fn = pref+"-"+part+suff;
        save(*in.lastImage, fn);
        std::cout << "saved file " << fn << std::endl;
      }
      std::cout << std::endl;
    }else{
      ERROR_LOG("the given filename must have a '#'-token, which can be replaced by the input ID");
    }
  }  
}

std::string fix_at_stuff(const std::string &s){
  std::vector<std::string> ts = tok(s,"@");
  if(!ts.size()) return "";
  return ts[0];
}

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
    
    rows[i/layout.width] << Image().label(in.id + ": "+in.a+" "+fix_at_stuff(in.b)).handle(in.id);
    
    if(pa("-o")){
      ProgArg o = pa("-o");

      if((int)(2*i+1) >= o.n()){
        throw ICLException("less outputs were given to -o "
                           "then inputs were given to -i");
      }
      in.out = new GenericImageOutput(o[2*i], o[2*i+1]);
    }
  }

  for(int i=0;i<layout.height;++i){
    gui << rows[i];
  }
  GUI camcfg;
  if(pa("-sync")){
    for(int i=1;i<nInputs;++i){
      if(inputs[i].a != inputs[0].a) {
        throw ICLException("option -s to synchronize all grabbers can "
                           "only be used, if all input types are identical");
      }
      inputs[0].grabber.syncChangesTo(&inputs[i].grabber);
    }
    camcfg << CamCfg();//inputs[0].a+","+inputs[0].b);
  }else{
    camcfg << CamCfg();
  }

  
  gui << ( HBox().minSize(0,2).maxSize(99,2) 
           << Button("stopped","running",true).handle("on")
           << camcfg
           << Fps(10).handle("fps")
           << Button("save").handle("save")
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
  while(!gui["on"].as<bool>()){
    Thread::msleep(10);
    if(!N && !asyncMode){
      static ButtonHandle save = gui["save"];
      if(save.wasTriggered()){
        save_all();
      }
    }
  }
  if(!N){
    gui["fps"].render();
    if(!asyncMode){
      static ButtonHandle save = gui["save"];
      
      for(int i=0;i<nInputs;++i){
        inputs[i]();
      }

      for(int i=0;i<nInputs;++i){
        inputs[i].save();
      }
      
      if(save.wasTriggered()){
        save_all();
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
  ("-o","automatically output each input image (output correspond to the inputs)")
  ("-i","list of inputs, two tokens per input, e.g. -i dc 0 dc 1 dc 2 file '*.jpg'");

  pa_init(n,ppc,
          "-size|-s(size) "
          "-format|-f(format) "
          "-depth|-d(depth) "
          "-reset-bus|-r "
          "-layout|-l(size) " 
          "-asynchronous|-a "
          "-output|-o(...) "
          "-sync-all-grabbers|-sync "
          "-i(...)");
  ICLApp app(n,ppc,"",init);

  ProgArg p = pa("-i");
  nInputs = p.n()/2;

  if(pa("-a")){
    if(pa("-o")) throw ICLException("output is not supported in async mode");
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
