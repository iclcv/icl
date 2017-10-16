/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryOpWork.h                  **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/MultiThreader.h>
#include <ICLFilter/UnaryOp.h>

namespace icl{
  namespace filter{

    /// Internally used Plugin class for multithreaded unary operations
    struct ICLFilter_API UnaryOpWork : public utils::MultiThreader::Work{
      /// Construktor
      UnaryOpWork(UnaryOp *op, const core::ImgBase *src, core::ImgBase *dst):
        op(op),src(src),dst(dst){}

      /// Destructor
      virtual ~UnaryOpWork(){}

      /// working function
      virtual void perform(){
        op->apply(src,&dst);
      }
      private:
      /// Wrapped op
      UnaryOp *op;

      /// Wrapped src image
      const core::ImgBase *src;

      /// Wrapped dst image
      core::ImgBase *dst;
    };

  } // namespace filter
}
