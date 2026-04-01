// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

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
