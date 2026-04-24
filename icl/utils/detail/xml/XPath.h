// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

/// \file
/// XPath subset used by `icl::utils::xml::Element::selectAll` and
/// friends.  See `xml-config-plan.md` Phase 7 for the supported
/// grammar.  This header exposes the AST types and the evaluator
/// entry point — parsing is done in XPath.cpp.

#pragma once

#include <icl/utils/Xml.h>

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace icl::utils::xml::detail {

  // ---------------------------------------------------------------
  // AST
  // ---------------------------------------------------------------

  enum class Axis  : std::uint8_t { Child, Descendant, Attribute, Self };
  enum class NameTestKind : std::uint8_t { Name, Star };

  struct NameTest {
    NameTestKind kind = NameTestKind::Name;
    std::string  name;  // valid if kind == Name
  };

  struct Predicate;
  using  PredicatePtr = std::unique_ptr<Predicate>;

  struct SelfIs    { std::string name; };
  struct AttrEq    { std::string name; std::string value; };
  struct AttrExists{ std::string name; };
  struct Index     { std::size_t pos; };      // 1-based XPath index
  struct BoolOr    { PredicatePtr lhs, rhs; };
  struct BoolAnd   { PredicatePtr lhs, rhs; };

  struct Predicate {
    std::variant<SelfIs, AttrEq, AttrExists, Index, BoolOr, BoolAnd> node;
  };

  struct Step {
    Axis     axis = Axis::Child;
    NameTest test;
    std::vector<PredicatePtr> predicates;
  };

  struct Path {
    bool              absolute = false;   // leading '/'
    std::vector<Step> steps;
  };

  // ---------------------------------------------------------------
  // Entry points (defined in XPath.cpp)
  // ---------------------------------------------------------------

  /// Parse an XPath expression.  Throws `xml::ParseError` on
  /// malformed input.
  ICLUtils_API Path parseXPath(std::string_view src);

  /// Evaluate `path` starting at `start` (within `doc`).  The result
  /// is the set of matched elements in document order.
  ICLUtils_API std::vector<Element> evaluateXPath(const Path &path,
                                                   Element start);

  /// Same, but evaluate a path whose final step is an attribute-axis
  /// step (`@name`) and return the first matched attribute.  Returns
  /// empty on no match or if the path is non-terminal-attribute.
  ICLUtils_API Attribute evaluateXPathAttr(const Path &path,
                                            Element start);

}  // namespace icl::utils::xml::detail
