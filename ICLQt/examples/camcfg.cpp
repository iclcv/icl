/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/camcfg.cpp                              **
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

#include <ICLQt/CamCfgWidget.h>
#include <QtGui/QApplication>
#include <ICLUtils/ProgArg.h>
#include <ICLIO/GenericGrabber.h>

#include <sstream>

using namespace icl;
using namespace std;

int main(int n, char **ppc){
  
  paex
  ("r","resets the dc bus on startup")
  ("8","scan for dc800 devices")
  ("u","scan for unicap devices")
  ("d","scan for dc devices")
  ("s","scan for SwissRanger devices")
  ("m","scan for Shared Memory devices ")
  ("kc","scan for Kinect Devices (Depth Image)")
  ("kd","scan for Kinect Devices (Color Image)")
  ("y","scan for Myrmex Devices ")
  ("c","scan for OpenCV-based devices ")
  ("-demo","add a DemoGrabber device")
  ("-i","ICL's default device specification")
  ("-l","if this flag is passed, no GUI is created, "
   "but all available devices are listed on stdout");
  painit(n,ppc,"-dc|d -dc800|8 -demo -unicap|u -mry|y -pwc|p -sr|s -cvcam|c -sm|m"
         " -reset-bus|-r|r -kinect-depth|kd -kinect-color|kc "
         "-input|-i(device-type,device-ID) -list-devices-only|-l");
  QApplication a(n,ppc);
  
  std::ostringstream str;
  if(pa("d")) str << ",dc";
  if(pa("8")) str << ",dc800";
  if(pa("-demo")) str << ",demo";
  if(pa("u")) str << ",unicap";
  if(pa("p"))str << ",pwc";
  if(pa("c"))str << ",cvcam";
  if(pa("s"))str << ",sr";
  if(pa("kd"))str << ",kinectd";
  if(pa("kc"))str << ",kinectc";
  if(pa("m"))str << ",sm";
  if(pa("y"))str << ",myr";
  if(pa("-i")) str << "," << pa("-i",0) << "=" << pa("-i",1);
  
  std::string devlist = str.str();
  if(!devlist.length()){
    pausage("no devices selected!");
    exit(-1);
  }
  devlist = devlist.substr(1); // removes the trailing comma!
  
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
          for(int k=0;k<lineStr.size();++k) {
            std::cout << " ";
          }
        }
        if(idAliases[i].find(' ') != std::string::npos){
          std::cout << "'" << idAliases[j] << "'" << std::endl;
        }else{
          std::cout << idAliases[j] <<  std::endl;
        }
      }
      std::cout << " " << (i<10?" ":"") << " description: " << gds[i].description << "\n\n";
    }
  }else{
    CamCfgWidget w(devlist,0);
    w.setGeometry(50,50,700,700);
    w.setWindowTitle("icl-camcfg (ICL' Camera Configuration Tool)");
    w.show();
    return a.exec();
  }
}
