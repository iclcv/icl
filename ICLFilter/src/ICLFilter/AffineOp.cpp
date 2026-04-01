// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <ICLFilter/AffineOp.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <cstring>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{

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

    AffineOp::AffineOp (scalemode eInterpolate)
      : m_eInterpolate(eInterpolate),
        ImageBackendDispatching(prototype()),
        m_adaptResultImage(true) {
       reset ();
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
         getShiftAndSize(src.getROI(), oSize, xShift, yShift);
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

  } // namespace filter
}
