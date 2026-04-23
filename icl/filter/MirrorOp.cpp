// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/MirrorOp.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
   static const char *AXIS_MENU = "axisHorz,axisVert,axisBoth";
   static const char *axisName(axis a){
     switch(a){
       case axisHorz: return "axisHorz";
       case axisVert: return "axisVert";
       case axisBoth: return "axisBoth";
     }
     return "axisHorz";
   }
   static axis parseAxis(const std::string &s){
     if(s == "axisVert") return axisVert;
     if(s == "axisBoth") return axisBoth;
     return axisHorz;
   }

   MirrorOp::MirrorOp (axis eAxis) :
      m_eAxis (eAxis)
   {
     addProperty("axis", utils::prop::menuFromCsv(AXIS_MENU), axisName(eAxis));
     registerCallback([this](const Property &p){
       if(p.name == "axis") m_eAxis = parseAxis(p.as<std::string>());
     });
   }

   REGISTER_CONFIGURABLE(MirrorOp, return new MirrorOp(axisHorz));

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

  } // namespace icl::filter