/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/OpROIHandler.cpp                         **
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

#include <ICLFilter/OpROIHandler.h>
#include <ICLUtils/Macros.h>

namespace icl {

   /// check+adapt destination images parameters against given values
  bool OpROIHandler::prepare (ImgBase **ppoDst, depth eDepth, 
                              const Size &imgSize, format eFormat, int nChannels, 
                              const Rect& roi, Time timestamp) {
    ICLASSERT_RETURN_VAL (ppoDst, false);
    if (m_bCheckOnly) {
      ImgBase* dst = *ppoDst;
      ICLASSERT_RETURN_VAL( dst , false);
      ICLASSERT_RETURN_VAL( dst->getDepth() == eDepth , false);
      ICLASSERT_RETURN_VAL( dst->getChannels () == nChannels ,false);
      ICLASSERT_RETURN_VAL( dst->getFormat () == eFormat ,false);
      if(dst->getROISize() != roi.getSize()){
        ERROR_LOG("ROI size missmatch: given: "<< roi.getSize() 
                  << "  destination: "<< dst->getROISize());
        return false;
      }
      
      dst->setTime(timestamp);
    } else {
      ensureCompatible (ppoDst, eDepth, imgSize, 
                        nChannels, eFormat, roi);
      (*ppoDst)->setTime(timestamp);
    }
    return true;
  }
}
