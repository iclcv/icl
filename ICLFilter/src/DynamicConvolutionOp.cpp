/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/DynamicConvolutionOp.cpp                 **
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

#include <ICLFilter/DynamicConvolutionOp.h>
#include <ICLCore/Img.h>

namespace icl{

  DynamicConvolutionOp::DynamicConvolutionOp (const ImgBase* poKernel) : 
     ConvolutionOp ()
  {
     poKernelBuf = new icl::Img<icl32f>(Size(3,3), 1);
     if (poKernel) setKernel (poKernel);
  }
  
  DynamicConvolutionOp::~DynamicConvolutionOp () {
     delete poKernelBuf;
  }

  void DynamicConvolutionOp::setKernel (const ImgBase* poKernel) {
     ICLASSERT_RETURN(poKernel->getChannels() == 1);

     // resize kernel buffer if necessary
     if (poKernel->getROISize() != poKernelBuf->getSize())
        poKernelBuf->setSize (poKernel->getROISize());

     // copy data from poKernel's ROI to poKernelBuf
     poKernel->convertROI(poKernelBuf);

     // set ConvolutionOp kernel from float data
     ConvolutionOp::setKernel (ConvolutionKernel(poKernelBuf->getData(0), poKernelBuf->getSize(), false));
  }

 }
