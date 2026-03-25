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

#include <ICLFilter/BinaryOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/ImgBase.h>

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

    // Default Image apply — delegates to legacy ImgBase** for backward compat
    void BinaryOp::apply(const core::Image &src1, const core::Image &src2, core::Image &dst){
      core::ImgBase *tmp = nullptr;
      apply(src1.ptr(), src2.ptr(), &tmp);
      if(tmp) dst = core::Image(tmp);
    }

    core::Image BinaryOp::apply(const core::ImgBase *a, const core::ImgBase *b){
      apply(a, b, &m_buf);
      return core::Image(m_buf->deepCopy());
    }

  } // namespace filter
}
