/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/MedianOp.cpp                   **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus,     **
**          Sergius Gaulik                                         **
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

#include <ICLFilter/MedianOp.h>
#include <ICLCore/Img.h>
#include <vector>
#include <algorithm>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    namespace{

      template<typename T>
      void apply_median (const Img<T> *src, Img<T> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) {
        // {{{ open
        std::vector<T> oList(oMaskSize.getDim());
        typename std::vector<T>::iterator itList = oList.begin();
        typename std::vector<T>::iterator itMedian = oList.begin()+((oMaskSize.width * oMaskSize.height)/2);


        for (int c=0;c<src->getChannels();c++){
          const ImgIterator<T> s(const_cast<T*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));
          const ImgIterator<T> sEnd = ImgIterator<T>::create_end_roi_iterator(src->getData(c),src->getWidth(),Rect(roiOffset, dst->getROISize()));
          ImgIterator<T> d= dst->beginROI(c);
          for(;s != sEnd;++s,++d){
            for(const ImgIterator<T> sR(s,oMaskSize,oAnchor); sR.inRegionSubROI(); ++sR, ++itList){
              *itList = *sR;
            }
            std::sort(oList.begin(),oList.end());
            *d = *itMedian;
            itList = oList.begin();
          }
        }
      }

      // }}}

      template<class T>
      inline void subMedian3x3(const T *l0, const T *l1, const T *l2, T *med) {
        T a0 = *l0++;
        T a1 = *l0++;
        T a2 = *l0;
        T b0 = *l1++;
        T b1 = *l1++;
        T b2 = *l1;
        T c0 = *l2++;
        T c1 = *l2++;
        T c2 = *l2;

        T A1 = std::min(a1, a2);
        a2 = std::max(a1, a2);
        a1 = std::max(a0, A1);
        a0 = std::min(a0, A1);
        A1 = std::min(a1, a2);
        a2 = std::max(a1, a2);

        T B1 = std::min(b1, b2);
        b2 = std::max(b1, b2);
        b1 = std::max(b0, B1);
        b0 = std::min(b0, B1);
        B1 = std::min(b1, b2);
        b2 = std::max(b1, b2);

        T C1 = std::min(c1, c2);
        c2 = std::max(c1, c2);
        c1 = std::max(c0, C1);
        c0 = std::min(c0, C1);
        C1 = std::min(c1, c2);
        c2 = std::max(c1, c2);

        a0 = std::max(a0, std::max(b0, c0));
        a2 = std::min(a2, std::max(b2, c2));
        b1 = std::min(B1, C1);
        b2 = std::max(B1, C1);
        b1 = std::max(A1, b1);
        a1 = std::min(b1, b2);

        b1 = std::min(a1, a2);
        b2 = std::max(a1, a2);
        b1 = std::max(a0, b1);
        *med = std::min(b1, b2);
      }

    #define MINMAX(tmp, a, b) \
        tmp = a;              \
        a = std::min(tmp, b); \
        b = std::max(tmp, b);

      template<class T1>
      inline void subMedian5x5(const T1 *l0, const T1 *l1, const T1 *l2, const T1 *l3, const T1 *l4, T1 *med) {
        T1 r00 = *l0++;
        T1 r01 = *l0++;
        T1 r02 = *l0++;
        T1 r03 = *l0++;
        T1 r04 = *l0;
        T1 r05 = *l1++;
        T1 r06 = *l1++;
        T1 r07 = *l1++;
        T1 r08 = *l1++;
        T1 r09 = *l1;
        T1 r10 = *l2++;
        T1 r11 = *l2++;
        T1 r12 = *l2++;
        T1 r13 = *l2++;
        T1 r14 = *l2;
        T1 r15 = *l3++;
        T1 r16 = *l3++;
        T1 r17 = *l3++;
        T1 r18 = *l3++;
        T1 r19 = *l3;
        T1 r20 = *l4++;
        T1 r21 = *l4++;
        T1 r22 = *l4++;
        T1 r23 = *l4++;
        T1 r24 = *l4;

        T1 tmp;

        MINMAX(tmp, r00, r01);

        MINMAX(tmp, r03, r04);
        MINMAX(tmp, r02, r04);
        MINMAX(tmp, r02, r03);

        MINMAX(tmp, r06, r07);
        MINMAX(tmp, r05, r07);
        MINMAX(tmp, r05, r06);

        MINMAX(tmp, r02, r05);

        MINMAX(tmp, r03, r06);
        MINMAX(tmp, r00, r06);
        MINMAX(tmp, r00, r03);

        MINMAX(tmp, r04, r07);
        MINMAX(tmp, r01, r07);
        MINMAX(tmp, r01, r04);


        MINMAX(tmp, r09, r10);
        MINMAX(tmp, r08, r10);
        MINMAX(tmp, r08, r09);

        MINMAX(tmp, r12, r13);
        MINMAX(tmp, r11, r13);
        MINMAX(tmp, r11, r12);

        MINMAX(tmp, r15, r16);
        MINMAX(tmp, r14, r16);
        MINMAX(tmp, r14, r15);

        MINMAX(tmp, r11, r14);
        MINMAX(tmp, r08, r14);
        MINMAX(tmp, r08, r11);

        MINMAX(tmp, r13, r16);
        MINMAX(tmp, r10, r16);
        MINMAX(tmp, r10, r13);


        MINMAX(tmp, r18, r19);
        MINMAX(tmp, r17, r19);
        MINMAX(tmp, r17, r18);

        MINMAX(tmp, r21, r22);
        MINMAX(tmp, r20, r22);
        MINMAX(tmp, r20, r21);

        MINMAX(tmp, r23, r24);

        MINMAX(tmp, r20, r23);
        MINMAX(tmp, r17, r23);
        MINMAX(tmp, r17, r20);

        MINMAX(tmp, r21, r24);
        MINMAX(tmp, r18, r24);
        MINMAX(tmp, r18, r21);

        MINMAX(tmp, r19, r22);


        r17 = std::max(r08, r17);
        MINMAX(tmp, r09, r18);
        MINMAX(tmp, r00, r18);
        MINMAX(tmp, r00, r09);
        r09 = std::max(r00, r09);
        MINMAX(tmp, r10, r19);
        MINMAX(tmp, r01, r19);
        MINMAX(tmp, r01, r10);
        MINMAX(tmp, r11, r20);
        MINMAX(tmp, r02, r20);
        r11 = std::max(r02, r11);
        MINMAX(tmp, r12, r21);
        MINMAX(tmp, r03, r21);
        MINMAX(tmp, r03, r12);
        MINMAX(tmp, r13, r22);
        MINMAX(tmp, r04, r22);
        r04 = std::min(r04, r22);
        MINMAX(tmp, r04, r13);
        MINMAX(tmp, r14, r23);
        MINMAX(tmp, r05, r23);
        MINMAX(tmp, r05, r14);
        MINMAX(tmp, r15, r24);
        r06 = std::min(r06, r24);
        MINMAX(tmp, r06, r15);
        r07 = std::min(r07, r16);
        r07 = std::min(r07, r19);
        r13 = std::min(r13, r21);
        r15 = std::min(r15, r23);
        r07 = std::min(r07, r13);
        r07 = std::min(r07, r15);
        r09 = std::max(r01, r09);
        r11 = std::max(r03, r11);
        r17 = std::max(r05, r17);
        r17 = std::max(r11, r17);
        r17 = std::max(r09, r17);
        MINMAX(tmp, r04, r10);
        MINMAX(tmp, r06, r12);
        MINMAX(tmp, r07, r14);
        MINMAX(tmp, r04, r06);
        r07 = std::max(r04, r07);
        MINMAX(tmp, r12, r14);
        r10 = std::min(r10, r14);
        MINMAX(tmp, r06, r07);
        MINMAX(tmp, r10, r12);
        MINMAX(tmp, r06, r10);
        r17 = std::max(r06, r17);
        MINMAX(tmp, r12, r17);
        r07 = std::min(r07, r17);
        MINMAX(tmp, r07, r10);
        MINMAX(tmp, r12, r18);
        r12 = std::max(r07, r12);
        r10 = std::min(r10, r18);
        MINMAX(tmp, r12, r20);
        r10 = std::min(r10, r20);

        *med = std::max(r10, r12);
      }

    #undef MINMAX

  #ifdef ICL_HAVE_SSE2

      // this one is only faster for images of the type Img8u
      template<class T, class T1>
      inline void sse_median3x3(const T *src0, T *dst0, T *dstEnd,
                                long srcWidth, long dstWidth, long lineWidth,
                                long step) {
        T *dstLEnd   = dst0 + lineWidth;
        T *dstSSEEnd = dstLEnd - (step - 1);

        for (; dst0<dstLEnd;) {
          if (dst0<dstSSEEnd) {
            const T *srcIt = src0;
            T *dstIt = dst0;

            T1 a0 = srcIt;
            T1 a1 = srcIt + 1;
            T1 a2 = srcIt + 2;
            srcIt += srcWidth;
            T1 b0 = srcIt;
            T1 b1 = srcIt + 1;
            T1 b2 = srcIt + 2;
            srcIt += srcWidth;

            T1 A1 = min(a1, a2);
            a2 = max(a1, a2);
            a1 = max(a0, A1);
            a0 = min(a0, A1);
            A1 = min(a1, a2);
            a2 = max(a1, a2);

            T1 B1 = min(b1, b2);
            b2 = max(b1, b2);
            b1 = max(b0, B1);
            b0 = min(b0, B1);
            B1 = min(b1, b2);
            b2 = max(b1, b2);

            for (; dstIt<dstEnd; dstIt += dstWidth, srcIt += srcWidth) {
              T1 c0 = a0;
              T1 c1 = A1;
              T1 c2 = a2;
              a0 = b0;
              A1 = B1;
              a2 = b2;
              b0 = srcIt;
              B1 = srcIt + 1;
              b2 = srcIt + 2;

              T1 C1 = min(c1, c2);
              c2 = max(c1, c2);
              c1 = max(c0, C1);
              c0 = min(c0, C1);
              C1 = min(c1, c2);
              c2 = max(c1, c2);

              a0 = max(a0, max(b0, c0));
              a2 = min(a2, max(b2, c2));
              b1 = min(B1, C1);
              b2 = max(B1, C1);
              b1 = max(A1, b1);
              a1 = min(b1, b2);

              b1 = min(a1, a2);
              b2 = max(a1, a2);
              b1 = max(a0, b1);
              min(b1, b2).storeu(dstIt);
            }

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
          } else {
            for (; dst0<dstLEnd; ++dst0, ++src0) {
              const T *srcIt = src0;
              T *dstIt = dst0;

              T a0 = *srcIt;
              T a1 = srcIt[1];
              T a2 = srcIt[2];
              srcIt += srcWidth;
              T b0 = *srcIt;
              T b1 = srcIt[1];
              T b2 = srcIt[2];
              srcIt += srcWidth;

              T A1 = std::min(a1, a2);
              a2 = std::max(a1, a2);
              a1 = std::max(a0, A1);
              a0 = std::min(a0, A1);
              A1 = std::min(a1, a2);
              a2 = std::max(a1, a2);

              T B1 = std::min(b1, b2);
              b2 = std::max(b1, b2);
              b1 = std::max(b0, B1);
              b0 = std::min(b0, B1);
              B1 = std::min(b1, b2);
              b2 = std::max(b1, b2);

              for (; dstIt<dstEnd; dstIt += dstWidth, srcIt += srcWidth) {
                T c0 = a0;
                T c1 = A1;
                T c2 = a2;
                a0 = b0;
                A1 = B1;
                a2 = b2;
                b0 = *srcIt;
                B1 = srcIt[1];
                b2 = srcIt[2];

                T C1 = std::min(c1, c2);
                c2 = std::max(c1, c2);
                c1 = std::max(c0, C1);
                c0 = std::min(c0, C1);
                C1 = std::min(c1, c2);
                c2 = std::max(c1, c2);

                a0 = std::max(a0, std::max(b0, c0));
                a2 = std::min(a2, std::max(b2, c2));
                b1 = std::min(B1, C1);
                b2 = std::max(B1, C1);
                b1 = std::max(A1, b1);
                a1 = std::min(b1, b2);

                b1 = std::min(a1, a2);
                b2 = std::max(a1, a2);
                b1 = std::max(a0, b1);
                *dstIt = std::min(b1, b2);
              }
            }
          }
        }
      }

      template<class T0, class T1>
      inline void subSSEMedian3x3(const T0 *l0, const T0 *l1, const T0 *l2, T0 *med) {
        T1 a0 = l0++;
        T1 a1 = l0++;
        T1 a2 = l0;
        T1 b0 = l1++;
        T1 b1 = l1++;
        T1 b2 = l1;
        T1 c0 = l2++;
        T1 c1 = l2++;
        T1 c2 = l2;

        T1 A1 = min(a1, a2);
        a2 = max(a1, a2);
        a1 = max(a0, A1);
        a0 = min(a0, A1);
        A1 = min(a1, a2);
        a2 = max(a1, a2);

        T1 B1 = min(b1, b2);
        b2 = max(b1, b2);
        b1 = max(b0, B1);
        b0 = min(b0, B1);
        B1 = min(b1, b2);
        b2 = max(b1, b2);

        T1 C1 = min(c1, c2);
        c2 = max(c1, c2);
        c1 = max(c0, C1);
        c0 = min(c0, C1);
        C1 = min(c1, c2);
        c2 = max(c1, c2);

        a0 = max(a0, max(b0, c0));
        a2 = min(a2, max(b2, c2));
        b1 = min(B1, C1);
        b2 = max(B1, C1);
        b1 = max(A1, b1);
        a1 = min(b1, b2);

        b1 = min(a1, a2);
        b2 = max(a1, a2);
        b1 = max(a0, b1);
        min(b1, b2).storeu(med);
      }

    #define MINMAX(tmp, a, b) \
        tmp = a;              \
        a = min(tmp, b);      \
        b = max(tmp, b);

      template<class T0, class T1>
      inline void subSSEMedian5x5(const T0 *l0, const T0 *l1, const T0 *l2, const T0 *l3, const T0 *l4, T0 *med) {
        T1 r00 = l0++;
        T1 r01 = l0++;
        T1 r02 = l0++;
        T1 r03 = l0++;
        T1 r04 = l0;
        T1 r05 = l1++;
        T1 r06 = l1++;
        T1 r07 = l1++;
        T1 r08 = l1++;
        T1 r09 = l1;
        T1 r10 = l2++;
        T1 r11 = l2++;
        T1 r12 = l2++;
        T1 r13 = l2++;
        T1 r14 = l2;
        T1 r15 = l3++;
        T1 r16 = l3++;
        T1 r17 = l3++;
        T1 r18 = l3++;
        T1 r19 = l3;
        T1 r20 = l4++;
        T1 r21 = l4++;
        T1 r22 = l4++;
        T1 r23 = l4++;
        T1 r24 = l4;

        T1 tmp;

        MINMAX(tmp, r00, r01);

        MINMAX(tmp, r03, r04);
        MINMAX(tmp, r02, r04);
        MINMAX(tmp, r02, r03);

        MINMAX(tmp, r06, r07);
        MINMAX(tmp, r05, r07);
        MINMAX(tmp, r05, r06);

        MINMAX(tmp, r02, r05);

        MINMAX(tmp, r03, r06);
        MINMAX(tmp, r00, r06);
        MINMAX(tmp, r00, r03);

        MINMAX(tmp, r04, r07);
        MINMAX(tmp, r01, r07);
        MINMAX(tmp, r01, r04);


        MINMAX(tmp, r09, r10);
        MINMAX(tmp, r08, r10);
        MINMAX(tmp, r08, r09);

        MINMAX(tmp, r12, r13);
        MINMAX(tmp, r11, r13);
        MINMAX(tmp, r11, r12);

        MINMAX(tmp, r15, r16);
        MINMAX(tmp, r14, r16);
        MINMAX(tmp, r14, r15);

        MINMAX(tmp, r11, r14);
        MINMAX(tmp, r08, r14);
        MINMAX(tmp, r08, r11);

        MINMAX(tmp, r13, r16);
        MINMAX(tmp, r10, r16);
        MINMAX(tmp, r10, r13);


        MINMAX(tmp, r18, r19);
        MINMAX(tmp, r17, r19);
        MINMAX(tmp, r17, r18);

        MINMAX(tmp, r21, r22);
        MINMAX(tmp, r20, r22);
        MINMAX(tmp, r20, r21);

        MINMAX(tmp, r23, r24);

        MINMAX(tmp, r20, r23);
        MINMAX(tmp, r17, r23);
        MINMAX(tmp, r17, r20);

        MINMAX(tmp, r21, r24);
        MINMAX(tmp, r18, r24);
        MINMAX(tmp, r18, r21);

        MINMAX(tmp, r19, r22);


        r17 = max(r08, r17);
        MINMAX(tmp, r09, r18);
        MINMAX(tmp, r00, r18);
        MINMAX(tmp, r00, r09);
        r09 = max(r00, r09);
        MINMAX(tmp, r10, r19);
        MINMAX(tmp, r01, r19);
        MINMAX(tmp, r01, r10);
        MINMAX(tmp, r11, r20);
        MINMAX(tmp, r02, r20);
        r11 = max(r02, r11);
        MINMAX(tmp, r12, r21);
        MINMAX(tmp, r03, r21);
        MINMAX(tmp, r03, r12);
        MINMAX(tmp, r13, r22);
        MINMAX(tmp, r04, r22);
        r04 = min(r04, r22);
        MINMAX(tmp, r04, r13);
        MINMAX(tmp, r14, r23);
        MINMAX(tmp, r05, r23);
        MINMAX(tmp, r05, r14);
        MINMAX(tmp, r15, r24);
        r06 = min(r06, r24);
        MINMAX(tmp, r06, r15);
        r07 = min(r07, r16);
        r07 = min(r07, r19);
        r13 = min(r13, r21);
        r15 = min(r15, r23);
        r07 = min(r07, r13);
        r07 = min(r07, r15);
        r09 = max(r01, r09);
        r11 = max(r03, r11);
        r17 = max(r05, r17);
        r17 = max(r11, r17);
        r17 = max(r09, r17);
        MINMAX(tmp, r04, r10);
        MINMAX(tmp, r06, r12);
        MINMAX(tmp, r07, r14);
        MINMAX(tmp, r04, r06);
        r07 = max(r04, r07);
        MINMAX(tmp, r12, r14);
        r10 = min(r10, r14);
        MINMAX(tmp, r06, r07);
        MINMAX(tmp, r10, r12);
        MINMAX(tmp, r06, r10);
        r17 = max(r06, r17);
        MINMAX(tmp, r12, r17);
        r07 = min(r07, r17);
        MINMAX(tmp, r07, r10);
        MINMAX(tmp, r12, r18);
        r12 = max(r07, r12);
        r10 = min(r10, r18);
        MINMAX(tmp, r12, r20);
        r10 = min(r10, r20);

        max(r10, r12).storeu(med);
      }

    #undef MINMAX
  #endif

