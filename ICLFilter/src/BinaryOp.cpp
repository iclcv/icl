/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/BinaryOp.cpp                             **
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
*********************************************************************/

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
