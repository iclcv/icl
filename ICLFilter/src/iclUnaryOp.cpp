#include <iclUnaryOp.h>
#include <iclUnaryOpWork.h>
#include <iclMacros.h>
#include <iclImageSplitter.h>

namespace icl{

  UnaryOp::UnaryOp():m_poMT(0){};
  
  UnaryOp::UnaryOp(const UnaryOp &other):m_poMT(0),m_oROIHandler(other.m_oROIHandler){}
  
  UnaryOp &UnaryOp::operator=(const UnaryOp &other){
    m_oROIHandler = other.m_oROIHandler;
    ICL_DELETE( m_poMT );
    return *this;
  }
  UnaryOp::~UnaryOp(){
    ICL_DELETE( m_poMT );
  }
  
  void UnaryOp::applyMT(const ImgBase *poSrc, ImgBase **ppoDst, unsigned int nThreads){
    ICLASSERT_RETURN( nThreads > 0 );
    ICLASSERT_RETURN( poSrc );
    if(nThreads == 1){
      apply(poSrc,ppoDst);
      return;
    }

    if(!prepare (ppoDst, poSrc)) return;
  
    bool ctr = getClipToROI();
    bool co = getCheckOnly();
    
    setClipToROI(false);
    setCheckOnly(true);
    
    const std::vector<ImgBase*> srcs = ImageSplitter::split(poSrc,nThreads);
    std::vector<ImgBase*> dsts = ImageSplitter::split(*ppoDst,nThreads);
    
    MultiThreader::WorkSet works(nThreads);
    
    for(unsigned int i=0;i<nThreads;i++){
      works[i] = new UnaryOpWork(this,srcs[i],const_cast<ImgBase*>(dsts[i]));
    }
    
    if(!m_poMT){
      m_poMT = new MultiThreader(nThreads);
    }else{
      if(m_poMT->getNumThreads() != (int)nThreads){
        delete m_poMT;
        m_poMT = new MultiThreader(nThreads);
      }
    }
    
    (*m_poMT)( works );
    
    for(unsigned int i=0;i<nThreads;i++){
      delete works[i];
    }

    setClipToROI(ctr);
    setCheckOnly(co);

    ImageSplitter::release(srcs);
    ImageSplitter::release(dsts);
  }

}
