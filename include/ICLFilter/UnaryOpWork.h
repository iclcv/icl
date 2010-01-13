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
