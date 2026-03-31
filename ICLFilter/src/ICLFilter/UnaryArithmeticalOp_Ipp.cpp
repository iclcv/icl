/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryArithmeticalOp_Ipp.cpp **
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

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLFilter/UnaryArithmeticalOp.h>

// IPP backends to be added when building on a platform with IPP
// Registration pattern:
//   using UAOp = icl::filter::UnaryArithmeticalOp;
//   using Op = UAOp::Op;
//   auto ipp = UAOp::prototype().backends(Backend::Ipp);
//   ipp.add<UAOp::ArithValSig>(Op::withVal, ipp_fn, applicableTo<...>, "...");
//   ipp.add<UAOp::ArithNoValSig>(Op::noVal, ipp_fn, applicableTo<...>, "...");
