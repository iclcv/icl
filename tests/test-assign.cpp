// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/Assign.h>
#include <icl/utils/AssignRegistry.h>

#include <any>
#include <stdexcept>
#include <string>

using namespace icl::utils;

// ============================================================
//  Fake stand-in types — exercise the trait/registry without
//  dragging in Qt.  Each fakes a "widget handle" by exposing the
//  natural C++ primitives (operator= for incoming conversions,
//  conversion operators for readback).  No `Assign<>`
//  specializations needed — the generic trait in `Assign.h`
//  defers to these operators via `std::is_assignable_v`.
// ============================================================

namespace test_assign {

  struct FakeSlider {
    int value = 0;
    FakeSlider &operator=(int v)   { value = v;                   return *this; }
    FakeSlider &operator=(float v) { value = static_cast<int>(v); return *this; }
    operator int() const { return value; }
  };

  struct FakeLabel {
    std::string text;
    FakeLabel &operator=(const std::string &s) { text = s; return *this; }
  };

  /// A type with no operators at all — used to verify the trait
  /// reports unrelated pairs as non-assignable.
  struct Opaque {};

}  // namespace test_assign

using test_assign::FakeSlider;
using test_assign::FakeLabel;
using test_assign::Opaque;

// ============================================================
//  Compile-time trait tests
// ============================================================

ICL_REGISTER_TEST("utils.assign.trait_primitives", "primitives + numeric conversions are assignable")
{
  static_assert(is_assignable_v<int, int>);
  static_assert(is_assignable_v<int, float>);
  static_assert(is_assignable_v<double, int>);
  static_assert(is_assignable_v<std::string, std::string>);
  static_assert(!is_assignable_v<int, std::string>);
  ICL_TEST_TRUE(true);
}

ICL_REGISTER_TEST("utils.assign.trait_class_with_operators", "classes with operator= / conversion operators satisfy the trait")
{
  // FakeSlider has operator=(int), operator=(float), operator int().
  static_assert(is_assignable_v<FakeSlider, int>);
  static_assert(is_assignable_v<FakeSlider, float>);
  static_assert(is_assignable_v<int, FakeSlider>);         // via operator int()
  static_assert(is_assignable_v<FakeLabel, std::string>);
  ICL_TEST_TRUE(true);
}

ICL_REGISTER_TEST("utils.assign.trait_false_for_unrelated", "no operator = ⇒ trait false")
{
  static_assert(!is_assignable_v<FakeLabel, FakeSlider>);
  static_assert(!is_assignable_v<FakeSlider, FakeLabel>);
  static_assert(!is_assignable_v<FakeLabel, int>);
  static_assert(!is_assignable_v<Opaque, int>);
  static_assert(!is_assignable_v<int, Opaque>);
  ICL_TEST_TRUE(true);
}

// ============================================================
//  Direct Assign::apply — exercises the generic trait body.
// ============================================================

ICL_REGISTER_TEST("utils.assign.apply_int_into_slider", "apply delegates to operator=(int)")
{
  FakeSlider s;
  int v = 42;
  Assign<FakeSlider, int>::apply(s, v);
  ICL_TEST_EQ(s.value, 42);
}

ICL_REGISTER_TEST("utils.assign.apply_float_into_slider", "apply delegates to operator=(float)")
{
  FakeSlider s;
  float v = 3.7f;
  Assign<FakeSlider, float>::apply(s, v);
  ICL_TEST_EQ(s.value, 3);
}

ICL_REGISTER_TEST("utils.assign.apply_slider_into_int", "apply uses conversion operator for readback")
{
  FakeSlider s;
  s.value = 99;
  int out = 0;
  Assign<int, FakeSlider>::apply(out, s);  // relies on FakeSlider::operator int()
  ICL_TEST_EQ(out, 99);
}

ICL_REGISTER_TEST("utils.assign.apply_string_into_label", "apply forwards to operator=(const string&)")
{
  FakeLabel l;
  std::string s = "hello";
  Assign<FakeLabel, std::string>::apply(l, s);
  ICL_TEST_EQ(l.text, std::string("hello"));
}

// ============================================================
//  Runtime registry: enroll / has / size
// ============================================================

