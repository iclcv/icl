/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryOp.cpp                   **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLFilter/BinaryOp.h>

namespace icl{
  namespace filter{
  
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
  
    const core::ImgBase *BinaryOp::apply(const core::ImgBase *a, const core::ImgBase *b){
      apply(a,b,&m_buf);
      return m_buf;
    }
  } // namespace filter
}
