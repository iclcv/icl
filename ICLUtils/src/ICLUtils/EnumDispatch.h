// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

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
      (void)((static_cast<int>(Values) == runtime
        ? (f(std::integral_constant<decltype(Values), Values>{}), true)
        : false) || ...);
    }

  } // namespace utils
} // namespace icl
