/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLQuick/Common.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLFilter/BinaryCompareOp.h>

int main(int n, char **ppc){
  painit(n,ppc,"[m]-input-1|-1|-a(device,device-params) "
         "[m]-input-2|-2|-b(device,device-params) "
         "-seclect-channel-a|-ca(int) "
         "-select-channel-b|-cb(int)");
  
  GenericGrabber ga(FROM_PROGARG("-a"));
  GenericGrabber gb(FROM_PROGARG("-b"));

  ga.setIgnoreDesiredParams(true);
  gb.setIgnoreDesiredParams(true);

  const ImgBase *a = ga.grab();
  const ImgBase *b = gb.grab();
  
  if(pa("-ca")){
    a = a->selectChannel(pa("-ca"));
  }
  if(pa("-cb")){
    b = b->selectChannel(pa("-cb"));
  }
  
  bool canBeCompared = true;
  
#define CHECK(X,MANDATORY)                                              \
  if(a->get##X() != b->get##X()){                                       \
    std::cout << "Images differ in " << #X << ":   A:" << a->get##X() << "    B:" << b->get##X() << std::endl; \
    if(MANDATORY) canBeCompared = false;                                \
  }else{                                                                \
    std::cout << #X << " is equal:             " << a->get##X() << std::endl; \
  }

  CHECK(Size,true);
  CHECK(Depth,true);
  CHECK(Channels,true);
  CHECK(Format,false);
  CHECK(ROI,false);
#undef CHECK

  if(!canBeCompared){
    std::cout << "images cannot be compared pixel-wise due to incompatibilities" << std::endl;
    return 0;
  }

  ImgBase *subImage = 0, *cmpImage=0;
  BinaryArithmeticalOp(BinaryArithmeticalOp::subOp).apply(a,b,&subImage);
  BinaryCompareOp(BinaryCompareOp::eq).apply(a,b,&cmpImage);


  QApplication app(n,ppc);
  GUI gui("hbox");
  gui << "image[@handle=sub@label='A-B']" << "image[@handle=eq@label='A==B']";
  gui.show();
  
  gui["sub"] = subImage;
  gui["eq"] = cmpImage;

  return app.exec();

}
