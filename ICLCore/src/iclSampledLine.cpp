#include <iclSampledLine.h>
#include <iclLineSampler.h>
namespace icl{

  static int estimate_buf_size(int aX, int aY, int bX, int bY){
    return iclMax(abs(aX-bX),abs(aY-bY))+1; //+1 even though a==b -> we have a sigle point!
  }

  void SampledLine::init(int aX, int aY, int bX, int bY){
    int n = estimate_buf_size(aX,aY,bX,bY);
    if(n>0){
      m_bufBegin = new Point[n];
      m_bufEnd = m_bufBegin+n;
      LineSampler::init(aX,aY,bX,bY,m_bufBegin,n);
      LineSampler::getPointers(&m_cur,&m_end);
    }else{
      m_bufBegin = m_bufEnd = 0;
    }
  }
  void SampledLine::init(int aX, int aY, int bX, int bY, int minX, int minY, int maxX, int maxY){
    int n = estimate_buf_size(aX,aY,bX,bY);
    if(n){
      m_bufBegin = new Point[n];
      m_bufEnd = m_bufBegin+n;
      LineSampler::init(aX,aY,bX,bY,minX,minY,maxX,maxY,m_bufBegin,n);
      LineSampler::getPointers(&m_cur,&m_end);
    }else{
      m_bufBegin = m_bufEnd = 0;
    }

  }

  SampledLine &SampledLine::operator=(const SampledLine &other){
    if(other.isNull()){
      ICL_DELETE_ARRAY(m_bufBegin);
      m_cur = m_end = 0;
    }else{
      if(other.getBufferSize() != getBufferSize()){
        ICL_DELETE_ARRAY(m_bufBegin);
        int n = other.getBufferSize();
        m_bufBegin = new Point[n];
        m_bufEnd = m_bufBegin+n;
      }
      m_cur = m_bufBegin + other.getBufferOffset();
      m_end = m_cur + other.remaining();
      std::copy(other.m_bufBegin,other.m_bufEnd,m_bufBegin);
    }
    return *this;
  }
}
