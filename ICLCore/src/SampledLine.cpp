/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCore module of ICL                         **
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

#include <ICLCore/SampledLine.h>
#include <ICLCore/LineSampler.h>
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
