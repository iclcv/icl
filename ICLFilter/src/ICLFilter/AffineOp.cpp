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
      template<class T>
      struct AffineImpl {
        static void apply(const Img<T> &src, Img<T> &dst,
                          const double M[3][3], scalemode interp) {
          Rect dr = dst.getROI();
          int sx = dr.x, sy = dr.y, ex = dr.right(), ey = dr.bottom();
          Rect r = src.getROI();

          for(int ch = 0; ch < src.getChannels(); ch++) {
            const Channel<T> srcCh = src[ch];
            Channel<T> dstCh = dst[ch];

            if(interp == interpolateLIN){
              for(int x = sx; x < ex; ++x){
                for(int y = sy; y < ey; ++y){
                  float x2 = M[0][0]*x + M[1][0]*y + M[2][0];
                  float y2 = M[0][1]*x + M[1][1]*y + M[2][1];
                  int x3 = round(x2);
                  int y3 = round(y2);
                  if(r.contains(x3,y3)){
                    dstCh(x,y) = src.subPixelLIN(x2,y2,ch);
                  }else{
                    dstCh(x,y) = 0;
                  }
                }
              }
            }else{
              for(int x = sx; x < ex; ++x){
                for(int y = sy; y < ey; ++y){
                  float x2 = M[0][0]*x + M[1][0]*y + M[2][0];
                  float y2 = M[0][1]*x + M[1][1]*y + M[2][1];
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
        }
      };

#ifdef ICL_HAVE_IPP
      template<>
      struct AffineImpl<icl8u> {
        static void apply(const Img8u &src, Img8u &dst,
                          const double (&aadT)[2][3], scalemode interp) {
          // IPP uses the original 2x3 transform matrix directly
          for(int c = 0; c < src.getChannels(); c++) {
            ippiWarpAffine_8u_C1R(src.getData(c),
                                  src.getSize(), src.getLineStep(),
                                  Rect(Point::null, src.getSize()),
                                  dst.getData(c),
                                  dst.getLineStep(), dst.getROI(),
                                  aadT, interp);
          }
        }
      };

      template<>
      struct AffineImpl<icl32f> {
        static void apply(const Img32f &src, Img32f &dst,
                          const double (&aadT)[2][3], scalemode interp) {
          for(int c = 0; c < src.getChannels(); c++) {
            ippiWarpAffine_32f_C1R(src.getData(c),
                                   src.getSize(), src.getLineStep(),
                                   Rect(Point::null, src.getSize()),
                                   dst.getData(c),
                                   dst.getLineStep(), dst.getROI(),
                                   aadT, interp);
          }
        }
      };
#endif
    } // anon namespace


    AffineOp::AffineOp (scalemode eInterpolate) : m_eInterpolate (eInterpolate),
                                                  m_adaptResultImage(true)  {
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

       src.visitWith(dst, [&](const auto &s, auto &d) {
         using T = typename std::remove_reference_t<decltype(s)>::type;
#ifdef ICL_HAVE_IPP
         if constexpr (std::is_same_v<T, icl8u> || std::is_same_v<T, icl32f>) {
           AffineImpl<T>::apply(s, d, m_aadT, m_eInterpolate);
         } else
#endif
         {
           // C++ fallback: compute inverse matrix for coordinate mapping
           FixedMatrix<double,3,3> M(m_aadT[0][0], m_aadT[0][1], m_aadT[0][2],
                                     m_aadT[1][0], m_aadT[1][1], m_aadT[1][2],
                                     0, 0, 1);
           double inv[3][3];
           M = M.inv();
           for(int i = 0; i < 3; ++i)
             for(int j = 0; j < 3; ++j)
               inv[i][j] = M(i,j);
           AffineImpl<T>::apply(s, d, inv, m_eInterpolate);
         }
       });

       if(m_adaptResultImage){
         translate(xShift, yShift);
       }
     }

  } // namespace filter
}
