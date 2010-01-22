#include <ICLFilter/BinaryOp.h>

namespace icl{

  BinaryOp::BinaryOp():m_buf(0){
  
  }
    
  BinaryOp::BinaryOp(const BinaryOp &other):
    m_oROIHandler(other.m_oROIHandler),m_buf(0){
  
  }
  
  BinaryOp &BinaryOp::operator=(const BinaryOp &other){
    m_oROIHandler = other.m_oROIHandler;
    return *this;
  }
  
  BinaryOp::~BinaryOp(){
    ICL_DELETE(m_buf);
  }

  const ImgBase *BinaryOp::apply(const ImgBase *a, const ImgBase *b){
    apply(a,b,&m_buf);
    return m_buf;
  }
}