namespace {
  void enrollAll() {
    auto &r = AssignRegistry::instance();
    r.enroll<FakeSlider, int>();
    r.enroll<FakeSlider, float>();
    r.enroll<int,        FakeSlider>();
    r.enroll<FakeLabel,  std::string>();
  }
}

ICL_REGISTER_TEST("utils.assign.registry_enroll_size", "enroll grows the table")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  ICL_TEST_TRUE(r.size() >= 4u);
}

ICL_REGISTER_TEST("utils.assign.registry_has_hit", "has() true for enrolled pair (Dst, Src order)")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  ICL_TEST_TRUE(r.has(typeid(FakeSlider), typeid(int)));
  ICL_TEST_TRUE(r.has(typeid(FakeSlider), typeid(float)));
  ICL_TEST_TRUE(r.has(typeid(int),        typeid(FakeSlider)));
  ICL_TEST_TRUE(r.has(typeid(FakeLabel),  typeid(std::string)));
}

ICL_REGISTER_TEST("utils.assign.registry_has_miss", "has() false for unenrolled pair")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  ICL_TEST_FALSE(r.has(typeid(FakeSlider), typeid(FakeLabel)));
  ICL_TEST_FALSE(r.has(typeid(double),     typeid(FakeLabel)));
  ICL_TEST_FALSE(r.has(typeid(Opaque),     typeid(int)));
}

ICL_REGISTER_TEST("utils.assign.registry_reenroll_idempotent", "re-enrolling same pair doesn't duplicate")
{
  auto &r = AssignRegistry::instance();
  const std::size_t before = r.size();
  r.enroll<FakeSlider, int>();
  r.enroll<FakeSlider, int>();
  r.enroll<FakeSlider, int>();
  ICL_TEST_EQ(r.size(), before);  // unchanged
}

// ============================================================
//  Runtime registry: dispatch
// ============================================================

ICL_REGISTER_TEST("utils.assign.dispatch_int_into_slider", "dispatch(FakeSlider = int)")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  std::any dst = FakeSlider{};
  std::any src = 77;
  r.dispatch(dst, src);
  ICL_TEST_EQ(std::any_cast<FakeSlider &>(dst).value, 77);
}

ICL_REGISTER_TEST("utils.assign.dispatch_string_into_label", "dispatch(FakeLabel = string)")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  std::any dst = FakeLabel{};
  std::any src = std::string("hi there");
  r.dispatch(dst, src);
  ICL_TEST_EQ(std::any_cast<FakeLabel &>(dst).text, std::string("hi there"));
}

ICL_REGISTER_TEST("utils.assign.dispatch_slider_into_int", "dispatch(int = FakeSlider) reads via conversion op")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  FakeSlider s;
  s.value = 12;
  std::any dst = 0;    // int
  std::any src = s;
  r.dispatch(dst, src);
  ICL_TEST_EQ(std::any_cast<int &>(dst), 12);
}

ICL_REGISTER_TEST("utils.assign.dispatch_unknown_dst_throws", "dispatch throws when Dst type has no rules")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  std::any dst = Opaque{};
  std::any src = 42;
  ICL_TEST_THROW(r.dispatch(dst, src), std::runtime_error);
}

ICL_REGISTER_TEST("utils.assign.dispatch_unknown_src_throws", "dispatch throws when pair isn't registered")
{
  enrollAll();
  auto &r = AssignRegistry::instance();
  std::any dst = FakeSlider{};
  std::any src = FakeLabel{};  // no rule FakeSlider = FakeLabel
  ICL_TEST_THROW(r.dispatch(dst, src), std::runtime_error);
}

// ============================================================
//  Parity — Path A (compile-time) and Path B (runtime) produce
//  the same result on the same input.
// ============================================================

ICL_REGISTER_TEST("utils.assign.parity_direct_vs_registry", "both paths produce identical results")
{
  enrollAll();

  // Path A (compile-time via operator=):
  FakeSlider a;
  a = 55;

  // Path B (runtime via registry):
  std::any dst = FakeSlider{};
  std::any src = 55;
  AssignRegistry::instance().dispatch(dst, src);
  auto &b = std::any_cast<FakeSlider &>(dst);

  ICL_TEST_EQ(a.value, 55);
  ICL_TEST_EQ(b.value, 55);
  ICL_TEST_EQ(a.value, b.value);
}
