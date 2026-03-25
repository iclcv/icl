/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/WeightChannelsOp.cpp           **
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

#include <ICLFilter/WeightChannelsOp.h>
#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLCore/Image.h>

using namespace icl::core;

namespace icl {

namespace filter{

    void WeightChannelsOp::applyImgBase (const ImgBase *poSrc, ImgBase **ppoDst) {
      ICLASSERT_RETURN(poSrc);
      ICLASSERT_RETURN( static_cast<int>(m_vecWeights.size()) == poSrc->getChannels() );

      const ImgBase *oTmpSrcImg;
      ImgBase *oTmpDstImg;

      if(!UnaryOp::prepare(ppoDst,poSrc)) return;

      UnaryArithmeticalOp op = UnaryArithmeticalOp(UnaryArithmeticalOp::mulOp);

      for (int c=0;c<poSrc->getChannels();c++) {
        oTmpSrcImg = poSrc->selectChannel(c);
        oTmpDstImg = (*ppoDst)->selectChannel(c);
        op.setValue(m_vecWeights[c]);
        op.apply(oTmpSrcImg, &oTmpDstImg);
        delete oTmpSrcImg;
        delete oTmpDstImg;
      }
    }

    void WeightChannelsOp::apply(const core::Image &src, core::Image &dst) {
      // TODO: use Image natively!
      ImgBase *dstPtr = dst.isNull() ? nullptr : dst.ptr();
      applyImgBase(src.ptr(), &dstPtr);
      if(dstPtr) dst = core::Image(*dstPtr);
    }

  } // namespace filter
} // namespace icl
