/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SSEUtils.h                       **
** Module : ICLUtils                                               **
** Authors: Sergius Gaulik                                         **
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

#pragma once

#ifdef USE_SSE
  #ifdef __SSE2__
    #include "emmintrin.h"
    #define __ICL_SSE2__
    #define ICL_SSE2
    #define HAVE_SSE2
    #ifdef __SSE3__
      #include "pmmintrin.h"
      #define __ICL_SSE3__
      #define ICL_SSE3
      #define HAVE_SSE3
      #if defined __SSSE3__
        #include "tmmintrin.h"
        #define HAVE_SSSE3
      #endif
    #endif
  #endif
#endif

#include <ICLUtils/SSETypes.h>

namespace icl{
  namespace utils{

    #ifdef HAVE_SSE2

      // ++ rounding ++ //

      // possible modes:
      // _MM_ROUND_NEAREST
      // _MM_ROUND_DOWN
      // _MM_ROUND_UP
      // _MM_ROUND_TOWARD_ZERO

      static const unsigned int INITIAL_ROUNDING_MODE = _MM_GET_ROUNDING_MODE();
      static unsigned int PREVIOUS_ROUNDING_MODE = INITIAL_ROUNDING_MODE;

      inline void sse_restore_initial_rounding_mode() {
        _MM_SET_ROUNDING_MODE(INITIAL_ROUNDING_MODE);
      }

      inline void sse_restore_previous_rounding_mode() {
        const unsigned int mode = _MM_GET_ROUNDING_MODE();
        _MM_SET_ROUNDING_MODE(PREVIOUS_ROUNDING_MODE);
        PREVIOUS_ROUNDING_MODE = mode;
      }

      inline void sse_set_rounding_mode(const unsigned int mode) {
        PREVIOUS_ROUNDING_MODE = _MM_GET_ROUNDING_MODE();
        _MM_SET_ROUNDING_MODE(mode);
      }

      // -- rounding -- //


      // ++ alignment ++ //

      template<class T>
      inline int sse_is_16byte_aligned(const T *ptr) {
        return !(((uintptr_t)ptr) & 15);
      }

      template<class T>
      inline int sse_is_not_16byte_aligned(const T *ptr) {
        return (((uintptr_t)ptr) & 15);
      }

      template<class T>
      inline int sse_is_aligned(const T *ptr, const unsigned int bytes) {
        return !(((uintptr_t)ptr) & (bytes-1));
      }

      template<class T>
      inline int sse_is_not_aligned(const T *ptr, const unsigned int bytes) {
        return (((uintptr_t)ptr) & (bytes-1));
      }

      // -- alignment -- //


      // ++ conditions ++ //

      template<class T>
      inline T sse_if(const T &vIf, T v0) {
        v0 &= vIf;
        return v0;
      }

      template<class T>
      inline T sse_ifelse(const T &vIf, T v0, T &v1) {
        v0 &= vIf;
        v0 += andnot(v1, vIf);
        return v0;
      }

      // -- conditions -- //


      // ++ for-loops ++ //

      // the sse_for functions can be implemented compact in only one function
      // using pointer-to-pointer, but it is slower than the current
      // implementation of many versions

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, D*),
                          void (*subSSEMethod)(const S*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
        }

