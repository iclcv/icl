/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/kinect-recorder/kinect-recorder.cpp       **
** Module : ICLGeom                                                **
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

HBox gui;
GenericGrabber c_in;
GenericGrabber d_in;
GenericImageOutput c_out,d_out,both_out;


void init(){
  if(pa("-s")){

    if(pa("-c")){
      c_in.init(pa("-c"));
    }else{
      c_in.init("kinectc","kinectc=0");
    }

    if(pa("-d")){
      d_in.init(pa("-d"));
    }else{
      d_in.init("kinectd","kinectd=0");
    }
    c_out.init("file",*pa("-s")+"/color-######.bicl");
    d_out.init("file",*pa("-s")+"/depth-######.bicl");

    gui << Image().handle("depth").minSize(16,12).label("depth image")
        << Image().handle("color").minSize(16,12).label("color image");
  }else{
    if(!pa("-d") && !pa("-c")) throw ICLException("no input given");
    if(pa("-d")){
      d_in.init(pa("-d"));

      gui << Image().handle("depth").minSize(16,12).label("depth image");
    }else if(pa("-do")) throw ICLException("depth output given, but no input!");
    if(pa("-c")){
      c_in.init(pa("-c"));
      gui << Image().handle("color").minSize(16,12).label("color image");
    }else if(pa("-co")) throw ICLException("color output given, but no input!");

    if(pa("-do")) d_out.init(pa("-do"));
    if(pa("-co")) c_out.init(pa("-co"));
  }
  gui << CamCfg("");

  if(pa("-man")){
    if(!pa("-co") && !pa("-do")){
      throw std::logic_error("manual mode '-man' without given "
                             "either given depth and/or given "
                             "color output does not make sense!");
    }
    if(pa("-drop").as<int>()){
      throw std::logic_error("naumal mode '-man' and inital drop "
                             "frames '-drop' cannot be combined");
    }
    gui << (VBox()
            << Button("start","stop",false).handle("man").label("manual recording")
            .tooltip("icl-kinect-recorder was started in 'manual' mode. Here"
                     "data is only recorded if this button is pressed")
            << CheckBox("re-init at start",false).handle("reinit")
            .tooltip("if this is checked, each time, the start/stop button is"
                     "toggled, both image outputs are reinitialized. In this"
                     "case, already recorded frame might be overwritten")
            << Label("0").handle("nRecorded").label("num recorded frames")
            );
  }

  if(pa("-ds")){
    Size s = pa("-ds");
    if(!d_in.isNull()) d_in.useDesired(s);
  }

  if(pa("-cs")){
    Size s = pa("-cs");
    if(!c_in.isNull()) c_in.useDesired(s);
  }

  gui << Show();
}


void run(){
  static int nDrop = pa("-drop");
  if(nDrop < 0){
    throw std::logic_error("num drop '-drop' frames must be >= 0");
  }

  const ImgBase *c=0,*d=0;
  if(!c_in.isNull()) c = c_in.grab();
  if(!d_in.isNull()) d = d_in.grab();

  if(nDrop){
    --nDrop;
  }else{
    if(pa("-man")){
      static bool lastMan = false;
      static int n = 0;
      bool man = gui["man"];
      if(man){
        if(lastMan != man){
          if(gui["reinit"]){
            if(pa("-do")) d_out.init(pa("-do"));
            if(pa("-co")) c_out.init(pa("-co"));
            n = 0;
          }
        }
        if(!c_out.isNull() && c) c_out.send(c);
        if(!d_out.isNull() && d) d_out.send(d);
        ++n;
        gui["nRecorded"] = n;
      }
      lastMan = man;

    }else{
      if(!c_out.isNull() && c) c_out.send(c);
      if(!d_out.isNull() && d) d_out.send(d);
    }
  }

  if(c) gui["color"] = c;
  if(d) gui["depth"] = d;
}

int main(int n, char **args){
  pa_explain("-s","deines a single output directory, where captured images\n"
             "are stored. If no input (-d) or (-c) is given, by default,\n"
             "the kinect color and depth camera are used");

  return ICLApp(n,args,"-depth-input|-d(2) -color-input|-c(2) "
                "-depth-size|-ds(size) -color-size|-cs(size) "
                "-depth-output|-do(2) -color-output|-co(2) "
                "-simple-io-params|-s(output-file-base-name) "
                "-drop-num-first-frames|-drop(n=0) "
                "-manually-trigger-start-and-stop|-man"
                ,init,run).exec();
}




