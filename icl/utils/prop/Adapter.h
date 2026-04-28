// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/plugin/PluginRegistry.h>

#include <any>
#include <functional>
#include <string>
#include <typeindex>

namespace icl::utils::prop {

  /// Per-constraint adapter that bridges structured constraints to the
  /// existing string-backed property world.
  ///
  /// Each concrete constraint type (`Range<T>`, `Menu<T>`, `Flag`, ...)
  /// enrolls one adapter at static-init time via
  /// `REGISTER_PROPERTY_CONSTRAINT`.  `Configurable` and `qt::Prop` look
  /// up the adapter by `std::type_index(typeid(Constraint))` and use its
  /// function members to:
  ///
  ///   - serialize the stored typed value to XML text (ConfigFile save);
  ///   - parse XML text back into a typed `std::any` (ConfigFile load);
  ///   - synthesize the legacy `getPropertyType()` tag (e.g.
  ///     `"range:slider"`, `"menu"`) during the transition period;
  ///   - synthesize the legacy `getPropertyInfo()` string (e.g.
  ///     `"[0,500]:0"`, `",fast,slow,auto"`).
  ///
  /// The `typeId` / `infoString` fields exist only for ConfigFile on-disk
  /// compatibility and for the deprecation path of
  /// `getPropertyType`/`getPropertyInfo`.  They are expected to retire
  /// together with those two virtuals.
  struct ConstraintAdapter {
    /// Stringify the stored typed value (for ConfigFile save).
    /// Input `value` is the typed `std::any` held by `Property::value`
    /// (content type matches `Constraint::value_type`).
    std::function<std::string(const std::any &value)> toString;

    /// Parse XML text into a typed `std::any` matching the constraint's
    /// `value_type`.  The `constraint` payload is passed in so adapters
    /// that need constraint data (e.g. a `Menu<T>`'s choice list when
    /// validating) have access to it.
    std::function<std::any(const std::any &constraint,
                           const std::string &text)> fromString;

    /// Legacy type tag (e.g. `"range:slider"`, `"menu"`, `"flag"`) used
    /// by `Configurable::getPropertyType()` and the XML `type="..."`
    /// attribute.  Passing the constraint in lets `Range<T>` return
    /// different tags for Slider vs Spinbox UIs.
    std::function<std::string(const std::any &constraint)> typeId;

    /// Legacy info string (e.g. `"[0,500]:0"`, `",fast,slow,auto"`) used
    /// by `Configurable::getPropertyInfo()`.  Empty for kinds that have
    /// no info (Flag, Command).
    std::function<std::string(const std::any &constraint)> infoString;
  };

  /// Registry keyed by `std::type_index` of the constraint type.
  ///
  /// Built on `PluginRegistry` with `OnDuplicate::KeepFirst` — each
  /// constraint type should self-register exactly once at static-init
  /// time.  First-wins on duplicate registration keeps behaviour
  /// deterministic across dyld load orderings.
  using ConstraintRegistry =
      PluginRegistry<std::type_index, ConstraintAdapter>;

  /// Global adapter registry.  Populated at static-init time by
  /// `REGISTER_PROPERTY_CONSTRAINT` macros.
  ICLUtils_API ConstraintRegistry &constraintRegistry();

  /// Look up the adapter for a constraint type.  Throws `ICLException`
  /// if no adapter is registered.
  ICLUtils_API const ConstraintAdapter &lookupAdapter(std::type_index t);

  /// Self-registration macro for constraint adapters.
  ///
  /// Usage (in a .cpp):
  /// ```
  ///   REGISTER_PROPERTY_CONSTRAINT(Flag, makeFlagAdapter());
  ///   REGISTER_PROPERTY_CONSTRAINT(Range<int>, makeRangeAdapter<int>());
  /// ```
  ///
  /// Emits a `static __attribute__((constructor, used))` free function.
  /// `static` gives internal linkage — no cross-TU symbol collisions for
  /// lines that happen to match.  `constructor` runs the registration at
  /// dyld time; `used` blocks dead-stripping.  This is the pattern
  /// CLAUDE.md identifies as the only macOS-portable option; anonymous-
  /// namespace static-storage registrars can be silently stripped.
  ///
  /// `Type` may be any C++ type including template instantiations with
  /// commas (`Range<int>`, `Menu<std::string>`); `typeid` + `#Type`
  /// avoid the need for a separate C-identifier tag.
  #define ICL_PROP_CONSTRAINT_CONCAT2(a, b) a##b
  #define ICL_PROP_CONSTRAINT_CONCAT(a, b) ICL_PROP_CONSTRAINT_CONCAT2(a, b)

  #define REGISTER_PROPERTY_CONSTRAINT(Type, ADAPTER_EXPR)                  \
    static __attribute__((constructor, used)) void                          \
    ICL_PROP_CONSTRAINT_CONCAT(_iclPropConstraintReg_, __LINE__)() {        \
      ::icl::utils::prop::constraintRegistry().registerPlugin(              \
          std::type_index(typeid(Type)), (ADAPTER_EXPR), #Type);            \
    }

} // namespace icl::utils::prop
