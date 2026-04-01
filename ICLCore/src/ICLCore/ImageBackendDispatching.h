// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

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
