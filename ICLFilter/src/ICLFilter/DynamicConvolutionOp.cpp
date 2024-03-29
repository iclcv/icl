/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/DynamicConvolutionOp.cpp       **
** Module : ICLFilter                                              **
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

#include <ICLFilter/DynamicConvolutionOp.h>
#include <ICLCore/Img.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

    DynamicConvolutionOp::DynamicConvolutionOp (const ImgBase* poKernel) :
       ConvolutionOp ()
    {
       poKernelBuf = new Img<icl32f>(Size(3,3), 1);
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
}
