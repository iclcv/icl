// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>

#include <initializer_list>
#include <string>
#include <utility>
#include <variant>
#include <vector>

/// Structured property-constraint types consumed by Configurable / qt::Prop.
///
/// Replaces the legacy stringly-typed `addProperty(name, type, info, value)`
/// grammar (`"range:slider"` + `"[0,255]:1"` etc.) with first-class C++
/// types that carry both storage type (`value_type`) and UI intent.
/// See TODO.md "Next: Configurable internal storage → typed std::any"
/// and the Session 53 arc for the full migration.
namespace icl::utils::prop {

  /// Which widget a numeric Range renders as.
  enum class UI {
    Slider,   //!< continuous slider (the 98% case)
    Spinbox,  //!< integer-step spinbox
  };

  /// Numeric range with min/max and optional step + widget style.
  ///
  /// The aggregate's member types drive CTAD, so call sites can write
  /// `Range{.min=0.f, .max=500.f}` (→ Range<float>) or
  /// `Range{.min=0, .max=100}` (→ Range<int>) without spelling the
  /// template argument.  Mixed literal types (e.g. `.min=0, .max=1.f`)
  /// fail to deduce — author must pick consistent literal types.
  ///
  /// `step == 0` means continuous.  `ui == Slider` is the default and
  /// fits ~98% of existing call sites.
  template<class T>
  struct Range {
    using value_type = T;
    T  min;
    T  max;
    T  step{};                //!< 0 = continuous
    UI ui = UI::Slider;
  };

  /// Categorical pick-one-from-list.
  ///
  /// Templated on value type so menus can hold ints/floats/strings
  /// uniformly.  Replaces both the old `"menu"` (string-only) and
  /// `"value-list"` (numeric-only) property types.
  ///
  /// Deduction guides let `Menu{"a","b","c"}` deduce `Menu<std::string>`
  /// from a brace-init-list of string literals.
  template<class T>
  struct Menu {
    using value_type = T;
    std::vector<T> choices;

    Menu() = default;
    Menu(std::initializer_list<T> xs) : choices(xs) {}
    explicit Menu(std::vector<T> xs)  : choices(std::move(xs)) {}
  };

  /// Deduction guide: `Menu{"a","b","c"}` → `Menu<std::string>` instead of
  /// the ill-behaved `Menu<const char*>`.
  Menu(std::initializer_list<const char*>) -> Menu<std::string>;

  /// Build a `Menu<std::string>` from a comma-separated runtime string.
  ///
  /// Used to migrate legacy call sites where the menu's choice list was
  /// a named constant (`const char* ARITH_MENU = "add,sub,mul,div"`) or
  /// computed at runtime (grabber enumerating supported formats):
  ///
  /// ```
  ///   addProperty("op", prop::menuFromCsv(ARITH_MENU), arithName(t));
  ///   addProperty("format", prop::menuFromCsv(grabber.getFormats()), cur);
  /// ```
  ///
  /// Empty tokens (leading / trailing / doubled separators) are dropped;
  /// whitespace around each token is trimmed.
  inline Menu<std::string> menuFromCsv(std::string_view csv, char sep = ',') {
    std::vector<std::string> v;
    auto is_space = [](unsigned char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; };
    std::size_t start = 0;
    for (std::size_t i = 0; i <= csv.size(); ++i) {
      if (i == csv.size() || csv[i] == sep) {
        std::string piece(csv.substr(start, i - start));
        while (!piece.empty() && is_space(static_cast<unsigned char>(piece.front()))) piece.erase(0, 1);
        while (!piece.empty() && is_space(static_cast<unsigned char>(piece.back())))  piece.pop_back();
        if (!piece.empty()) v.push_back(std::move(piece));
        start = i + 1;
      }
    }
    return Menu<std::string>(std::move(v));
  }

  /// Boolean flag (checkbox).  Stored value is `bool`.
  struct Flag { using value_type = bool; };

  /// Button-style property with no value.  Setting it fires callbacks
  /// but stores nothing.  Stored value is `std::monostate`.
  struct Command { using value_type = std::monostate; };

  /// Read-only display string.  The value is settable by the owner but
  /// the UI does not offer an edit control.  Volatileness (the
  /// `addProperty` timer arg) works exactly as for any other kind —
  /// there is no separate refresh knob.
  struct Info { using value_type = std::string; };

  /// Free-form string with optional maximum length (0 = unlimited).
  struct Text {
    using value_type = std::string;
    int maxLength = 0;
  };

  /// Catch-all constraint for legacy / dynamic-registration properties
  /// whose type + info strings don't map to any of the structured
  /// `prop::*` / `core::prop::*` kinds.
  ///
  /// Populated by `Configurable::buildConstraintFromLegacy` as a
  /// fallback when the (type, info) pair is unrecognised or malformed
  /// (e.g. Pylon/OpenNI pass hardware-introspected strings that ICL
  /// doesn't know about).  Stored value is `std::string` — the raw
  /// legacy value survives unchanged.
  ///
  /// Guarantees that every `Property::constraint` is populated: the
  /// `getPropertyType` / `getPropertyInfo` synthesizers can lookup the
  /// Generic adapter which returns the cached strings verbatim, so
  /// external Configurable introspection (icl-configurable-info etc.)
  /// still reports the original tags.
  ///
  /// qt::Prop doesn't render Generic — the dispatch chain falls through
  /// to an ERROR_LOG.  That matches pre-session behaviour for unknown
  /// types.
  struct Generic {
    using value_type = std::string;
    std::string type_string;
    std::string info_string;
  };

} // namespace icl::utils::prop
