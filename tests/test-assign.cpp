// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/Assign.h>
#include <icl/utils/AssignRegistry.h>

#include <any>
#include <stdexcept>
#include <string>
#include <type_traits>

using namespace icl::utils;

// ============================================================
//  Fake stand-in types — exercise both Assign dispatch paths
//  without Qt.  FakeSlider uses the explicit `as<T>()` pattern
//  (no implicit conversion operators); FakeLabel accepts strings
//  via a normal operator= (direct path).
// ============================================================

namespace test_assign {

  // Exposes readbacks via `as<T>()` — exercises the extraction path.
  struct FakeSlider {
    int value = 0;
    FakeSlider &operator=(int v)   { value = v;                   return *this; }
    FakeSlider &operator=(float v) { value = static_cast<int>(v); return *this; }
    FakeSlider &operator=(const std::string &s) { value = std::stoi(s); return *this; }

    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>(value); }

    template<typename T>
      requires std::is_same_v<T, std::string>
    T as() const { return std::to_string(value); }
  };

  // Accepts a string via operator= — exercises the direct path.
  struct FakeLabel {
    std::string text;
    FakeLabel &operator=(const std::string &s) { text = s; return *this; }
  };

  // Type with BOTH operator=(int) AND as<int>() — used to check
  // that the direct path wins (disjoint-constraint tiebreak).
  struct WithBoth {
    int value = 0;
    WithBoth &operator=(int v) { value = v + 1;  return *this; }   // mark of direct
    template<typename T> T as() const { return static_cast<T>(value - 1); }  // inverse
  };

  // No operators at all.
  struct Opaque {};

}  // namespace test_assign

using test_assign::FakeSlider;
using test_assign::FakeLabel;
using test_assign::WithBoth;
using test_assign::Opaque;

// ============================================================
//  Trait tests — `value` must be true if either path works.
// ============================================================

ICL_REGISTER_TEST("utils.assign.trait_primitives", "primitives + numeric conversions satisfy the trait")
{
  static_assert(is_assignable_v<int, int>);
  static_assert(is_assignable_v<int, float>);
  static_assert(is_assignable_v<double, int>);
  static_assert(is_assignable_v<std::string, std::string>);
  static_assert(!is_assignable_v<int, std::string>);
  ICL_TEST_TRUE(true);
}

ICL_REGISTER_TEST("utils.assign.trait_direct_path", "operator= on dst side ⇒ direct-path assignable")
{
  static_assert(is_assignable_v<FakeSlider, int>);    // via operator=(int)
  static_assert(is_assignable_v<FakeSlider, float>);  // via operator=(float)
  static_assert(is_assignable_v<FakeLabel, std::string>);
  ICL_TEST_TRUE(true);
}

ICL_REGISTER_TEST("utils.assign.trait_extract_path", "src.as<Dst>() ⇒ extraction-path assignable")
{
  static_assert(is_assignable_v<int, FakeSlider>);         // via fake.as<int>()
  static_assert(is_assignable_v<float, FakeSlider>);       // via fake.as<float>()
  static_assert(is_assignable_v<double, FakeSlider>);      // via fake.as<double>()
  static_assert(is_assignable_v<std::string, FakeSlider>); // via fake.as<std::string>()
  ICL_TEST_TRUE(true);
}

ICL_REGISTER_TEST("utils.assign.trait_neither_path", "no path available ⇒ trait false")
{
  static_assert(!is_assignable_v<FakeLabel, FakeSlider>);  // no op=, no as<FakeLabel>() on FakeSlider
  static_assert(!is_assignable_v<FakeSlider, FakeLabel>);  // no op=(FakeLabel), no FakeLabel::as<FakeSlider>()
  static_assert(!is_assignable_v<FakeLabel, int>);
  static_assert(!is_assignable_v<Opaque, int>);
  static_assert(!is_assignable_v<int, Opaque>);
  ICL_TEST_TRUE(true);
}

// ============================================================
//  Direct apply — exercises each path explicitly.
// ============================================================

ICL_REGISTER_TEST("utils.assign.apply_direct_op_eq", "direct path: dst = src via operator=")
{
  FakeSlider s;
  int v = 42;
  Assign<FakeSlider, int>::apply(s, v);
  ICL_TEST_EQ(s.value, 42);
}

ICL_REGISTER_TEST("utils.assign.apply_extract_via_as", "extract path: dst = src.as<Dst>()")
{
  FakeSlider s;
  s.value = 99;
  int out = 0;
  Assign<int, FakeSlider>::apply(out, s);  // must go through as<int>()
  ICL_TEST_EQ(out, 99);
}

ICL_REGISTER_TEST("utils.assign.apply_extract_string", "extraction produces a std::string")
{
  FakeSlider s;
  s.value = 7;
  std::string out;
  Assign<std::string, FakeSlider>::apply(out, s);
  ICL_TEST_EQ(out, std::string("7"));
}

