// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/prop/Adapter.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>

#include <sstream>
#include <string>
#include <variant>

namespace icl::utils::prop {

  ConstraintRegistry &constraintRegistry() {
    // Meyers singleton.  `KeepFirst` so duplicate static-init enrollments
    // (e.g. the same adapter dragged in from two TUs) silently no-op
    // rather than throwing — matches the other ICL plugin registries.
    static ConstraintRegistry reg(OnDuplicate::KeepFirst);
    return reg;
  }

  const ConstraintAdapter &lookupAdapter(std::type_index t) {
    return constraintRegistry().getOrThrow(t).payload;
  }

  namespace {

    // ------------------------------------------------------------------
    // Range<T>
    // ------------------------------------------------------------------

    template<class T>
    ConstraintAdapter makeRangeAdapter() {
      return {
        // toString
        [](const std::any &value) {
          return str(std::any_cast<T>(value));
        },
        // fromString
        [](const std::any & /*constraint*/, const std::string &text) {
          return std::any(parse<T>(text));
        },
        // typeId — Slider or Spinbox → legacy XML tags
        [](const std::any &constraint) -> std::string {
          const auto &r = std::any_cast<const Range<T> &>(constraint);
          return r.ui == UI::Spinbox ? "range:spinbox" : "range:slider";
        },
        // infoString — "[min,max]" or "[min,max]:step"
        [](const std::any &constraint) -> std::string {
          const auto &r = std::any_cast<const Range<T> &>(constraint);
          std::ostringstream s;
          s << '[' << r.min << ',' << r.max << ']';
          if (r.step != T{}) s << ':' << r.step;
          return s.str();
        },
      };
    }

    // ------------------------------------------------------------------
    // Menu<T>
    //
    // XML compat: legacy "menu" is string-only; "value-list" is the
    // numeric variant.  We surface both as "menu" — the on-disk tag is
    // driven by the constraint type, not storage type, and qt::Prop's
    // renderer dispatches on the C++ constraint type anyway.  If a
    // downstream tool specifically relies on the "value-list" tag
    // we'll re-introduce it at that point.
    // ------------------------------------------------------------------

    template<class T>
    ConstraintAdapter makeMenuAdapter() {
      return {
        [](const std::any &value) {
          return str(std::any_cast<T>(value));
        },
        [](const std::any & /*constraint*/, const std::string &text) {
          return std::any(parse<T>(text));
        },
        [](const std::any & /*constraint*/) -> std::string {
          return "menu";
        },
        // Info string: legacy format is leading-comma + delim-joined
        // choices, e.g. ",fast,slow,auto".
        [](const std::any &constraint) -> std::string {
          const auto &m = std::any_cast<const Menu<T> &>(constraint);
          std::string out;
          for (const auto &c : m.choices) {
            out += ',';
            out += str(c);
          }
          return out;
        },
      };
    }

    // ------------------------------------------------------------------
    // Flag
    //
    // XML compat: values serialized as "on"/"off" (parse<bool> accepts
    // "on"/"off"/"true"/"false"/"0"/"1" all the same on load).
    // ------------------------------------------------------------------

    ConstraintAdapter makeFlagAdapter() {
      return {
        [](const std::any &value) -> std::string {
          return std::any_cast<bool>(value) ? "on" : "off";
        },
        [](const std::any & /*constraint*/, const std::string &text) {
          return std::any(parse<bool>(text));
        },
        [](const std::any &) -> std::string { return "flag"; },
        [](const std::any &) -> std::string { return ""; },
      };
    }

    // ------------------------------------------------------------------
    // Command
    //
    // Button-like; has no storable value.  toString returns empty,
    // fromString returns a monostate any.  Callers should never write a
    // Command property to ConfigFile (saveProperties already skips it
    // today), but the adapter still exists for uniformity.
    // ------------------------------------------------------------------

    ConstraintAdapter makeCommandAdapter() {
      return {
        [](const std::any &) -> std::string { return ""; },
        [](const std::any &, const std::string &) {
          return std::any(std::monostate{});
        },
        [](const std::any &) -> std::string { return "command"; },
        [](const std::any &) -> std::string { return ""; },
      };
    }

    // ------------------------------------------------------------------
    // Info
    //
    // Read-only display string.  Same storage as Text, but semantically
    // not editable by the GUI — that distinction lives in the renderer.
    // ------------------------------------------------------------------

    ConstraintAdapter makeInfoAdapter() {
      return {
        [](const std::any &value) {
          return std::any_cast<std::string>(value);
        },
        [](const std::any &, const std::string &text) {
          return std::any(text);
        },
        [](const std::any &) -> std::string { return "info"; },
        [](const std::any &) -> std::string { return ""; },
      };
    }

    // ------------------------------------------------------------------
    // Text
    //
    // XML compat: type="string", info=<maxLength as text>.
    // ------------------------------------------------------------------

    ConstraintAdapter makeTextAdapter() {
      return {
        [](const std::any &value) {
          return std::any_cast<std::string>(value);
        },
        [](const std::any &, const std::string &text) {
          return std::any(text);
        },
        [](const std::any &) -> std::string { return "string"; },
        [](const std::any &constraint) -> std::string {
          const auto &t = std::any_cast<const Text &>(constraint);
          return str(t.maxLength);
        },
      };
    }

  } // anonymous namespace

  // Static-init enrollments for the utils-side constraint kinds.
  // Additional kinds (core::prop::Color, core::prop::ImageView, ...)
  // enroll themselves in their owning module's TU.

  REGISTER_PROPERTY_CONSTRAINT(Range<int>,          makeRangeAdapter<int>());
  REGISTER_PROPERTY_CONSTRAINT(Range<float>,        makeRangeAdapter<float>());

  REGISTER_PROPERTY_CONSTRAINT(Menu<int>,           makeMenuAdapter<int>());
  REGISTER_PROPERTY_CONSTRAINT(Menu<float>,         makeMenuAdapter<float>());
  REGISTER_PROPERTY_CONSTRAINT(Menu<std::string>,   makeMenuAdapter<std::string>());

  REGISTER_PROPERTY_CONSTRAINT(Flag,                makeFlagAdapter());
  REGISTER_PROPERTY_CONSTRAINT(Command,             makeCommandAdapter());
  REGISTER_PROPERTY_CONSTRAINT(Info,                makeInfoAdapter());
  REGISTER_PROPERTY_CONSTRAINT(Text,                makeTextAdapter());

} // namespace icl::utils::prop
