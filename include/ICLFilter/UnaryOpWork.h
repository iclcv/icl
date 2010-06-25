/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/UnaryOpWork.h                        **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_UNARY_OP_WORK_H
#define ICL_UNARY_OP_WORK_H

#include <ICLUtils/MultiThreader.h>
#include <ICLFilter/UnaryOp.h>

namespace icl{

  /// Internally used Plugin class for multithreaded unary operations
  struct UnaryOpWork : public MultiThreader::Work{
    /// Construktor
    UnaryOpWork(UnaryOp *op, const ImgBase *src, ImgBase *dst):
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
    const ImgBase *src;
    
    /// Wrapped dst image
    ImgBase *dst;
  };

}
#endif
