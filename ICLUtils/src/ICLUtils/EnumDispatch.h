/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/EnumDispatch.h                   **
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

#include <type_traits>

namespace icl {
  namespace utils {

    /// Dispatch a runtime enum/int value to a compile-time template parameter.
    ///
    /// Calls f(std::integral_constant<E, V>{}) for the matching value.
    /// Use decltype(tag)::value inside f to get the constexpr value.
    ///
    /// Example:
    ///   dispatchEnum<Op::add, Op::sub, Op::mul>(optype, [&](auto tag) {
    ///       applyTyped<decltype(tag)::value>(src, dst);
    ///   });
    template<auto... Values, class F>
    void dispatchEnum(int runtime, F&& f) {
      ((static_cast<int>(Values) == runtime
        ? (f(std::integral_constant<decltype(Values), Values>{}), true)
        : false) || ...);
    }

  } // namespace utils
} // namespace icl
