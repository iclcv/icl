/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/examples/inplace-op-test.cpp                 **
** Module : ICLFilter                                              **
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
*********************************************************************/

#include <ICLQuick/Quick.h>
#include <ICLFilter/InplaceArithmeticalOp.h>
#include <ICLFilter/InplaceLogicalOp.h>

int main(){
  ImgQ a = scale(create("parrot"),0.5);
  ImgQ addRes = copy(a);
  ImgQ notRes = copy(a);
  Img8u binNotRes = cvt8u(copy(a));

  InplaceArithmeticalOp(InplaceArithmeticalOp::addOp,100).apply(&addRes);
  InplaceLogicalOp(InplaceLogicalOp::notOp).apply(&notRes);
  InplaceLogicalOp(InplaceLogicalOp::binNotOp).apply(&binNotRes);
  
  
  
  show( ( label(a,"original"),
          label(addRes,"add(100)"),
          label(notRes,"logical not (!)"),
          label(cvt(binNotRes),"binary not (~)")
      ) );
  
  return 0;  
};
