/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/MirrorOp.cpp                   **
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

#include <ICLFilter/MirrorOp.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{
     MirrorOp::MirrorOp (axis eAxis) :
        m_eAxis (eAxis)
     {}

     void MirrorOp::apply(const Image &src, Image &dst) {
        Point oROIOffset;
        if(getClipToROI()){
           m_oSrcOffset = src.getROIOffset();
           oROIOffset = m_oDstOffset = Point::null;
           m_oSize = src.getROISize();
        } else {
           m_oDstOffset = m_oSrcOffset = Point::null;
           m_oSize = src.getSize();
           oROIOffset = src.getROIOffset();
           if(m_eAxis == axisHorz || m_eAxis == axisBoth)
              oROIOffset.y = m_oSize.height - oROIOffset.y - src.getROISize().height;
           if(m_eAxis == axisVert || m_eAxis == axisBoth)
              oROIOffset.x = m_oSize.width - oROIOffset.x - src.getROISize().width;
        }

        if(!prepare(dst, src.getDepth(), m_oSize,
                    src.getFormat(), src.getChannels(),
                    Rect(oROIOffset, src.getROISize()), src.getTime())) return;

        src.visitWith(dst, [this](const auto &s, auto &d) {
           for(int c=0; c < s.getChannels(); c++){
              flippedCopyChannelROI(m_eAxis, &s, c, m_oSrcOffset, m_oSize,
                                    &d, c, m_oDstOffset, m_oSize);
           }
        });
     }

  } // namespace filter
}
