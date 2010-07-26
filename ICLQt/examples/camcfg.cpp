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

using namespace icl;
using namespace std;

int main(int n, char **ppc){
  paex
  ("-r","resets the dc bus on startup")
  ("-800","if this flag is set, application trys dc devices to setup\n"
   "in ieee1394-B mode with 800MBit iso transfer rate")
  ("-no-unicap","disable unicap support")
  ("-no-dc","disable dc grabber support")
  ("-no-pwc","disable pwc grabber support");
  
  painit(n,ppc,"-use-IEEE1394-B|-800 -reset-bus|-r -no-unicap|-u -no-dc|-d -no-pwc|-p -no-sr|-s -no-opencv|-o");
  QApplication a(n,ppc);
  
  CamCfgWidget::CreationFlags flags(pa("-800")?800:400,
                                    pa("-r"),
                                    pa("-no-unicap"),
                                    pa("-no-dc"),
                                    pa("-no-pwc"),
                                    pa("-no-sr"),
                                    pa("-no-opencv"));
  
  
  CamCfgWidget w(flags);

  w.setGeometry(50,50,800,800);
  w.setWindowTitle("camcfg (ICL Camera Configuration Tool)");
  w.show();

  
  return a.exec();
}
