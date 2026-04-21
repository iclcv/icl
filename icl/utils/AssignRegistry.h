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
    ///
    /// Registers two parallel function pointers per pair — one for
    /// `std::any`-wrapped payloads, one for raw `void *` pointers.
    /// Callers pick the form that matches their storage: scripting
    /// bridges typically own the data inside an `std::any`; legacy
    /// stores (`DataStore`, `MultiTypeMap`) keep raw `void *` +
    /// RTTI-name strings.  Both variants reach the same
    /// `Assign<Dst, Src>::apply` call.
    template<typename Dst, typename Src>
    static void enroll() {
      static_assert(is_assignable_v<Dst, Src>,
                    "AssignRegistry::enroll<Dst, Src>: `dst = src` is not "
                    "well-formed — give the class an operator= or a "
                    "conversion operator for this pair first");
      auto &r = instance();
      std::type_index dstT(typeid(Dst));
      std::type_index srcT(typeid(Src));
      r.m_map[dstT][srcT] = Fn{
        +[](std::any &dst, std::any &src) {
          Assign<Dst, Src>::apply(std::any_cast<Dst &>(dst),
                                  std::any_cast<Src &>(src));
        },
        +[](void *dst, void *src) {
          Assign<Dst, Src>::apply(*static_cast<Dst *>(dst),
                                  *static_cast<Src *>(src));
        }
      };
      // Shadow lookup so string-keyed stores (DataStore) can translate
      // RTTI-name -> type_index at dispatch time.
      r.m_nameToType.insert({std::string(typeid(Dst).name()), dstT});
      r.m_nameToType.insert({std::string(typeid(Src).name()), srcT});
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

    /// Enroll `T = T` (identity) for each `T`.  Needed so that
    /// `DataStore`/`Slot`-proxy consumers who extract a stored value
    /// into a local of the same type (e.g. `SliderHandle s = gui["x"]`)
    /// find a rule via the runtime registry.
    template<typename... Ts>
    static void enroll_identity() {
      (enroll<Ts, Ts>(), ...);
    }

    /// Dispatch with `std::any`-wrapped payloads.  Thin wrapper over
    /// the typed function pointer stored at enrollment time.
    /// @throws std::runtime_error if no rule is registered for the pair.
    /// @throws std::bad_any_cast  if `dst`/`src` somehow hold different
    ///                            types than their `type_index` suggests.
    static void dispatch(std::any &dst, std::any &src);

    /// Dispatch with raw pointers and explicit type indices.  Used by
    /// `DataStore::Data::assign` and other legacy stores.
    /// @throws std::runtime_error if no rule is registered for the pair.
    static void dispatch(void *dst, std::type_index dstType,
                         void *src, std::type_index srcType);

    /// Dispatch with raw pointers and RTTI-name strings.  Looks up
    /// `type_index` via the name table populated at enrollment time.
    /// @throws std::runtime_error if either name is unknown or no rule
    ///         is registered for the pair.
    static void dispatch(void *dst, const std::string &dstName,
                         void *src, const std::string &srcName);

    /// True iff a rule is registered for `Dst = Src`.
    static bool has(std::type_index dstType, std::type_index srcType) noexcept;

    /// Total number of registered `(Dst, Src)` pairs.
    static std::size_t size() noexcept;

  private:
    /// Meyers singleton — holds the actual map state.  Private: all
    /// public API is static and routes through `instance()` internally,
    /// so callers never need to write `.instance().` anywhere.
    static AssignRegistry &instance();

    /// Per-pair function pointers.  Both reach the same
    /// `Assign<Dst, Src>::apply` call, differing only in how the
    /// payload is presented to them.
    struct Fn {
      void (*any)(std::any &, std::any &);
      void (*ptr)(void *, void *);
    };

    /// Shared lookup for all three dispatch entry points.  Throws on
    /// miss with a descriptive message.  Defined in the .cpp.
    static const Fn &lookup(std::type_index dstT, std::type_index srcT,
                            const char *dstName, const char *srcName);

    std::unordered_map<
      std::type_index,
      std::unordered_map<std::type_index, Fn>
    > m_map;  // keyed as m_map[Dst][Src]

    /// Shadow name table: RTTI name string -> `type_index`.  Populated
    /// alongside every `enroll<Dst, Src>()`.  Lets string-keyed stores
    /// (DataStore/MultiTypeMap) drive the registry without needing
    /// compile-time type info at the call site.
    std::unordered_map<std::string, std::type_index> m_nameToType;

    AssignRegistry() = default;
    AssignRegistry(const AssignRegistry &) = delete;
    AssignRegistry &operator=(const AssignRegistry &) = delete;
  };

}  // namespace icl::utils
