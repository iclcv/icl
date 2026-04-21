// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>

#include <type_traits>

namespace icl::utils {

  /// Compile-time trait and minimal executor for `dst = src` assignments
  /// across heterogeneous types.
  ///
  /// `Assign<Dst, Src>::value` is `true` iff the expression `dst = src`
  /// is well-formed — i.e., whenever `Dst` has an appropriate `operator=`
  /// overload, or `Src` has a conversion to something `Dst` accepts.
  /// The implementation defers to `std::is_assignable_v<Dst&, Src>`,
  /// which already captures the C++ overload / conversion rules.
  ///
  /// Template parameter order matches `dst = src` and
  /// `std::is_assignable<Dst&, Src>`.
  ///
  /// Use-sites:
  /// - **Compile-time dispatch** (typed handles, `slider = 42`) uses
  ///   plain C++ overload resolution — this trait is *not* involved.
  /// - **Runtime dispatch** (`AssignRegistry`, for string-keyed stores)
  ///   uses `apply()` as the bridge between type-erased `std::any`
  ///   payloads and the class's native `operator=`.
  ///
  /// Classes add support for a new source type simply by defining
  /// `operator=(const Src&)` (or an implicit conversion chain) and
  /// enrolling `Assign<Dst, Src>` in the registry.  No per-pair
  /// specializations of this template are needed, and none should
  /// normally be added.
  template<typename Dst, typename Src>
  struct Assign : std::bool_constant<std::is_assignable_v<Dst &, Src>> {
    static void apply(Dst &dst, Src &src)
      requires std::is_assignable_v<Dst &, Src>
    {
      dst = src;
    }
  };

  /// Convenience trait: `is_assignable_v<Dst, Src>` is `true` iff
  /// `dst = src` is well-formed.
  template<typename Dst, typename Src>
  inline constexpr bool is_assignable_v = Assign<Dst, Src>::value;

}  // namespace icl::utils
