/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
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
