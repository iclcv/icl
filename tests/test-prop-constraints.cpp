// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"

#include <icl/utils/Configurable.h>
#include <icl/utils/prop/Adapter.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/prop/Constraints.h>
// `core/prop/Constraints.h` includes `core/Image.h`, which contains an
// inline `as<T>()` template that static_casts `ImgBase*` → `Img<T>*`.
// Clang 21 requires the inheritance relationship to be visible at the
// cast site — forward declarations in Types.h aren't enough.  Pulling
// in Img.h here makes Img<T>'s derivation from ImgBase visible.  See
// TODO.md for the Image.h-side fix.
#include <icl/core/Img.h>

#include <any>
#include <string>
#include <type_traits>
#include <typeindex>
#include <variant>

using namespace icl::utils;
namespace prop = icl::utils::prop;
// `utils::Range<T>` (numeric range) pre-dates `prop::Range<T>` and is
// transitively visible through core/ includes, so we explicitly
// qualify the constraint version as `prop::Range`.  Non-clashing
// constraint names (`Menu`, `Flag`, ...) stay unqualified via the
// targeted using-declarations below.
using prop::Menu;
using prop::Flag;
using prop::Command;
using prop::Info;
using prop::Text;
using prop::UI;
using prop::constraintRegistry;
using prop::lookupAdapter;

namespace {
  /// Minimal concrete Configurable for tests.  Exposes the protected
  /// `addProperty` overloads (both the legacy string-taking one and the
  /// new typed template) so we can exercise them directly.
  struct TestConf : public Configurable {
    using Configurable::addProperty;
    using Configurable::prop;
  };
}

// ---------------------------------------------------------------------------
// CTAD / aggregate deduction — compile-time property-shape checks
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.range.ctad_designated_float",
                  "prop::Range{.min=0.f,.max=500.f} deduces prop::Range<float>")
{
  auto r = prop::Range{.min = 0.f, .max = 500.f};
  static_assert(std::is_same_v<decltype(r), prop::Range<float>>);
  ICL_TEST_TRUE(r.ui == UI::Slider);   // default
  ICL_TEST_EQ(r.min, 0.f);
  ICL_TEST_EQ(r.max, 500.f);
  ICL_TEST_EQ(r.step, 0.f);
}

ICL_REGISTER_TEST("utils.prop.range.ctad_designated_int",
                  "prop::Range{.min=0,.max=100,.ui=Spinbox} deduces prop::Range<int>")
{
  auto r = prop::Range{.min = 0, .max = 100, .ui = UI::Spinbox};
  static_assert(std::is_same_v<decltype(r), prop::Range<int>>);
  ICL_TEST_TRUE(r.ui == UI::Spinbox);
  ICL_TEST_EQ(r.step, 0);
}

ICL_REGISTER_TEST("utils.prop.range.ctad_designated_with_step",
                  "prop::Range{.min=1,.max=31,.step=2,.ui=Spinbox} deduces prop::Range<int>")
{
  auto r = prop::Range{.min = 1, .max = 31, .step = 2, .ui = UI::Spinbox};
  static_assert(std::is_same_v<decltype(r), prop::Range<int>>);
  ICL_TEST_EQ(r.step, 2);
}

ICL_REGISTER_TEST("utils.prop.range.ctad_positional",
                  "prop::Range{0.f, 500.f} also deduces via positional aggregate init")
{
  auto r = prop::Range{0.f, 500.f};
  static_assert(std::is_same_v<decltype(r), prop::Range<float>>);
  ICL_TEST_EQ(r.min, 0.f);
  ICL_TEST_EQ(r.max, 500.f);
}

ICL_REGISTER_TEST("utils.prop.range.positional", "positional aggregate init also works")
{
  prop::Range<float> r{0.f, 1.f};
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
  static_assert(std::is_same_v<prop::Range<float>::value_type, float>);
  static_assert(std::is_same_v<prop::Range<int>::value_type,   int>);
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
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(prop::Range<int>))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(prop::Range<float>))));
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
// prop::Range adapter — value round-trip + legacy typeId/infoString
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.adapter.range_float.round_trip",
                  "prop::Range<float> value serializes and parses back through adapter")
{
  const auto &a = lookupAdapter(std::type_index(typeid(prop::Range<float>)));
  std::any constraint = prop::Range{.min = 0.f, .max = 500.f};
  std::string s = a.toString(std::any(123.5f));
  std::any back = a.fromString(constraint, s);
  ICL_TEST_NEAR(std::any_cast<float>(back), 123.5f, 1e-5f);
}

ICL_REGISTER_TEST("utils.prop.adapter.range_float.type_and_info",
                  "prop::Range<float> typeId reflects UI; infoString encodes [min,max]:step")
{
  const auto &a = lookupAdapter(std::type_index(typeid(prop::Range<float>)));

  std::any slider = prop::Range{.min = 0.f, .max = 500.f};
  ICL_TEST_EQ(a.typeId(slider),     std::string("range:slider"));
  ICL_TEST_EQ(a.infoString(slider), std::string("[0,500]"));

  std::any steppedSpin = prop::Range{.min = 1.f, .max = 31.f, .step = 2.f,
                               .ui = UI::Spinbox};
  ICL_TEST_EQ(a.typeId(steppedSpin),     std::string("range:spinbox"));
  ICL_TEST_EQ(a.infoString(steppedSpin), std::string("[1,31]:2"));
}

