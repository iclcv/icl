/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/AffineOp.cpp                   **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus      **
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

#include <ICLFilter/AffineOp.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/Img.h>
#include <ICLCore/Image.h>
#include <cstring>
#include <ICLMath/FixedMatrix.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace filter{

    namespace {

      // C++ fallback: inverse-map destination pixels to source via inverse matrix
      void cpp_affine(const Image &src, Image &dst, const double* fwd, scalemode interp) {
        // Compute inverse of the 2x3 forward transform (extended to 3x3)
        FixedMatrix<double,3,3> M(fwd[0], fwd[1], fwd[2],
                                   fwd[3], fwd[4], fwd[5],
                                   0, 0, 1);
        M = M.inv();
        double inv[3][3];
        for(int i = 0; i < 3; ++i)
          for(int j = 0; j < 3; ++j)
            inv[i][j] = M(i,j);

        src.visitWith(dst, [&](const auto &s, auto &d) {
          using T = typename std::remove_reference_t<decltype(s)>::type;
          Rect dr = d.getROI();
          int sx = dr.x, sy = dr.y, ex = dr.right(), ey = dr.bottom();
          Rect r = s.getROI();

          for(int ch = 0; ch < s.getChannels(); ch++) {
            const Channel<T> srcCh = s[ch];
            Channel<T> dstCh = d[ch];

            if(interp == interpolateLIN){
              for(int x = sx; x < ex; ++x){
                for(int y = sy; y < ey; ++y){
                  float x2 = inv[0][0]*x + inv[1][0]*y + inv[2][0];
                  float y2 = inv[0][1]*x + inv[1][1]*y + inv[2][1];
                  int x3 = round(x2);
                  int y3 = round(y2);
                  if(r.contains(x3,y3)){
                    dstCh(x,y) = s.subPixelLIN(x2,y2,ch);
                  }else{
                    dstCh(x,y) = 0;
                  }
                }
              }
            }else{
              for(int x = sx; x < ex; ++x){
                for(int y = sy; y < ey; ++y){
                  float x2 = inv[0][0]*x + inv[1][0]*y + inv[2][0];
                  float y2 = inv[0][1]*x + inv[1][1]*y + inv[2][1];
                  int x3 = round(x2);
                  int y3 = round(y2);
                  if(r.contains(x3,y3)){
                    dstCh(x,y) = srcCh(x3,y3);
                  }else{
                    dstCh(x,y) = 0;
                  }
                }
              }
            }
          }
        });
      }

    } // anonymous namespace


    AffineOp::AffineOp (scalemode eInterpolate) : m_eInterpolate (eInterpolate),
                                                    m_adaptResultImage(true)  {
       reset ();
       initDispatching("AffineOp");
       auto& sel = addSelector<AffineSig>("apply");
       sel.add(Backend::Cpp, cpp_affine);
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

       getSelector<AffineSig>("apply").resolve(src)->apply(
         src, dst, &m_aadT[0][0], m_eInterpolate);

       if(m_adaptResultImage){
         translate(xShift, yShift);
       }
     }

  } // namespace filter
}
