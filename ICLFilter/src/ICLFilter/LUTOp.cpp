/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/LUTOp.cpp                      **
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

#include <ICLFilter/LUTOp.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

    const char* toString(LUTOp::Op op) {
      switch(op) {
        case LUTOp::Op::reduceBits: return "reduceBits";
      }
      return "?";
    }

    core::ImageBackendDispatching& LUTOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<ReduceBitsSig>(Op::reduceBits);
        return true;
      }();
      (void)init;
      return proto;
    }

    LUTOp::LUTOp(icl8u quantizationLevels):
      ImageBackendDispatching(prototype()),
      m_bLevelsSet(true), m_bLutSet(false),
      m_ucQuantizationLevels(quantizationLevels),
      m_poBuffer(new Img8u())
    {}

    LUTOp::LUTOp(const std::vector<icl8u> &lut):
      ImageBackendDispatching(prototype()),
      m_bLevelsSet(false), m_bLutSet(true),
      m_vecLUT(lut),
      m_ucQuantizationLevels(0),
      m_poBuffer(new Img8u())
    {}

    void LUTOp::setLUT(const std::vector<icl8u> &lut){
      m_vecLUT = lut;
      m_bLutSet = true;
      m_bLevelsSet = false;
      m_ucQuantizationLevels = 0;
    }
    void LUTOp::setQuantizationLevels(int levels){
      m_ucQuantizationLevels = levels;
      m_vecLUT.clear();
      m_bLutSet = false;
      m_bLevelsSet = true;
    }

    icl8u LUTOp::getQuantizationLevels() const{
      return m_ucQuantizationLevels;
    }
    const std::vector<icl8u> &LUTOp::getLUT() const{
      return m_vecLUT;
    }

    bool LUTOp::isLUTSet() const{
      return m_bLutSet;
    }
    bool LUTOp::isLevelsSet() const{
      return m_bLevelsSet;
    }


    void LUTOp::simple(const Img8u *src, Img8u *dst, const std::vector<icl8u>& lut){
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
      ICLASSERT_RETURN( lut.size() >= 256 );

      src->lut(lut.data(),dst,8);
    }

    void LUTOp::reduceBits(const Img8u *src, Img8u *dst, icl8u n){
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
      ICLASSERT_RETURN( n > 0 );
      std::vector<icl8u> lut(256), lv(n);
      float range = 255.0f / (n - 1);
      for(int i = 0; i < n; i++) {
        lv[i] = round(iclMin(i * range, 255.f));
      }
      for(int i = 0; i < 256; i++) {
        float rel = i / 256.f;
        lut[i] = lv[static_cast<int>(round(rel * (n - 1)))];
      }
      simple(src, dst, lut);
    }

    void LUTOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN(!src.isNull());

      const Img8u *src8u;
      if(src.getDepth() != depth8u){
        src.ptr()->convert(m_poBuffer);
        src8u = m_poBuffer;
      }else{
        src8u = &src.as8u();
      }
      if(!prepare(dst, src, depth8u)) return;

      if(m_bLevelsSet){
        getSelector<ReduceBitsSig>(Op::reduceBits).resolve(dst)->apply(
          *src8u, dst.as8u(), m_ucQuantizationLevels);
      }else{
        simple(src8u, &dst.as8u(), m_vecLUT);
      }
    }

  } // namespace filter
}
