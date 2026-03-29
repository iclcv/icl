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

    bool BinaryOp::prepare(core::Image &dst, const core::Image &src) {
      core::ImgBase *tmp = dst.isNull() ? nullptr : dst.ptr();
      bool r = m_oROIHandler.prepare(&tmp, src.ptr());
      if(r && tmp) dst = core::Image(*tmp);
      return r;
    }

    bool BinaryOp::prepare(core::Image &dst, const core::Image &src, core::depth d) {
      core::ImgBase *tmp = dst.isNull() ? nullptr : dst.ptr();
      bool r = m_oROIHandler.prepare(&tmp, src.ptr(), d);
      if(r && tmp) dst = core::Image(*tmp);
      return r;
    }

    bool BinaryOp::prepare(core::Image &dst, core::depth d, const utils::Size &s,
                           core::format fmt, int ch, const utils::Rect &roi,
                           utils::Time t) {
      core::ImgBase *tmp = dst.isNull() ? nullptr : dst.ptr();
      bool r = m_oROIHandler.prepare(&tmp, d, s, fmt, ch, roi, t);
      if(r && tmp) dst = core::Image(*tmp);
      return r;
    }

    // Legacy ImgBase** apply — final bridge delegating to Image-based apply
    void BinaryOp::apply(const core::ImgBase *src1, const core::ImgBase *src2, core::ImgBase **dst){
      ICLASSERT_RETURN(src1);
      ICLASSERT_RETURN(src2);
      ICLASSERT_RETURN(dst);
      core::Image s1(*src1), s2(*src2);
      core::Image d;
      if(*dst) d = core::Image(**dst);
      apply(s1, s2, d);
      if(!d.isNull()){
        if(!*dst || (*dst)->getDepth() != d.getDepth()){
          ICL_DELETE(*dst);
          *dst = d.ptr()->shallowCopy();
        } else {
          d.ptr()->deepCopy(dst);
        }
      }
    }

    core::Image BinaryOp::apply(const core::ImgBase *a, const core::ImgBase *b){
      core::Image dst;
      apply(core::Image(*a), core::Image(*b), dst);
      return dst;
    }

  } // namespace filter
}