#ifdef ICL_HAVE_SSE2
      template<class T>
      void apply_median_all(const Img<T> *src, Img<T> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) {
        // {{{ open
        std::vector<T> oList(oMaskSize.getDim());
        typename std::vector<T>::iterator itList = oList.begin();
        typename std::vector<T>::iterator itMedian = oList.begin()+((oMaskSize.width * oMaskSize.height)/2);


        for (int c=0;c<src->getChannels();c++){
          const ImgIterator<T> s(const_cast<T*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));
          const ImgIterator<T> sEnd = ImgIterator<T>::create_end_roi_iterator(src->getData(c),src->getWidth(),Rect(roiOffset, dst->getROISize()));
          ImgIterator<T> d= dst->beginROI(c);
          for(;s != sEnd;++s,++d){
            for(const ImgIterator<T> sR(s,oMaskSize,oAnchor); sR.inRegionSubROI(); ++sR, ++itList){
              *itList = *sR;
            }
            std::sort(oList.begin(),oList.end());
            *d = *itMedian;
            itList = oList.begin();
          }
        }
      }

      template<>
      void apply_median_all(const Img<icl8u> *src, Img<icl8u> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) {
        // {{{ open
        const int half      = oMaskSize.getDim() / 2;
        const int halfexact = (oMaskSize.getDim() + 1) / 2;

        for (int c = 0; c < src->getChannels(); c++) {
          ImgIterator<icl8u> d    = dst->beginROI(c);
          const ImgIterator<icl8u> s(const_cast<icl8u*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));
          const ImgIterator<icl8u> sR(s,oMaskSize,oAnchor);
          const icl8u *s_tl = &(*sR);

          for (int k = 0; k < dst->getROIWidth(); ++k, ++d, ++s_tl, ++s) {
            icl8u median = 0;
            icl16s hist[256] = {0};
            icl16s left = 0;

            for (const ImgIterator<icl8u> sR(s,oMaskSize,oAnchor); sR.inRegionSubROI(); ++sR){
              hist[*sR]++;
            }


            icl16s sum = 0;

            for (int i = 0; i < 256; ++i) {
              sum += hist[i];
              if (sum >= halfexact) {
                *d = median = i;
                left = sum - hist[i];

                break;
              }
            }


            const icl8u *sRtl = s_tl;
            const icl8u *sRbl = &(s_tl[src->getWidth()*oMaskSize.height]);

            for (ImgIterator<icl8u> dCol(d,Size(1,dst->getROIHeight()-1),Point(0,-1)); dCol.inRegionSubROI();
                  ++dCol, sRtl = &(sRtl[src->getWidth()]), sRbl = &(sRbl[src->getWidth()])) {
              for (int i = 0; i < oMaskSize.width; ++i) {
                hist[sRtl[i]]--;
                if (sRtl[i] < median) --left;
                hist[sRbl[i]]++;
                if (sRbl[i] < median) ++left;
              }

              if (left > half) {
                do {
                  --median;
                  left -= hist[median];
                } while(left > half);
              } else {
                while (left + hist[median] < halfexact) {
                  left += hist[median];
                  ++median;
                }
              }

              *dCol = median;
            }
          }
        }
      }
      template<>
      void apply_median_all(const Img<icl16s> *src, Img<icl16s> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) {
        // {{{ open
        const int half      = oMaskSize.getDim() / 2;
        const int halfexact = (oMaskSize.getDim() + 1) / 2;

        for (int c = 0; c < src->getChannels(); c++) {
          ImgIterator<icl16s> d    = dst->beginROI(c);
          const ImgIterator<icl16s> s(const_cast<icl16s*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));
          const ImgIterator<icl16s> sR(s,oMaskSize,oAnchor);
          const icl16s *s_tl = &(*sR);

          for (int k = 0; k < dst->getROIWidth(); ++k, ++d, ++s_tl, ++s) {
            icl16s median = 0;
            icl16s hist[65536] = {0};
            icl16s left = 0;

            for (const ImgIterator<icl16s> sR(s,oMaskSize,oAnchor); sR.inRegionSubROI(); ++sR){
              hist[*sR]++;
            }


            icl16s sum = 0;

            for (int i = 0; i < 65536; ++i) {
              sum += hist[i];
              if (sum >= halfexact) {
                *d = median = i;
                left = sum - hist[i];

                break;
              }
            }


            const icl16s *sRtl = s_tl;
            const icl16s *sRbl = &(s_tl[src->getWidth()*oMaskSize.height]);

            for (ImgIterator<icl16s> dCol(d,Size(1,dst->getROIHeight()-1),Point(0,-1)); dCol.inRegionSubROI();
                  ++dCol, sRtl = &(sRtl[src->getWidth()]), sRbl = &(sRbl[src->getWidth()])) {
              for (int i = 0; i < oMaskSize.width; ++i) {
                hist[sRtl[i]]--;
                if (sRtl[i] < median) --left;
                hist[sRbl[i]]++;
                if (sRbl[i] < median) ++left;
              }

              if (left > half) {
                do {
                  --median;
                  left -= hist[median];
                } while(left > half);
              } else {
                while (left + hist[median] < halfexact) {
                  left += hist[median];
                  ++median;
                }
              }

              *dCol = median;
            }
          }
        }
      }
      //#endif
      //#ifdef ICL_HAVE_SSE2

      #define APPLY_MEDIAN(T0, T1, STEP)                                                                                          \
        template<>                                                                                                                \
        void apply_median(const Img<T0> *src, Img<T0> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) { \
          if (oMaskSize == Size(3,3)) {                                                                                           \
            for (int c = 0; c < src->getChannels(); c++) {                                                                        \
              const ImgIterator<T0> s_it(const_cast<T0*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));   \
              const T0 *s = &(*s_it);                                                                                             \
              T0 *d = dst->getROIData(c);                                                                                         \
              ImgIterator<T0> d_end = dst->endROI(c);                                                                             \
                                                                                                                                  \
              sse_for(s - src->getWidth() - 1, s - 1, s + src->getWidth() - 1, d, &(*d_end),                                      \
                      src->getWidth(), dst->getWidth(), dst->getROIWidth(),                                                       \
                      subMedian3x3<T0>, subSSEMedian3x3<T0,T1>, STEP);                                                            \
            }                                                                                                                     \
          } else if (oMaskSize == Size(5,5)) {                                                                                    \
            for (int c = 0; c < src->getChannels(); c++) {                                                                        \
              const ImgIterator<T0> s_it(const_cast<T0*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));   \
              const T0 *s = &(*s_it);                                                                                             \
              T0 *d = dst->getROIData(c);                                                                                         \
              ImgIterator<T0> d_end = dst->endROI(c);                                                                             \
                                                                                                                                  \
              sse_for(s - 2 * src->getWidth() - 2, s - src->getWidth() - 2, s - 2,                                                \
                      s + src->getWidth() - 2, s + 2 * src->getWidth() - 2, d, &(*d_end),                                         \
                      src->getWidth(), dst->getWidth(), dst->getROIWidth(),                                                       \
                      subMedian5x5<T0>, subSSEMedian5x5<T0,T1>, STEP);                                                            \
            }                                                                                                                     \
          } else {                                                                                                                \
            apply_median_all(src, dst, oMaskSize, roiOffset, oAnchor);                                                            \
          }                                                                                                                       \
        }

      #define APPLY_MEDIAN2(T0, T1, STEP)                                                                                         \
        template<>                                                                                                                \
        void apply_median(const Img<T0> *src, Img<T0> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) { \
          if (oMaskSize == Size(3,3)) {                                                                                           \
            for (int c = 0; c < src->getChannels(); c++) {                                                                        \
              const ImgIterator<T0> s_it(const_cast<T0*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));   \
              const T0 *s = &(*s_it);                                                                                             \
              T0 *d = dst->getROIData(c);                                                                                         \
              ImgIterator<T0> d_end = dst->endROI(c);                                                                             \
                                                                                                                                  \
              sse_median3x3<T0,T1>(s - src->getWidth() - 1, d, &(*d_end),                                                         \
                      src->getWidth(), dst->getWidth(), dst->getROIWidth(),                                                       \
                      STEP);                                                                                                      \
            }                                                                                                                     \
          } else if (oMaskSize == Size(5,5)) {                                                                                    \
            for (int c = 0; c < src->getChannels(); c++) {                                                                        \
              const ImgIterator<T0> s_it(const_cast<T0*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));   \
              const T0 *s = &(*s_it);                                                                                             \
              T0 *d = dst->getROIData(c);                                                                                         \
              ImgIterator<T0> d_end = dst->endROI(c);                                                                             \
                                                                                                                                  \
              sse_for(s - 2 * src->getWidth() - 2, s - src->getWidth() - 2, s - 2,                                                \
                      s + src->getWidth() - 2, s + 2 * src->getWidth() - 2, d, &(*d_end),                                         \
                      src->getWidth(), dst->getWidth(), dst->getROIWidth(),                                                       \
                      subMedian5x5<T0>, subSSEMedian5x5<T0,T1>, STEP);                                                            \
            }                                                                                                                     \
          } else {                                                                                                                \
            apply_median_all(src, dst, oMaskSize, roiOffset, oAnchor);                                                            \
          }                                                                                                                       \
        }

  #else

      #define APPLY_MEDIAN(T0, STEP)                                                                                          \
        template<>                                                                                                                \
        void apply_median(const Img<T0> *src, Img<T0> *dst, const Size &oMaskSize,const Point &roiOffset, const Point &oAnchor) { \
          if (oMaskSize == Size(3,3)) {                                                                                           \
            for (int c = 0; c < src->getChannels(); c++) {                                                                        \
              const ImgIterator<T0> s(const_cast<T0*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));      \
              ImgIterator<T0> d     = dst->beginROI(c);                                                                           \
              ImgIterator<T0> d_end = dst->endROI(c);                                                                             \
                                                                                                                                  \
              for (; d != d_end; ++d, ++s) {                                                                                      \
                subMedian3x3<T0>(&(*s) - src->getWidth() - 1, &(*s) - 1, &(*s) + src->getWidth() - 1, &(*d));                     \
              }                                                                                                                   \
            }                                                                                                                     \
          } else if (oMaskSize == Size(5,5)) {                                                                                    \
            for (int c = 0; c < src->getChannels(); c++) {                                                                        \
              const ImgIterator<T0> s(const_cast<T0*>(src->getData(c)),src->getWidth(), Rect(roiOffset, dst->getROISize()));      \
              ImgIterator<T0> d     = dst->beginROI(c);                                                                           \
              ImgIterator<T0> d_end = dst->endROI(c);                                                                             \
                                                                                                                                  \
              for (; d != d_end; ++d, ++s) {                                                                                      \
                subMedian5x5<T0>(&(*s) - 2 * src->getWidth() - 2, &(*s) - src->getWidth() - 2, &(*s) - 2,                         \
                                 &(*s) + src->getWidth() - 2, &(*s) + 2 * src->getWidth() - 2, &(*d));                            \
              }                                                                                                                   \
            }                                                                                                                     \
          } else {                                                                                                                \
            apply_median_all(src, dst, oMaskSize, roiOffset, oAnchor);                                                            \
          }                                                                                                                       \
        }

  #endif

  #ifndef ICL_HAVE_IPP

    #ifdef ICL_HAVE_SSE2
      APPLY_MEDIAN2(icl8u, icl128i8u, 16);
      APPLY_MEDIAN(icl16s, icl128i16s, 8);
    #else
      APPLY_MEDIAN(icl8u, 16);
      APPLY_MEDIAN(icl16s, 8);
    #endif

  #endif

    #ifdef ICL_HAVE_SSE2
      APPLY_MEDIAN(icl32f, icl128, 4);
      #undef APPLY_MEDIAN2
    #else
      APPLY_MEDIAN(icl32f, 4);
    #endif

      #undef APPLY_MEDIAN

  #ifdef ICL_HAVE_IPP

      template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiSize, IppiPoint)>
      void ippMedian(const Img<T>* src, Img<T> *dst, const Size& maskSize,const Point &roiOffset, const Point &oAnchor) {
        // {{{ open

      for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData (c,roiOffset), src->getLineStep(),
                 dst->getROIData (c), dst->getLineStep(),
                 dst->getROISize(), maskSize, oAnchor);
       }
    }

      // }}}

      template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiMaskSize)>
      void ippMedianFixed(const Img<T>*src, Img<T> *dst,const Point &roiOffset, int maskSize) {
        // {{{ open

        for(int c=0; c < src->getChannels(); c++) {
        ippiFunc(src->getROIData(c,roiOffset), src->getLineStep(),
                 dst->getROIData(c), dst->getLineStep(),
                 dst->getROISize(), maskSize == 3 ? ippMskSize3x3 : ippMskSize5x5);
      }
    }

      // }}}

      template<>
      void apply_median<icl8u>(const Img8u *src, Img8u *dst, const Size &maskSize,const Point &roiOffset, const Point &anchor){
        // {{{ open

        if(maskSize == Size(3,3)){
          ippMedianFixed<icl8u,ippiFilterMedianCross_8u_C1R>(src,dst,roiOffset,3);
        }else if(maskSize == Size(5,5)){
          ippMedianFixed<icl8u,ippiFilterMedianCross_8u_C1R>(src,dst,roiOffset,5);
        }else{
          WARNING_LOG("only 3x3 and 5x5 medianOp supported in IPP implementation");
          //ippiFilterMedianBorder_8u_C1R implementation possible
          //ippMedian<icl8u,ippiFilterMedian_8u_C1R>(src,dst,maskSize,roiOffset,anchor);//only 64f_C1R supported
        }
      }

      // }}}

      template<>
      void apply_median<icl16s>(const Img16s *src, Img16s *dst, const Size &maskSize,const Point &roiOffset, const Point &anchor){
        // {{{ open

        if(maskSize == Size(3,3)){
          ippMedianFixed<icl16s,ippiFilterMedianCross_16s_C1R>(src,dst,roiOffset,3);
        }else if(maskSize == Size(5,5)){
          ippMedianFixed<icl16s,ippiFilterMedianCross_16s_C1R>(src,dst,roiOffset,5);
        }else{
          WARNING_LOG("only 3x3 and 5x5 medianOp supported in IPP implementation");
          //ippMedian<icl16s,ippiFilterMedian_16s_C1R>(src,dst,maskSize,roiOffset,anchor);//only 64f_C1R supported
        }
      }

  /* our SSE version is always faster than IPP for icl32f
      template<>
      void apply_median<icl32f>(const Img32f *src, Img32f *dst, const Size &maskSize,const Point &roiOffset, const Point &anchor){
        // {{{ open

        ippMedian<icl32f,ippiFilterMedian_32f_C1R>(src,dst,maskSize,roiOffset,anchor);
      }

      // }}}
  */

  #endif

    } // end of anonymous namespace

    void MedianOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
      // {{{ open

      FUNCTION_LOG("");

      if (!prepare (ppoDst, poSrc)) return;

      switch(poSrc->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                        \
        case depth##D:                                    \
          apply_median(poSrc->asImg<icl##D>(),            \
                       (*ppoDst)->asImg<icl##D>(),        \
                       getMaskSize(),                     \
                       getROIOffset(),                    \
                       getAnchor());                      \
          break;
        ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
      }

    }

    // }}}

  } // namespace filter
}
