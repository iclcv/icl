/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/apps/image-compare/image-compare.cpp             **
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

#include <ICLQt/Common.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLFilter/BinaryCompareOp.h>

int main(int n, char **ppc){
  pa_init(n,ppc,"[m]-input-1|-1|-a(device,device-params) "
          "[m]-input-2|-2|-b(device,device-params) "
          "-seclect-channel-a|-ca(int) "
          "-select-channel-b|-cb(int)");

  GenericGrabber ga;
  GenericGrabber gb;
  ga.init(pa("-a"));
  gb.init(pa("-b"));

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
  HBox gui;
  gui << Image().handle("sub").label("A-B")
      << Image().handle("eq").label("A==B")
      << Show();

  gui["sub"] = subImage;
  gui["eq"] = cmpImage;

  return app.exec();

}
