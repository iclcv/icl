/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImgOps.h                           **
** Module : ICLCore                                                **
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

#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
  namespace core {

    /// Singleton that owns BackendSelectors for Img utility operations
    /// (mirror, min, max, lut, etc.). Uses ImgBase* as dispatch context
    /// so Img<T> methods can dispatch directly via `this`.
    ///
    /// Backends self-register from _Cpp.cpp / _Ipp.cpp / _Mkl.cpp files.
    class ICLCore_API ImgOps : public ImgBaseBackendDispatching {
    public:
      // ---- Dispatch signatures (ImgBase& + operation args) ----
      using MirrorSig = void(ImgBase&, axis, bool roiOnly);

      /// Access the singleton instance (lazy-init, thread-safe)
      static ImgOps& instance();

    private:
      ImgOps();
    };

  } // namespace core
} // namespace icl
