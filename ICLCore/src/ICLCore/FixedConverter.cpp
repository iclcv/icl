/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/FixedConverter.cpp                 **
** Module : ICLCore                                                **
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

#include <ICLCore/FixedConverter.h>


namespace icl{
  namespace core{

    FixedConverter::FixedConverter(const ImgParams &p, depth d, bool applyToROIOnly):
      m_oParams(p),m_oConverter(applyToROIOnly),m_eDepth(d) { }

    void FixedConverter::apply(const ImgBase *poSrc, ImgBase **ppoDst){
      FUNCTION_LOG("");
      ICLASSERT_RETURN( poSrc );
      ICLASSERT_RETURN( ppoDst );
      ensureCompatible(ppoDst,m_eDepth,m_oParams);
      m_oConverter.apply(poSrc,*ppoDst);
    }
  } // namespace core
}
