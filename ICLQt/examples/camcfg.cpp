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

using namespace icl;
using namespace std;

int main(int n, char **ppc){
  paex
  ("r","resets the dc bus on startup")
  ("8","scan for dc800 devices")
  ("u","scan for unicap devices")
  ("d","scan for dc devices")
  ("s","scan for SwissRanger devices")
  ("c","scan for OpenCV-based devices ")
  ("-demo","add a DemoGrabber device")
  ("-i","ICL's default device specification");
  painit(n,ppc,"-dc|d -dc800|8 -demo -unicap|u -pwc|p -sr|s -cvcam|c"
         " -reset-bus|-r|r -input|-i(device-type,device-ID)");
  QApplication a(n,ppc);
  
  std::ostringstream str;
  if(pa("d")) str << ",dc";
  if(pa("8")) str << ",dc800";
  if(pa("-demo")) str << ",demo";
  if(pa("u")) str << ",unicap";
  if(pa("p"))str << ",pwc";
  if(pa("c"))str << ",cvcam";
  if(pa("-i")) str << "," << pa("-i",0) << "=" << pa("-i",1);
  
  std::string devlist = str.str();
  if(!devlist.length()){
    pausage("no devices selected!");
    exit(-1);
  }
  devlist = devlist.substr(1); // removes the trailing comma!
  
  if(pa("r")) GenericGrabber::resetBus(devlist);
  
  CamCfgWidget w(devlist,0);
  w.setGeometry(50,50,700,700);
  w.setWindowTitle("icl-camcfg (ICL' Camera Configuration Tool)");
  w.show();
  return a.exec();
}