ICL_REGISTER_TEST("utils.assign.apply_direct_wins_over_extract", "when both are available, direct path is chosen")
{
  WithBoth w;
  // Direct (operator=): stores (src + 1).  Extract (as<int>()): returns (value - 1).
  // If direct is selected, w.value == 42 + 1 = 43.
  int v = 42;
  Assign<WithBoth, int>::apply(w, v);
  ICL_TEST_EQ(w.value, 43);
}

// ============================================================
//  Runtime registry: enroll / has / dispatch.
// ============================================================

namespace {
  void enrollAll() {
    AssignRegistry::enroll<FakeSlider,   int>();          // direct
    AssignRegistry::enroll<FakeSlider,   float>();        // direct
    AssignRegistry::enroll<int,          FakeSlider>();   // extract
    AssignRegistry::enroll<float,        FakeSlider>();   // extract
    AssignRegistry::enroll<std::string,  FakeSlider>();   // extract (string specialization)
    AssignRegistry::enroll<FakeLabel,    std::string>();  // direct
  }
}

ICL_REGISTER_TEST("utils.assign.registry_has", "has() reflects enrolled pairs")
{
  enrollAll();
  ICL_TEST_TRUE(AssignRegistry::has(typeid(FakeSlider),  typeid(int)));
  ICL_TEST_TRUE(AssignRegistry::has(typeid(int),         typeid(FakeSlider)));
  ICL_TEST_TRUE(AssignRegistry::has(typeid(std::string), typeid(FakeSlider)));
  ICL_TEST_FALSE(AssignRegistry::has(typeid(FakeLabel),  typeid(FakeSlider)));
}

ICL_REGISTER_TEST("utils.assign.dispatch_direct", "dispatch uses direct path when available")
{
  enrollAll();
  std::any dst = FakeSlider{};
  std::any src = 77;
  AssignRegistry::dispatch(dst, src);
  ICL_TEST_EQ(std::any_cast<FakeSlider &>(dst).value, 77);
}

ICL_REGISTER_TEST("utils.assign.dispatch_extract", "dispatch uses extraction path via as<T>()")
{
  enrollAll();
  FakeSlider s;
  s.value = 12;
  std::any dst = 0;
  std::any src = s;
  AssignRegistry::dispatch(dst, src);
  ICL_TEST_EQ(std::any_cast<int &>(dst), 12);
}

ICL_REGISTER_TEST("utils.assign.dispatch_extract_to_string", "dispatch dispatches as<std::string>()")
{
  enrollAll();
  FakeSlider s;
  s.value = 256;
  std::any dst = std::string{};
  std::any src = s;
  AssignRegistry::dispatch(dst, src);
  ICL_TEST_EQ(std::any_cast<std::string &>(dst), std::string("256"));
}

ICL_REGISTER_TEST("utils.assign.dispatch_unknown_throws", "unknown pair throws")
{
  enrollAll();
  std::any dst = Opaque{};
  std::any src = 42;
  ICL_TEST_THROW(AssignRegistry::dispatch(dst, src), std::runtime_error);
}

ICL_REGISTER_TEST("utils.assign.enroll_symmetric", "enroll_symmetric<A, Bs...> registers both directions for every B")
{
  AssignRegistry::enroll_symmetric<FakeSlider, int, float, std::string>();

  // forward direction: FakeSlider = B
  ICL_TEST_TRUE(AssignRegistry::has(typeid(FakeSlider), typeid(int)));
  ICL_TEST_TRUE(AssignRegistry::has(typeid(FakeSlider), typeid(float)));
  ICL_TEST_TRUE(AssignRegistry::has(typeid(FakeSlider), typeid(std::string)));

  // reverse direction: B = FakeSlider (via as<B>())
  ICL_TEST_TRUE(AssignRegistry::has(typeid(int),         typeid(FakeSlider)));
  ICL_TEST_TRUE(AssignRegistry::has(typeid(float),       typeid(FakeSlider)));
  ICL_TEST_TRUE(AssignRegistry::has(typeid(std::string), typeid(FakeSlider)));
}

ICL_REGISTER_TEST("utils.assign.reenroll_idempotent", "re-enrolling same pair doesn't duplicate")
{
  const std::size_t before = AssignRegistry::size();
  AssignRegistry::enroll<FakeSlider, int>();
  AssignRegistry::enroll<FakeSlider, int>();
  ICL_TEST_EQ(AssignRegistry::size(), before);
}

// ============================================================
//  Parity — direct path and runtime dispatch produce equal results.
// ============================================================

ICL_REGISTER_TEST("utils.assign.parity_direct_vs_runtime", "compile-time and runtime yield same result")
{
  enrollAll();

  FakeSlider a;
  a = 55;   // operator=(int)

  std::any dst = FakeSlider{};
  std::any src = 55;
  AssignRegistry::dispatch(dst, src);
  auto &b = std::any_cast<FakeSlider &>(dst);

  ICL_TEST_EQ(a.value, 55);
  ICL_TEST_EQ(b.value, 55);
  ICL_TEST_EQ(a.value, b.value);
}
