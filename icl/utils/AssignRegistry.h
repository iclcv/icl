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
    /// Meyers singleton.  Safe to call at any time after static init.
    static AssignRegistry &instance();

    /// Enroll a `Dst = Src` pair for runtime dispatch.  Requires that
    /// `dst = src` is well-formed (checked via `static_assert`).
    /// Overwrites any prior entry for the pair.
    template<typename Dst, typename Src>
    void enroll() {
      static_assert(is_assignable_v<Dst, Src>,
                    "AssignRegistry::enroll<Dst, Src>: `dst = src` is not "
                    "well-formed — give the class an operator= or a "
                    "conversion operator for this pair first");
      m_map[std::type_index(typeid(Dst))]
           [std::type_index(typeid(Src))] =
        +[](std::any &dst, std::any &src) {
          Assign<Dst, Src>::apply(std::any_cast<Dst &>(dst),
                                  std::any_cast<Src &>(src));
        };
    }

    /// Dispatch: invoke the registered rule for `(dst.type(), src.type())`.
    /// @throws std::runtime_error if no rule is registered for the pair.
    /// @throws std::bad_any_cast  if `dst`/`src` somehow hold different
    ///                            types than their `type_index` suggests
    ///                            (shouldn't happen in normal use).
    void dispatch(std::any &dst, std::any &src) const;

    /// True iff a rule is registered for `Dst = Src`.
    bool has(std::type_index dstType, std::type_index srcType) const noexcept;

    /// Total number of registered `(Dst, Src)` pairs.
    std::size_t size() const noexcept;

  private:
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
