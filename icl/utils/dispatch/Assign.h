// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>

#include <type_traits>

namespace icl::utils {

  /// Compile-time trait + minimal executor for `dst = src`-style
  /// assignments across heterogeneous types.
  ///
  /// Accepts two forms, in priority order:
  ///
  /// 1. **Direct**: `dst = src` — works whenever an appropriate
  ///    `operator=` (or built-in assignment) exists.  Covers primitives
  ///    and classes with `operator=(Src)`.
  ///
  /// 2. **Extraction**: `dst = src.template as<Dst>()` — the fallback
  ///    used when `Src` does not implicitly convert to `Dst` but
  ///    provides an explicit `as<Dst>()` member template for the
  ///    target type.  This is how classes expose their readback
  ///    surface without adding implicit conversion operators (which
  ///    are a well-known source of overload-resolution foot-guns).
  ///
  /// `value` is `true` iff at least one of the two forms is
  /// well-formed.  The two `apply()` overloads have disjoint
  /// constraints — exactly one is selected per `(Dst, Src)`
  /// instantiation.
  ///
  /// Template parameter order matches `dst = src` and
  /// `std::is_assignable<Dst&, Src>`.
  ///
  /// Use-sites:
  /// - **Compile-time dispatch** (typed handles): users write
  ///   natural C++ — `slider = 42` (direct) or
  ///   `int v = slider.as<int>()` (explicit extraction).  This
  ///   trait is not involved.
  /// - **Runtime dispatch** (AssignRegistry): uses `apply()` to
  ///   bridge type-erased `std::any` payloads back to the typed
  ///   operators / extraction methods on the class.

  template<typename Dst, typename Src>
  concept DirectlyAssignable = std::is_assignable_v<Dst &, Src>;

  template<typename Dst, typename Src>
  concept ExtractableAs = requires(Dst &d, Src &s) {
    d = s.template as<Dst>();
  };

  template<typename Dst, typename Src>
  struct Assign : std::bool_constant<
      DirectlyAssignable<Dst, Src> || ExtractableAs<Dst, Src>>
  {
    /// Path 1 — direct assignment.  Preferred when available.
    static void apply(Dst &dst, Src &src)
      requires DirectlyAssignable<Dst, Src>
    {
      dst = src;
    }

    /// Path 2 — extraction fallback.  Selected only when direct
    /// assignment would not compile; delegates to `src.as<Dst>()`.
    static void apply(Dst &dst, Src &src)
      requires (!DirectlyAssignable<Dst, Src> && ExtractableAs<Dst, Src>)
    {
      dst = src.template as<Dst>();
    }
  };

  /// Convenience trait: true iff some `Assign<Dst, Src>::apply`
  /// overload is well-formed.
  template<typename Dst, typename Src>
  inline constexpr bool is_assignable_v = Assign<Dst, Src>::value;

}  // namespace icl::utils
