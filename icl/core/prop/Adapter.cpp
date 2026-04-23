// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/prop/Constraints.h>
#include <icl/utils/prop/Adapter.h>
#include <icl/utils/StringUtils.h>

#include <any>
#include <string>

namespace icl::core::prop {

  namespace {

    // ------------------------------------------------------------------
    // Color — RGB triplet.  str()/parse() go through FixedMatrix's
    // stream operators (which wrap DynMatrix's formatting).
    // ------------------------------------------------------------------

    utils::prop::ConstraintAdapter makeColorAdapter() {
      return {
        [](const std::any &v) {
          return utils::str(std::any_cast<core::Color>(v));
        },
        [](const std::any &, const std::string &s) {
          return std::any(utils::parse<core::Color>(s));
        },
        [](const std::any &) -> std::string { return "color"; },
        [](const std::any &) -> std::string { return ""; },
      };
    }

    // ------------------------------------------------------------------
    // ImageView — not serializable.
    //
    // toString returns empty and fromString returns an empty Image
    // rather than throwing, so that per-property iteration (e.g.
    // icl-configurable-info, saveProperties) doesn't need a special-case
    // guard — an image row contributes nothing and is indistinguishable
    // from a missing entry.  Consumers that specifically need the live
    // image read `Property::typed_value` directly via `any_cast`.
    // ------------------------------------------------------------------

    utils::prop::ConstraintAdapter makeImageViewAdapter() {
      return {
        [](const std::any &) -> std::string { return ""; },
        [](const std::any &, const std::string &) {
          return std::any(core::Image{});
        },
        [](const std::any &) -> std::string { return "image"; },
        [](const std::any &) -> std::string { return ""; },
      };
    }

  } // anonymous

  REGISTER_PROPERTY_CONSTRAINT(Color,     makeColorAdapter());
  REGISTER_PROPERTY_CONSTRAINT(ImageView, makeImageViewAdapter());

} // namespace icl::core::prop
