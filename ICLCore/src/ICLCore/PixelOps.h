/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/PixelOps.h                         **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>

namespace icl{
  namespace core{

    /// copies data from source to destination array (memcpy) \ingroup GENERAL
    /** Explicitly instantiated for all 5 ICL depth types.
        For performance-critical inner loops, consider using memcpy directly. */
    template<class T>
    ICLCore_API void copy(const T *src, const T *srcEnd, T *dst);

    /// converts data from source to destination array with type casting \ingroup GENERAL
    /** Explicitly instantiated for all 25 source/destination type pairs.
        IPP-optimized or SSE2-optimized specializations are used where available. */
    template<class srcT, class dstT>
    ICLCore_API void convert(const srcT *poSrcStart, const srcT *poSrcEnd, dstT *poDst);

  } // namespace core
}
