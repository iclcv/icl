// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <icl/filter/AffineOp.h>
#include <icl/core/CoreFunctions.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <cstring>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(AffineOp::Op op) {
    switch(op) {
      case AffineOp::Op::apply: return "apply";
    }
    return "?";
  }

  core::ImageBackendDispatching& AffineOp::prototype() {
    static core::ImageBackendDispatching proto;
    static bool init = [&] {
      proto.addSelector<AffineSig>(Op::apply);
      return true;
    }();
    (void)init;
    return proto;
  }

  static const char *INTERP_MENU = "NN,LIN,RA";

  static const char *interpName(core::scalemode m){
    switch(m){
      case core::interpolateNN:  return "NN";
      case core::interpolateLIN: return "LIN";
      case core::interpolateRA:  return "RA";
    }
    return "LIN";
  }
  static core::scalemode parseInterp(const std::string &s){
    if(s == "NN") return core::interpolateNN;
    if(s == "RA") return core::interpolateRA;
    return core::interpolateLIN;
  }

  // Rebuild m_aadT from the four scalar knobs (scale, rotate.deg, translate.x/y)
  // in a fixed order: identity → scale → rotate → translate. Any imperative
  // rotate()/scale()/translate() calls between property changes are lost on
  // the next property-driven rebuild — the property knobs are the authoritative
  // source of truth for property-consumer code (e.g. filter-playground).
  void AffineOp::rebuildFromProperties(){
    const double sx = prop("scale.x").as<double>();
    const double sy = prop("scale.y").as<double>();
    const double deg = prop("rotate.deg").as<double>();
    const double tx = prop("translate.x").as<double>();
    const double ty = prop("translate.y").as<double>();
    reset();
    scale(sx, sy);
    rotate(deg);
    translate(tx, ty);
  }

  AffineOp::AffineOp (scalemode eInterpolate)
    : ImageBackendDispatching(prototype()),
      m_eInterpolate(eInterpolate),
      m_adaptResultImage(true) {
    reset();
    addProperty("scale.x",utils::prop::Range{.min=0.1f, .max=5.f, .step=0.01f}, 1.0);
    addProperty("scale.y",utils::prop::Range{.min=0.1f, .max=5.f, .step=0.01f}, 1.0);
    addProperty("rotate.deg",utils::prop::Range{.min=-360.f, .max=360.f, .step=0.1f}, 0);
    addProperty("translate.x",utils::prop::Range{.min=-500, .max=500}, 0);
    addProperty("translate.y",utils::prop::Range{.min=-500, .max=500}, 0);
    addProperty("interpolation",utils::prop::menuFromCsv(INTERP_MENU), interpName(eInterpolate));
    registerCallback([this](const Property &p){
      if(p.name == "interpolation"){
        m_eInterpolate = parseInterp(p.as<std::string>());
      }else if(p.name == "scale.x"     || p.name == "scale.y" ||
               p.name == "rotate.deg"  ||
               p.name == "translate.x" || p.name == "translate.y"){
        rebuildFromProperties();
      }
    });
  }

   void AffineOp::reset () {
      m_aadT[0][0] = m_aadT[1][1] = 1.0;
      m_aadT[0][1] = m_aadT[1][0] = m_aadT[0][2] = m_aadT[1][2] = 0.0;
   }

   void AffineOp::rotate (double dAngle) {
      double c=cos(dAngle * M_PI / 180.);
      double s=sin(dAngle * M_PI / 180.);
      double O[2][3]; memcpy (O, m_aadT, 6*sizeof(double));

      m_aadT[0][0] = O[0][0]*c + O[0][1]*s;
      m_aadT[0][1] = O[0][1]*c - O[0][0]*s;
      m_aadT[1][0] = O[1][0]*c + O[1][1]*s;
      m_aadT[1][1] = O[1][1]*c - O[1][0]*s;
   }

   inline void AffineOp::applyT (const double p[2], double adResult[2]) {
      adResult[0] = m_aadT[0][0]*p[0] + m_aadT[0][1]*p[1] + m_aadT[0][2];
      adResult[1] = m_aadT[1][0]*p[0] + m_aadT[1][1]*p[1] + m_aadT[1][2];
   }

   inline void AffineOp::useMinMax (const double adCur[2],
                                  double adMin[2], double adMax[2]) {
      adMin[0] = std::min (adCur[0], adMin[0]);
      adMin[1] = std::min (adCur[1], adMin[1]);
      adMax[0] = std::max (adCur[0], adMax[0]);
      adMax[1] = std::max (adCur[1], adMax[1]);
   }

   void AffineOp::getShiftAndSize (const Rect& roi, Size& size,
                                 double& xShift, double& yShift) {
      double adMin[2], adMax[2], adCur[2];
      double aadRect[4][2] = {{static_cast<double>(roi.x), static_cast<double>(roi.y)},
                              {static_cast<double>(roi.x) + static_cast<double>(roi.width), static_cast<double>(roi.y)},
                              {static_cast<double>(roi.x) + static_cast<double>(roi.width), static_cast<double>(roi.y) + static_cast<double>(roi.height)},
                              {static_cast<double>(roi.x), static_cast<double>(roi.y) + static_cast<double>(roi.height)}};

      applyT (aadRect[0], adMax); adMin[0] = adMax[0]; adMin[1] = adMax[1];
      applyT (aadRect[1], adCur); useMinMax (adCur, adMin, adMax);
      applyT (aadRect[2], adCur); useMinMax (adCur, adMin, adMax);
      applyT (aadRect[3], adCur); useMinMax (adCur, adMin, adMax);

      size.width  = static_cast<int>(ceil(adMax[0] - adMin[0])); xShift = adMin[0];
      size.height = static_cast<int>(ceil(adMax[1] - adMin[1])); yShift = adMin[1];
   }

   void AffineOp::apply(const Image &src, Image &dst) {
     ICLASSERT_RETURN(!src.isNull());

     double xShift=0, yShift=0;
     Size oSize;

     if(m_adaptResultImage){
       // dst is sized to the bbox of a transformed region. clipToROI
       // controls which region: src.getROI() (tight dst, only ROI is
       // visible) vs src.getImageRect() (dst big enough to contain the
       // full transformed image — pixels outside the ROI appear as the
       // backend's background color, because backends always sample
       // only within src.getROI()).
       const Rect region = getClipToROI() ? src.getROI() : src.getImageRect();
       getShiftAndSize(region, oSize, xShift, yShift);
       translate(-xShift, -yShift);
     }else{
       oSize = src.getSize();
     }

     if(!prepare(dst, src.getDepth(), oSize,
                 src.getFormat(), src.getChannels(),
                 Rect(Point::null, oSize), src.getTime())) return;

     getSelector<AffineSig>(Op::apply).resolve(src)->apply(
       src, dst, &m_aadT[0][0], m_eInterpolate);

     if(m_adaptResultImage){
       translate(xShift, yShift);
     }
   }

  REGISTER_CONFIGURABLE_DEFAULT(AffineOp);
  } // namespace icl::filter