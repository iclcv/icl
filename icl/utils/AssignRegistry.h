// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Assign.h>

#include <any>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace icl::utils {

  /// Runtime, type-erased dispatcher for `dst = src` assignments when
  /// one or both sides are only known at runtime (string-keyed stores,
  /// scripting bridges, config loaders).
  ///
  /// For each `(Dst, Src)` pair enrolled at startup, a lambda is stored
  /// that unwraps two `std::any` payloads back to the concrete types and
  /// invokes `Assign<Dst, Src>::apply` — which itself just calls
  /// `dst = src` via the class's `operator=` / conversion operators.
  ///
  /// Pairs must be enrolled explicitly.  The `static_assert` inside
  /// `enroll<Dst, Src>()` fails to compile if no `operator=` / conversion
  /// chain exists, keeping runtime dispatch in sync with the compile-time
  /// trait by construction.
  ///
  /// Template parameter order is `<Dst, Src>` throughout — matching the
  /// direction of `dst = src`.
  class ICLUtils_API AssignRegistry {
  public:
    /// Enroll a `Dst = Src` pair for runtime dispatch.  Requires that
    /// `dst = src` is well-formed (checked via `static_assert`).
    /// Overwrites any prior entry for the pair.
    template<typename Dst, typename Src>
    static void enroll() {
      static_assert(is_assignable_v<Dst, Src>,
                    "AssignRegistry::enroll<Dst, Src>: `dst = src` is not "
                    "well-formed — give the class an operator= or a "
                    "conversion operator for this pair first");
      instance().m_map
        [std::type_index(typeid(Dst))]
        [std::type_index(typeid(Src))] =
        +[](std::any &dst, std::any &src) {
          Assign<Dst, Src>::apply(std::any_cast<Dst &>(dst),
                                  std::any_cast<Src &>(src));
        };
    }

    /// Enroll `A = B` AND `B = A` for each `B` in one call.  Both
    /// directions must be assignable for every `B` (each side's
    /// `static_assert` fires independently).
    ///
    /// Example:
    /// ```
    ///   AssignRegistry::enroll_symmetric<SliderHandle, int, float, double, std::string>();
    /// ```
    /// enrolls 8 entries (4 × 2 directions) in a single line.
    template<typename A, typename... Bs>
    static void enroll_symmetric() {
      (enroll<A, Bs>(), ...);   // A = B_i
      (enroll<Bs, A>(), ...);   // B_i = A
    }

    /// One-directional bulk enroll — `Dst = Src` for each Src.  For
    /// handles that accept multiple source types but have no readback
    /// (pure "receivers", e.g. LabelHandle).
    template<typename Dst, typename... Srcs>
    static void enroll_receiver() {
      (enroll<Dst, Srcs>(), ...);
    }

    /// One-directional bulk enroll — `Dst = Src` for each Dst.  For
    /// handles whose state can only be *read* (pure "providers",
    /// e.g. ButtonHandle, TabHandle).
    template<typename Src, typename... Dsts>
    static void enroll_provider() {
      (enroll<Dsts, Src>(), ...);
    }

    /// Dispatch: invoke the registered rule for `(dst.type(), src.type())`.
    /// @throws std::runtime_error if no rule is registered for the pair.
    /// @throws std::bad_any_cast  if `dst`/`src` somehow hold different
    ///                            types than their `type_index` suggests
    ///                            (shouldn't happen in normal use).
    static void dispatch(std::any &dst, std::any &src);

    /// True iff a rule is registered for `Dst = Src`.
    static bool has(std::type_index dstType, std::type_index srcType) noexcept;

    /// Total number of registered `(Dst, Src)` pairs.
    static std::size_t size() noexcept;

  private:
    /// Meyers singleton — holds the actual map state.  Private: all
    /// public API is static and routes through `instance()` internally,
    /// so callers never need to write `.instance().` anywhere.
    static AssignRegistry &instance();

    using Fn = void (*)(std::any &, std::any &);
    std::unordered_map<
      std::type_index,
      std::unordered_map<std::type_index, Fn>
    > m_map;  // keyed as m_map[Dst][Src]

    AssignRegistry() = default;
    AssignRegistry(const AssignRegistry &) = delete;
    AssignRegistry &operator=(const AssignRegistry &) = delete;
  };

}  // namespace icl::utils
