/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
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

#include <ICLFilter/LUTOp.h>

namespace icl{
  
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
    
    for(int c=src->getChannels()-1; c >= 0; --c) {
      const ImgIterator<icl8u> itSrc = src->beginROI(c);
      const ImgIterator<icl8u> itSrcEnd = src->endROI(c);
      ImgIterator<icl8u> itDst = dst->beginROI(c);
      for(;itSrc != itSrcEnd ; ++itSrc, ++itDst){
        *itDst = lut[*itSrc];
      }
    }    
  }

  void LUTOp::reduceBits(const Img8u *src, Img8u *dst, icl8u n){
#ifdef HAVE_IPP
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getROISize() == dst->getROISize() );
    ICLASSERT_RETURN( src->getChannels() == dst->getChannels() );
    ICLASSERT_RETURN( n > 0 );
    for(int c= src->getChannels()-1 ; c >= 0 ; --c){
      ippiReduceBits_8u_C1R(src->getROIData(c),src->getLineStep(),dst->getROIData(c),dst->getLineStep(),
                          src->getROISize(),0, ippDitherNone, n);
    }
#else
    std::vector<icl8u> lut(256),lv;
    float range = 256.0/n;
    for(int i=0;i<n;i++)lv.push_back((int)round(i*range));
    for(int i=0;i<256;i++){
      lut[i]=lv[(int)round((float)i/n)];
    }
    // calculate table
    simple(src,dst,lut);
#endif
  }
  
}
