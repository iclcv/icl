/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/apps/camera-config/camera-config.cpp             **
** Module : ICLQt                                                  **
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

#include <ICLQt/CamCfgWidget.h>
#include <ICLQt/GUI.h>
#include <ICLQt/Common.h>
#include <ICLQt/Application.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Mutex.h>
#include <ICLIO/GenericGrabber.h>

#include <sstream>

using namespace icl::utils;
using namespace icl::io;
using namespace icl::qt;
using namespace std;

std::string devlist;
Mutex mux;
CamCfgWidget* inst = NULL;

void init(){
  inst -> setGeometry(50,50,700,700);
  inst -> setWindowTitle("icl-camcfg (ICL' Camera Configuration Tool)");
  inst -> show();
}

void run(){
  Mutex::Locker l(mux);
  if(inst) inst->update();
}

int main(int n, char **ppc){

  pa_explain
  ("r","resets the dc bus on startup")
  ("8","scan for dc800 devices")
  ("u","scan for unicap devices")
  ("d","scan for dc devices")
  ("s","scan for SwissRanger devices")
  ("v","scan for V4L devices")
  ("m","scan for Shared Memory devices")
  ("k","scan for Kinect devices")
  ("y","scan for Myrmex devices")
  ("c","scan for OpenCV-based devices")
  ("b","scan for Basler-Pylon-based devices")
  ("a","scan for all devices (experimental)")
  ("q","scan for Qt-Multimedia devices")
  ("-demo","add a DemoGrabber device")
  ("-i","ICL's default device specification")
  ("-l","if this flag is passed, no GUI is created, "
   "but all available devices are listed on stdout");
  pa_init(n,ppc,"-dc|d -dc800|8 -demo -unicap|u -mry|y -optris|o -xi|x -pwc|p -sr|s -cvcam|c -sm|m -v4l|v -all|a "
          " -reset-bus|-r|r -kinect|k -pylon|b -qt|q "
          "-input|-i(device-type,device-ID) -list-devices-only|-l");


  std::ostringstream str;
  if(pa("d")) str << ",dc";
  if(pa("8")) str << ",dc800";
  if(pa("-demo")) str << ",demo";
  if(pa("u")) str << ",unicap";
  if(pa("p"))str << ",pwc";
  if(pa("c"))str << ",cvcam";
  if(pa("s"))str << ",sr";
  if(pa("k"))str << ",kinectd,kinectc,kinecti";
  if(pa("m"))str << ",sm";
  if(pa("v"))str << ",v4l";
  if(pa("o"))str << ",optris";
  if(pa("x"))str << ",xi";
  if(pa("y"))str << ",myr";
  if(pa("q"))str << ",qtcam";
  if(pa("b"))str << ",pylon";
  if(pa("-i")) str << "," << pa("-i",0) << "=" << pa("-i",1);
  if(pa("a") || pa("-all")) str.flush();

  devlist = str.str();
  if(!devlist.length()){
    //pa_show_usage("no devices selected!");
    //exit(-1);
  } else {
    devlist = devlist.substr(1); // removes the trailing comma!
  }

  if(pa("r")) GenericGrabber::resetBus(devlist);

  if(pa("-l")){
    std::vector<GrabberDeviceDescription> gds = GenericGrabber::getDeviceList(devlist);
    std::string lastType = "";
    for(unsigned int i=0;i<gds.size();++i){
      if(gds[i].type != lastType){
        lastType = gds[i].type;
        std::cout << "\n" << lastType << " devices:" << std::endl;
      }
      std::ostringstream line;
      line << i << (i<10?" ":"") << "  ID aliases: ";
      std::string lineStr = line.str();
      std::cout << lineStr;
      std::vector<std::string> idAliases = tok(gds[i].id,"|||",false);
      for(unsigned int j=0;j<idAliases.size();++j){
        if(j){
          for(unsigned int k=0;k<lineStr.size();++k) {
            std::cout << " ";
          }
        }
        if(idAliases[j].find(' ') != std::string::npos){
          std::cout << "'" << idAliases[j] << "'" << std::endl;
        }else{
          std::cout << idAliases[j] <<  std::endl;
        }
      }
      std::cout << " " << (i<10?" ":"") << " description: " << gds[i].description << "\n\n";
    }
  }else{
    ICLApp app(n,ppc,"",init,run);
    inst = new CamCfgWidget(devlist,0);
    int ret = app.exec();
    mux.lock();
    ICL_DELETE(inst);
    mux.unlock();
    return ret;
  }
}
