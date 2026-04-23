// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"

#include <icl/utils/prop/Adapter.h>
#include <icl/utils/prop/Constraints.h>

#include <any>
#include <string>
#include <type_traits>
#include <typeindex>
#include <variant>

using namespace icl::utils;
using namespace icl::utils::prop;

// ---------------------------------------------------------------------------
// CTAD / aggregate deduction — compile-time property-shape checks
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.range.ctad_designated_float",
                  "Range{.min=0.f,.max=500.f} deduces Range<float>")
{
  auto r = Range{.min = 0.f, .max = 500.f};
  static_assert(std::is_same_v<decltype(r), Range<float>>);
  ICL_TEST_TRUE(r.ui == UI::Slider);   // default
  ICL_TEST_EQ(r.min, 0.f);
  ICL_TEST_EQ(r.max, 500.f);
  ICL_TEST_EQ(r.step, 0.f);
}

ICL_REGISTER_TEST("utils.prop.range.ctad_designated_int",
                  "Range{.min=0,.max=100,.ui=Spinbox} deduces Range<int>")
{
  auto r = Range{.min = 0, .max = 100, .ui = UI::Spinbox};
  static_assert(std::is_same_v<decltype(r), Range<int>>);
  ICL_TEST_TRUE(r.ui == UI::Spinbox);
  ICL_TEST_EQ(r.step, 0);
}

ICL_REGISTER_TEST("utils.prop.range.ctad_designated_with_step",
                  "Range{.min=1,.max=31,.step=2,.ui=Spinbox} deduces Range<int>")
{
  auto r = Range{.min = 1, .max = 31, .step = 2, .ui = UI::Spinbox};
  static_assert(std::is_same_v<decltype(r), Range<int>>);
  ICL_TEST_EQ(r.step, 2);
}

ICL_REGISTER_TEST("utils.prop.range.ctad_positional",
                  "Range{0.f, 500.f} also deduces via positional aggregate init")
{
  auto r = Range{0.f, 500.f};
  static_assert(std::is_same_v<decltype(r), Range<float>>);
  ICL_TEST_EQ(r.min, 0.f);
  ICL_TEST_EQ(r.max, 500.f);
}

ICL_REGISTER_TEST("utils.prop.range.positional", "positional aggregate init also works")
{
  Range<float> r{0.f, 1.f};
  ICL_TEST_EQ(r.min, 0.f);
  ICL_TEST_EQ(r.max, 1.f);
  ICL_TEST_EQ(r.step, 0.f);
  ICL_TEST_TRUE(r.ui == UI::Slider);
}

ICL_REGISTER_TEST("utils.prop.menu.ctad_string", "Menu{\"a\",\"b\"} deduces Menu<std::string>")
{
  auto m = Menu{"fast", "slow", "auto"};
  static_assert(std::is_same_v<decltype(m), Menu<std::string>>);
  ICL_TEST_EQ(m.choices.size(), 3u);
  ICL_TEST_EQ(m.choices[0], std::string("fast"));
}

ICL_REGISTER_TEST("utils.prop.menu.ctad_int", "Menu{1,2,4} deduces Menu<int>")
{
  auto m = Menu{1, 2, 4, 8};
  static_assert(std::is_same_v<decltype(m), Menu<int>>);
  ICL_TEST_EQ(m.choices.size(), 4u);
  ICL_TEST_EQ(m.choices[2], 4);
}

ICL_REGISTER_TEST("utils.prop.value_types", "value_type aliases expose storage types")
{
  static_assert(std::is_same_v<Range<float>::value_type, float>);
  static_assert(std::is_same_v<Range<int>::value_type,   int>);
  static_assert(std::is_same_v<Menu<std::string>::value_type, std::string>);
  static_assert(std::is_same_v<Flag::value_type,    bool>);
  static_assert(std::is_same_v<Command::value_type, std::monostate>);
  static_assert(std::is_same_v<Info::value_type,    std::string>);
  static_assert(std::is_same_v<Text::value_type,    std::string>);
}

// ---------------------------------------------------------------------------
// Adapter registry — static-init enrollments
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.registry.has_builtin_adapters",
                  "all 9 utils-side adapters are registered at static-init time")
{
  const auto &reg = constraintRegistry();
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Range<int>))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Range<float>))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Menu<int>))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Menu<float>))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Menu<std::string>))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Flag))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Command))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Info))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(Text))));
}

ICL_REGISTER_TEST("utils.prop.registry.lookup_throws_on_unknown",
                  "lookupAdapter throws for unregistered types")
{
  struct NotAConstraint {};
  bool threw = false;
  try {
    lookupAdapter(std::type_index(typeid(NotAConstraint)));
  } catch (...) {
    threw = true;
  }
  ICL_TEST_TRUE(threw);
}