ICL_REGISTER_TEST("utils.prop.adapter.range_int.round_trip",
                  "prop::Range<int> value round-trips via adapter")
{
  const auto &a = lookupAdapter(std::type_index(typeid(prop::Range<int>)));
  std::any constraint = prop::Range{.min = 0, .max = 100, .ui = UI::Spinbox};
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

// ---------------------------------------------------------------------------
// Typed addProperty<C> — the Configurable-side integration (step 2)
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.prop.configurable.addProperty_range_float",
                  "addProperty<prop::Range<float>> populates legacy strings and constraint payload")
{
  TestConf c;
  c.addProperty("gain", prop::Range{.min = 0.f, .max = 500.f}, 250.f);

  ICL_TEST_EQ(c.prop("gain").type(),  std::string("range:slider"));
  ICL_TEST_EQ(c.prop("gain").info(),  std::string("[0,500]"));
  ICL_TEST_NEAR(c.prop("gain").as<float>(), 250.f, 1e-5f);

  // constraint payload is recoverable and matches what we passed
  const auto &p = c.prop("gain");
  ICL_TEST_TRUE(p.constraint.has_value());
  const auto &r = std::any_cast<const prop::Range<float> &>(p.constraint);
  ICL_TEST_EQ(r.min, 0.f);
  ICL_TEST_EQ(r.max, 500.f);
  ICL_TEST_TRUE(r.ui == UI::Slider);
}

ICL_REGISTER_TEST("utils.prop.configurable.addProperty_range_spinbox",
                  "addProperty<prop::Range<int>> with Spinbox UI synthesizes range:spinbox + stepped info")
{
  TestConf c;
  c.addProperty("ksize",
                prop::Range{.min = 1, .max = 31, .step = 2, .ui = UI::Spinbox},
                3);

  ICL_TEST_EQ(c.prop("ksize").type(), std::string("range:spinbox"));
  ICL_TEST_EQ(c.prop("ksize").info(), std::string("[1,31]:2"));
  ICL_TEST_EQ(c.prop("ksize").as<int>(), 3);

  const auto &r = std::any_cast<const prop::Range<int> &>(c.prop("ksize").constraint);
  ICL_TEST_EQ(r.step, 2);
  ICL_TEST_TRUE(r.ui == UI::Spinbox);
}

ICL_REGISTER_TEST("utils.prop.configurable.addProperty_menu_string",
                  "addProperty<Menu<string>> synthesizes type=menu and comma-listed info")
{
  TestConf c;
  c.addProperty("mode", Menu{"fast", "slow", "auto"}, std::string("slow"));

  ICL_TEST_EQ(c.prop("mode").type(), std::string("menu"));
  ICL_TEST_EQ(c.prop("mode").info(), std::string(",fast,slow,auto"));
  ICL_TEST_EQ(c.prop("mode").value.str(), std::string("slow"));

  const auto &m = std::any_cast<const Menu<std::string> &>(c.prop("mode").constraint);
  ICL_TEST_EQ(m.choices.size(), 3u);
  ICL_TEST_EQ(m.choices[1], std::string("slow"));
}

ICL_REGISTER_TEST("utils.prop.configurable.addProperty_flag",
                  "addProperty<Flag> stores typed bool; callers extract via implicit bool cast")
{
  TestConf c;
  c.addProperty("enabled", Flag{}, true);

  ICL_TEST_EQ(c.prop("enabled").type(), std::string("flag"));
  ICL_TEST_EQ(c.prop("enabled").info(), std::string(""));
  // Typed extraction — the idiomatic read path.
  ICL_TEST_TRUE(bool(c.prop("enabled").value));
  ICL_TEST_EQ(c.prop("enabled").as<bool>(), true);
  // Property::value string field retired in step 9 commit 4 —
  // typed_value is the sole storage.  Round-trip through the adapter
  // produces the legacy "on" format on demand.
  ICL_TEST_TRUE(c.prop("enabled").constraint.has_value());
  ICL_TEST_TRUE(std::any_cast<bool>(c.prop("enabled").typed_value));
}

ICL_REGISTER_TEST("utils.prop.configurable.addProperty_command",
                  "addProperty<Command> registers with no value; .str() throws by design")
{
  TestConf c;
  c.addProperty("save", Command{});

  ICL_TEST_EQ(c.prop("save").type(), std::string("command"));
  ICL_TEST_EQ(c.prop("save").info(), std::string(""));
  // Command stores std::monostate — intentionally not stringifiable.
  // Consumers that iterate all properties (e.g. icl-configurable-info)
  // must guard per-row.
  bool threw = false;
  try { (void)c.prop("save").value.str(); } catch (...) { threw = true; }
  ICL_TEST_TRUE(threw);
  // The typed payload is the monostate sentinel.
  ICL_TEST_TRUE(c.prop("save").typed_value.type() == typeid(std::monostate));
}

