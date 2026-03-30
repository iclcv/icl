/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImageBackendDispatching.h          **
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

#include <ICLUtils/BackendDispatching.h>
#include <ICLCore/Image.h>

namespace icl {
  namespace core {

    // Re-export general types into core namespace
    using utils::Backend;
    using utils::backendName;

    /// Image backend dispatching — typedef to BackendDispatching<Image>.
    /// Used by filter operations (UnaryOp, BinaryOp) that work with Image values.
    using ImageBackendDispatching = utils::BackendDispatching<Image>;

    /// ImgBase* backend dispatching — for Img<T> member functions that
    /// dispatch on `this`. Avoids the overhead of constructing an Image wrapper.
    using ImgBaseBackendDispatching = utils::BackendDispatching<ImgBase*>;

    /// Predefined applicability for Image context.
    template<class... Ts>
    bool applicableTo(const Image& src) {
      depth d = src.getDepth();
      return ((d == getDepth<Ts>()) || ...);
    }

    /// Predefined applicability for ImgBase* context.
    template<class... Ts>
    bool applicableToBase(ImgBase* const& p) {
      depth d = p->getDepth();
      return ((d == getDepth<Ts>()) || ...);
    }

  } // namespace core
} // namespace icl
