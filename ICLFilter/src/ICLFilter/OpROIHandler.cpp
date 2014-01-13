/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/OpROIHandler.cpp               **
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

#include <ICLUtils/Macros.h>
#include <ICLFilter/OpROIHandler.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{
  
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
  } // namespace filter
}