// ---------------------------------------------------------------------------
// Range adapter — value round-trip + legacy typeId/infoString
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.adapter.range_float.round_trip",
                  "Range<float> value serializes and parses back through adapter")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Range<float>)));
  std::any constraint = Range{.min = 0.f, .max = 500.f};
  std::string s = a.toString(std::any(123.5f));
  std::any back = a.fromString(constraint, s);
  ICL_TEST_NEAR(std::any_cast<float>(back), 123.5f, 1e-5f);
}

ICL_REGISTER_TEST("utils.prop.adapter.range_float.type_and_info",
                  "Range<float> typeId reflects UI; infoString encodes [min,max]:step")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Range<float>)));

  std::any slider = Range{.min = 0.f, .max = 500.f};
  ICL_TEST_EQ(a.typeId(slider),     std::string("range:slider"));
  ICL_TEST_EQ(a.infoString(slider), std::string("[0,500]"));

  std::any steppedSpin = Range{.min = 1.f, .max = 31.f, .step = 2.f,
                               .ui = UI::Spinbox};
  ICL_TEST_EQ(a.typeId(steppedSpin),     std::string("range:spinbox"));
  ICL_TEST_EQ(a.infoString(steppedSpin), std::string("[1,31]:2"));
}

ICL_REGISTER_TEST("utils.prop.adapter.range_int.round_trip",
                  "Range<int> value round-trips via adapter")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Range<int>)));
  std::any constraint = Range{.min = 0, .max = 100, .ui = UI::Spinbox};
  std::any back = a.fromString(constraint, a.toString(std::any(42)));
  ICL_TEST_EQ(std::any_cast<int>(back), 42);
}

// ---------------------------------------------------------------------------
// Menu adapter — round-trip + legacy info format
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.adapter.menu_string.info",
                  "Menu<string> infoString is leading-comma-separated list")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Menu<std::string>)));
  std::any constraint = Menu{"fast", "slow", "auto"};
  ICL_TEST_EQ(a.typeId(constraint),     std::string("menu"));
  ICL_TEST_EQ(a.infoString(constraint), std::string(",fast,slow,auto"));
}

ICL_REGISTER_TEST("utils.prop.adapter.menu_int.round_trip",
                  "Menu<int> value round-trips via adapter")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Menu<int>)));
  std::any constraint = Menu{8, 16, 32, 64};
  std::any back = a.fromString(constraint, a.toString(std::any(32)));
  ICL_TEST_EQ(std::any_cast<int>(back), 32);
  ICL_TEST_EQ(a.infoString(constraint), std::string(",8,16,32,64"));
}

// ---------------------------------------------------------------------------
// Flag adapter — "on"/"off" XML compat
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.adapter.flag.serialize",
                  "Flag serializes true/false as \"on\"/\"off\"")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Flag)));
  ICL_TEST_EQ(a.toString(std::any(true)),  std::string("on"));
  ICL_TEST_EQ(a.toString(std::any(false)), std::string("off"));
  ICL_TEST_EQ(a.typeId(std::any(Flag{})),  std::string("flag"));
  ICL_TEST_EQ(a.infoString(std::any(Flag{})), std::string(""));
}

ICL_REGISTER_TEST("utils.prop.adapter.flag.parse_variants",
                  "Flag parse accepts on/off/true/false")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Flag)));
  std::any c = Flag{};
  ICL_TEST_TRUE (std::any_cast<bool>(a.fromString(c, "on")));
  ICL_TEST_FALSE(std::any_cast<bool>(a.fromString(c, "off")));
  ICL_TEST_TRUE (std::any_cast<bool>(a.fromString(c, "true")));
  ICL_TEST_FALSE(std::any_cast<bool>(a.fromString(c, "false")));
}

// ---------------------------------------------------------------------------
// Command / Info / Text adapters
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.adapter.command.typeId",
                  "Command adapter reports typeId \"command\"")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Command)));
  ICL_TEST_EQ(a.typeId(std::any(Command{})),     std::string("command"));
  ICL_TEST_EQ(a.infoString(std::any(Command{})), std::string(""));
  ICL_TEST_EQ(a.toString(std::any(std::monostate{})), std::string(""));
}

ICL_REGISTER_TEST("utils.prop.adapter.info.identity",
                  "Info adapter serializes its string value verbatim")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Info)));
  ICL_TEST_EQ(a.toString(std::any(std::string("v1.0.0"))), std::string("v1.0.0"));
  ICL_TEST_EQ(a.typeId(std::any(Info{})), std::string("info"));
}

ICL_REGISTER_TEST("utils.prop.adapter.text.maxlength",
                  "Text adapter surfaces maxLength via infoString")
{
  const auto &a = lookupAdapter(std::type_index(typeid(Text)));
  std::any c = Text{.maxLength = 128};
  ICL_TEST_EQ(a.typeId(c),     std::string("string"));
  ICL_TEST_EQ(a.infoString(c), std::string("128"));
  ICL_TEST_EQ(a.toString(std::any(std::string("hello"))), std::string("hello"));
}