        for (; dst0<dstEnd; ++src0, ++dst0) {
          // convert 1 value
          (*subMethod)(src0, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
            dst1 += step;
        }

        for (; dst0<dstEnd; ++src0, ++dst0, ++dst1) {
          // convert 1 value
          (*subMethod)(src0, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
        }

        for (; dst0<dstEnd; ++src0, ++dst0, ++dst1, ++dst2) {
          // convert 1 value
          (*subMethod)(src0, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dst3, D *dstEnd,
                          void (*subMethod)(const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
        }

        for (; dst0<dstEnd; ++src0, ++dst0, ++dst1, ++dst2, ++dst3) {
          // convert 1 value
          (*subMethod)(src0, dst0, dst1, dst2, dst3);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            dst0 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++dst0) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            dst0 += step;
            dst1 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++dst0, ++dst1) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++dst0, ++dst1, ++dst2) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++dst0, ++dst1, ++dst2, ++dst3) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0, dst1, dst2, dst3);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++dst0) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
            dst1 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++dst0, ++dst1) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++dst0, ++dst1, ++dst2) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++dst0, ++dst1, ++dst2, ++dst3) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++src3, ++dst0) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
            dst1 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++src3, ++dst0, ++dst1) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++src3, ++dst0, ++dst1, ++dst2) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          long step) {
        D *dstSSEEnd = dstEnd - (step - 1);

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
        }

        for (; dst0<dstEnd; ++src0, ++src1, ++src2, ++src3, ++dst0, ++dst1, ++dst2, ++dst3) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);
        }
      }

      // the sse_for functions can be implemented compact in only one function
      // using pointer-to-pointer, but it is slower than the current
      // implementation of many versions

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, D*),
                          void (*subSSEMethod)(const S*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, dst0 += dStep) {
          // convert 1 value
          (*subMethod)(src0, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, dst0 += dStep, dst1 += dStep) {
          // convert 1 value
          (*subMethod)(src0, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
          // convert 1 value
          (*subMethod)(src0, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dst3, D *dstEnd,
                          void (*subMethod)(const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
          // convert 1 value
          (*subMethod)(src0, dst0, dst1, dst2, dst3);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            dst0 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, dst0 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, dst0 += dStep, dst1 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          void (*subMethod)(const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, dst0, dst1, dst2, dst3);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep, dst1 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep, dst1 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0, dst1);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2);
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstSSEEnd = dstEnd - (dstStep - 1);
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstSSEEnd;) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
        }

        for (; dst0<dstEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
          // convert 1 value
          (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);
        }
      }

      // -- for-loops -- //


      // ++ for-loops with ROI ++ //

      // the sse_for functions can be implemented compact in only one function
      // using pointer-to-pointer, but it is slower than the current
      // implementation of many versions

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*),
                          void (*subSSEMethod)(const S*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++dst0) {
              // convert 1 value
              (*subMethod)(src0, dst0);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
            dst1 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++dst0, ++dst1) {
              // convert 1 value
              (*subMethod)(src0, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++dst0, ++dst1, ++dst2) {
              // convert 1 value
              (*subMethod)(src0, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++dst0, ++dst1, ++dst2, ++dst3) {
              // convert 1 value
              (*subMethod)(src0, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            dst0 += step;
            dst1 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++dst0, ++dst1) {
              // convert 1 value
              (*subMethod)(src0, src1, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++dst0, ++dst1, ++dst2) {
              // convert 1 value
              (*subMethod)(src0, src1, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D *dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++dst0, ++dst1, ++dst2, ++dst3) {
              // convert 1 value
              (*subMethod)(src0, src1, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++dst0) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
            dst1 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++dst0, ++dst1) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++dst0, ++dst1, ++dst2) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++dst0, ++dst1, ++dst2, ++dst3) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++src3, ++dst0) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
            dst1 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++src3, ++dst0, ++dst1) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++src3, ++dst0, ++dst1, ++dst2) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          long step) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (step - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += step;
            src1 += step;
            src2 += step;
            src3 += step;
            dst0 += step;
            dst1 += step;
            dst2 += step;
            dst3 += step;
          } else {
            for (; dst0<dstLEnd; ++src0, ++src1, ++src2, ++src3, ++dst0, ++dst1, ++dst2, ++dst3) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      // the sse_for functions can be implemented compact in only one function
      // using pointer-to-pointer, but it is slower than the current
      // implementation of many versions

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*),
                          void (*subSSEMethod)(const S*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, dst0 += dStep) {
              // convert 1 value
              (*subMethod)(src0, dst0);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, dst0 += dStep, dst1 += dStep) {
              // convert 1 value
              (*subMethod)(src0, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
              // convert 1 value
              (*subMethod)(src0, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0,
                          D *dst0, D *dst1, D *dst2, D *dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
              // convert 1 value
              (*subMethod)(src0, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, dst0 += dStep, dst1 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1,
                          D *dst0, D *dst1, D *dst2, D *dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep, dst1 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep, dst1 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0, dst1);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
          }
        }
      }

      template<class S, class D>
      inline void sse_for(const S *src0, const S *src1, const S *src2, const S *src3,
                          D *dst0, D *dst1, D *dst2, D* dst3, D *dstEnd,
                          long srcWidth, long dstWidth, long lineWidth,
                          void (*subMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          void (*subSSEMethod)(const S*, const S*, const S*, const S*, D*, D*, D*, D*),
                          long srcStep, long dstStep) {
        D *dstLEnd   = dst0 + lineWidth;
        D *dstSSEEnd = dstLEnd - (dstStep - 1);
        long srcOffset = srcWidth - lineWidth;
        long dstOffset = dstWidth - lineWidth;
        long sStep, dStep;

        if (srcStep < dstStep) {
          dStep = dstStep / srcStep;
          sStep = 1;
        } else {
          sStep = srcStep / dstStep;
          dStep = 1;
        }

        for (; dst0<dstEnd;) {
          if (dst0<dstSSEEnd) {
            // convert 'rvalues' values at the same time
            (*subSSEMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);

            // increment pointers to the next values
            src0 += srcStep;
            src1 += srcStep;
            src2 += srcStep;
            src3 += srcStep;
            dst0 += dstStep;
            dst1 += dstStep;
            dst2 += dstStep;
            dst3 += dstStep;
          } else {
            for (; dst0<dstLEnd; src0 += sStep, src1 += sStep, src2 += sStep, src3 += sStep, dst0 += dStep, dst1 += dStep, dst2 += dStep, dst3 += dStep) {
              // convert 1 value
              (*subMethod)(src0, src1, src2, src3, dst0, dst1, dst2, dst3);
            }

            // move all pointers to the next line
            dstLEnd   += dstWidth;
            dstSSEEnd += dstWidth;
            src0 += srcOffset;
            src1 += srcOffset;
            src2 += srcOffset;
            src3 += srcOffset;
            dst0 += dstOffset;
            dst1 += dstOffset;
            dst2 += dstOffset;
            dst3 += dstOffset;
          }
        }
      }

      // -- for-loops with ROI -- //

    #endif

  } // namespace utils
}
