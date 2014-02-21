/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/PThreadFix.h                     **
** Module : ICLUtils                                               **
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


#pragma once

#if 0
#if   defined(ICL_HAVE_IPP) || defined(ICL_HAVE_MKL)
#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Timer.h>

namespace icl{
  namespace utils{
    static icl::Timer ICL_STATIC_FIX_TO_AVOID_UNDEFINED_REFERENCE_TO_PTHREAD_ATFORK;
    struct ICLUtils_API ICL_PThreadAtForkFix{
      ICL_PThreadAtForkFix();
      static ICL_PThreadAtForkFix fix;
    };
  } // namespace utils
}

// for some reason, this symbol is missing, and we cannot
// tell the linker to find it ??
#define EXPLICITLY_INSTANTIATE_PTHREAD_AT_FORK                  \
  extern "C"{                                                   \
    extern int pthread_atfork(void (*)(void), void (*)(void),   \
                              void (*)(void)){                  \
    }                                                           \
  }
#else
#define EXPLICITLY_INSTANTIATE_PTHREAD_AT_FORK
#endif

#endif