ICL_REGISTER_TEST("utils.prop.configurable.addProperty_info",
                  "addProperty<Info> registers a read-only string")
{
  TestConf c;
  c.addProperty("version", Info{}, std::string("v1.0.0"));

  ICL_TEST_EQ(c.prop("version").type(),  std::string("info"));
  ICL_TEST_EQ(c.prop("version").value.str(), std::string("v1.0.0"));
}

ICL_REGISTER_TEST("utils.prop.configurable.addProperty_text",
                  "addProperty<Text> surfaces maxLength in info")
{
  TestConf c;
  c.addProperty("weights", Text{.maxLength = 128}, std::string("1,2,3"));

  ICL_TEST_EQ(c.prop("weights").type(), std::string("string"));
  ICL_TEST_EQ(c.prop("weights").info(), std::string("128"));
  ICL_TEST_EQ(c.prop("weights").value.str(), std::string("1,2,3"));
}

ICL_REGISTER_TEST("utils.prop.configurable.legacy_dispatch_to_typed",
                  "legacy string-taking addProperty builds the typed constraint internally")
{
  TestConf c;
  c.addProperty("legacy.prop", "range:slider", "[0,100]", AutoParse<std::string>(50));

  ICL_TEST_EQ(c.prop("legacy.prop").type(), std::string("range:slider"));
  ICL_TEST_EQ(c.prop("legacy.prop").info(), std::string("[0,100]"));
  // Step 9 commit 1: the legacy overload now dispatches internally to
  // the typed path, so every registered property has both constraint
  // and typed_value populated — even through the string-taking API.
  ICL_TEST_TRUE(c.prop("legacy.prop").constraint.has_value());
  const auto &r = std::any_cast<const prop::Range<int>&>(c.prop("legacy.prop").constraint);
  ICL_TEST_EQ(r.min, 0);
  ICL_TEST_EQ(r.max, 100);
  ICL_TEST_EQ(std::any_cast<int>(c.prop("legacy.prop").typed_value), 50);
}

// ---------------------------------------------------------------------------
// core::prop constraints — cross-module extensibility (step 4a)
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("core.prop.registry.has_builtin_adapters",
                  "core::prop::Color and ImageView are registered at static-init time")
{
  const auto &reg = constraintRegistry();
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(icl::core::prop::Color))));
  ICL_TEST_TRUE(reg.has(std::type_index(typeid(icl::core::prop::ImageView))));
}

ICL_REGISTER_TEST("core.prop.adapter.color.round_trip",
                  "Color adapter typeId reports \"color\" and round-trips core::Color")
{
  const auto &a = lookupAdapter(std::type_index(typeid(icl::core::prop::Color)));
  ICL_TEST_EQ(a.typeId(std::any(icl::core::prop::Color{})),     std::string("color"));
  ICL_TEST_EQ(a.infoString(std::any(icl::core::prop::Color{})), std::string(""));

  const icl::core::Color src(10, 128, 255);
  std::string s = a.toString(std::any(src));
  std::any back = a.fromString(std::any(icl::core::prop::Color{}), s);
  const auto recovered = std::any_cast<icl::core::Color>(back);
  ICL_TEST_EQ(recovered[0], src[0]);
  ICL_TEST_EQ(recovered[1], src[1]);
  ICL_TEST_EQ(recovered[2], src[2]);
}

ICL_REGISTER_TEST("core.prop.adapter.imageview.nonserializable",
                  "ImageView adapter typeId reports \"image\"; toString is empty by design")
{
  const auto &a = lookupAdapter(std::type_index(typeid(icl::core::prop::ImageView)));
  ICL_TEST_EQ(a.typeId(std::any(icl::core::prop::ImageView{})), std::string("image"));
  ICL_TEST_EQ(a.infoString(std::any(icl::core::prop::ImageView{})), std::string(""));
  // Images don't round-trip through XML — toString returns "" rather
  // than throw, so saveProperties / configurable-info can iterate
  // without guarding.
  ICL_TEST_EQ(a.toString(std::any(icl::core::Image{})), std::string(""));
}

ICL_REGISTER_TEST("core.prop.configurable.addProperty_color",
                  "addProperty<core::prop::Color> stores typed Color and synthesizes type=\"color\"")
{
  TestConf c;
  const icl::core::Color bg(32, 64, 96);
  c.addProperty("bg", icl::core::prop::Color{}, bg);

  ICL_TEST_EQ(c.prop("bg").type(), std::string("color"));
  // Typed access — the fast path.
  const auto &stored = std::any_cast<const icl::core::Color &>(
      c.prop("bg").typed_value);
  ICL_TEST_EQ(stored[0], bg[0]);
  ICL_TEST_EQ(stored[1], bg[1]);
  ICL_TEST_EQ(stored[2], bg[2]);
}
