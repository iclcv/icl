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

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

    LUTOp::LUTOp(icl8u quantizationLevels):
      m_bLevelsSet(true), m_bLutSet(false),
      m_ucQuantizationLevels(quantizationLevels),
      m_poBuffer(new Img8u()){

    }

    LUTOp::LUTOp(const std::vector<icl8u> &lut):
      m_bLevelsSet(false), m_bLutSet(true),
      m_vecLUT(lut),
      m_ucQuantizationLevels(0),
      m_poBuffer(new Img8u()){
    }

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


    void LUTOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
      ICLASSERT_RETURN(poSrc);
      ICLASSERT_RETURN(ppoDst);
      ICLASSERT_RETURN(poSrc != *ppoDst);


      if(poSrc->getDepth() != depth8u){
        poSrc->convert(m_poBuffer);
        poSrc = m_poBuffer;
      }
      if (!prepare (ppoDst, poSrc, depth8u)) return;

      if(m_bLevelsSet){
        reduceBits(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(),m_ucQuantizationLevels);
      }else{
        simple(poSrc->asImg<icl8u>(), (*ppoDst)->asImg<icl8u>(),m_vecLUT);
      }
    }

    void LUTOp::simple(const Img8u *src, Img8u *dst, const std::vector<icl8u>& lut){
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
      ICLASSERT_RETURN( lut.size() >= 256 );

      src->lut(lut.data(),dst,8);
    }

    void LUTOp::reduceBits(const Img8u *src, Img8u *dst, icl8u n){
  #ifdef ICL_HAVE_IPP
      ICLASSERT_RETURN( src && dst );
      ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
      ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
      ICLASSERT_RETURN( n > 0 );
      for(int c= src->getChannels()-1 ; c >= 0 ; --c){
        Ipp8u *pBuffer = NULL;
        int pBufferSize;
        ippiReduceBitsGetBufferSize(ippC1, src->getROISize(), 0, ippDitherNone, &pBufferSize);
        pBuffer = ippsMalloc_8u(pBufferSize);
        ippiReduceBits_8u_C1R(src->getROIData(c),src->getLineStep(),dst->getROIData(c),dst->getLineStep(),
                            src->getROISize(),0, ippDitherNone, n, pBuffer);
        ippsFree(pBuffer);
      }
  #else
      // n = nLevels
      std::vector<icl8u> lut(256),lv(n);
      float range = 255.0/(n-1);
      for(int i=0;i<n;i++) {
        lv[i] = round(iclMin(i*range,255.f)); //6 levels: [0,51,102,153,204,255]
      }
      for(int i=0;i<256;i++){
        float rel = i/256.f;
        lut[i]=lv[(int)round(rel * (n-1))];
      }
      simple(src,dst,lut);
  #endif
    }
  } // namespace filter
}
