// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/AnyMap.h>

#include <string>
#include <typeinfo>
#include <vector>

using namespace icl::utils;

// --- basic set/get ---

ICL_REGISTER_TEST("utils.anymap.empty", "default-constructed map is empty")
{
  AnyMap m;
  ICL_TEST_TRUE(m.empty());
  ICL_TEST_EQ(m.size(), 0u);
  ICL_TEST_FALSE(m.contains("anything"));
}

ICL_REGISTER_TEST("utils.anymap.set_get_int", "round-trip int")
{
  AnyMap m;
  m.set("x", 42);
  ICL_TEST_TRUE(m.contains("x"));
  ICL_TEST_EQ(m.size(), 1u);
  ICL_TEST_EQ(m.get<int>("x"), 42);
}

ICL_REGISTER_TEST("utils.anymap.set_get_string", "round-trip std::string")
{
  AnyMap m;
  m.set<std::string>("s", "hello");
  ICL_TEST_EQ(m.get<std::string>("s"), std::string("hello"));
}

ICL_REGISTER_TEST("utils.anymap.set_get_vector", "round-trip std::vector (arrays go inside std::any)")
{
  AnyMap m;
  std::vector<int> v{1, 2, 3, 4};
  m.set("v", v);
  const auto &out = m.get<std::vector<int>>("v");
  ICL_TEST_EQ(out.size(), 4u);
  ICL_TEST_EQ(out[0], 1);
  ICL_TEST_EQ(out[3], 4);
}

ICL_REGISTER_TEST("utils.anymap.overwrite_same_type", "set overwrites existing value")
{
  AnyMap m;
  m.set("x", 1);
  m.set("x", 99);
  ICL_TEST_EQ(m.get<int>("x"), 99);
  ICL_TEST_EQ(m.size(), 1u);
}

ICL_REGISTER_TEST("utils.anymap.overwrite_different_type", "set overwrites even across types")
{
  AnyMap m;
  m.set("x", 1);
  m.set<std::string>("x", "was an int");
  ICL_TEST_TRUE(m.containsAs<std::string>("x"));
  ICL_TEST_FALSE(m.containsAs<int>("x"));
  ICL_TEST_EQ(m.get<std::string>("x"), std::string("was an int"));
}

// --- get error paths ---

ICL_REGISTER_TEST("utils.anymap.get_missing_throws", "get on missing key throws out_of_range")
{
  AnyMap m;
  ICL_TEST_THROW(m.get<int>("nope"), std::out_of_range);
}

ICL_REGISTER_TEST("utils.anymap.get_wrong_type_throws", "get with wrong type throws bad_any_cast")
{
  AnyMap m;
  m.set("x", 42);
  ICL_TEST_THROW(m.get<std::string>("x"), std::bad_any_cast);
}

ICL_REGISTER_TEST("utils.anymap.typeof_missing_throws", "typeOf on missing key throws")
{
  AnyMap m;
  ICL_TEST_THROW(m.typeOf("nope"), std::out_of_range);
}

// --- tryGet (non-throwing) ---

ICL_REGISTER_TEST("utils.anymap.tryget_hit", "tryGet returns pointer when present")
{
  AnyMap m;
  m.set("x", 42);
  int *p = m.tryGet<int>("x");
  ICL_TEST_TRUE(p != nullptr);
  ICL_TEST_EQ(*p, 42);
}

ICL_REGISTER_TEST("utils.anymap.tryget_missing", "tryGet returns nullptr when key missing")
{
  AnyMap m;
  ICL_TEST_TRUE(m.tryGet<int>("nope") == nullptr);
}

ICL_REGISTER_TEST("utils.anymap.tryget_wrong_type", "tryGet returns nullptr on type mismatch")
{
  AnyMap m;
  m.set("x", 42);
  ICL_TEST_TRUE(m.tryGet<std::string>("x") == nullptr);
}

ICL_REGISTER_TEST("utils.anymap.tryget_mutates", "mutation through tryGet pointer is visible")
{
  AnyMap m;
  m.set("x", 1);
  *m.tryGet<int>("x") = 7;
  ICL_TEST_EQ(m.get<int>("x"), 7);
}

ICL_REGISTER_TEST("utils.anymap.get_ref_mutates", "mutation through get<T>& reference is visible")
{
  AnyMap m;
  m.set("x", 1);
  m.get<int>("x") = 11;
  ICL_TEST_EQ(m.get<int>("x"), 11);
}

// --- const access ---

ICL_REGISTER_TEST("utils.anymap.const_get", "const-ref get works on const map")
{
  AnyMap m;
  m.set("x", 42);
  const AnyMap &cm = m;
  ICL_TEST_EQ(cm.get<int>("x"), 42);
  const int *p = cm.tryGet<int>("x");
  ICL_TEST_TRUE(p != nullptr);
  ICL_TEST_EQ(*p, 42);
}

// --- contains / containsAs ---

ICL_REGISTER_TEST("utils.anymap.contains_type_agnostic", "contains returns true regardless of type")
{
  AnyMap m;
  m.set("x", 42);
  ICL_TEST_TRUE(m.contains("x"));
  ICL_TEST_FALSE(m.contains("y"));
}

ICL_REGISTER_TEST("utils.anymap.contains_as_type_specific", "containsAs<T> discriminates by type")
{
  AnyMap m;
  m.set("x", 42);
  ICL_TEST_TRUE(m.containsAs<int>("x"));
  ICL_TEST_FALSE(m.containsAs<double>("x"));
  ICL_TEST_FALSE(m.containsAs<int>("y"));
}

// --- typeOf ---

ICL_REGISTER_TEST("utils.anymap.typeof_matches", "typeOf returns the stored type")
{
  AnyMap m;
  m.set("x", 42);
  m.set<std::string>("s", "abc");
  ICL_TEST_TRUE(m.typeOf("x") == typeid(int));
  ICL_TEST_TRUE(m.typeOf("s") == typeid(std::string));
}

// --- erase / clear ---

ICL_REGISTER_TEST("utils.anymap.erase_removes", "erase deletes entry and reports true")
{
  AnyMap m;
  m.set("x", 42);
  ICL_TEST_TRUE(m.erase("x"));
  ICL_TEST_FALSE(m.contains("x"));
  ICL_TEST_EQ(m.size(), 0u);
}

ICL_REGISTER_TEST("utils.anymap.erase_missing", "erase on missing key returns false")
{
  AnyMap m;
  ICL_TEST_FALSE(m.erase("nope"));
}

ICL_REGISTER_TEST("utils.anymap.clear", "clear empties the map")
{
  AnyMap m;
  m.set("a", 1);
  m.set("b", 2);
  m.set("c", 3);
  ICL_TEST_EQ(m.size(), 3u);
  m.clear();
  ICL_TEST_TRUE(m.empty());
  ICL_TEST_EQ(m.size(), 0u);
}

// --- iteration ---

ICL_REGISTER_TEST("utils.anymap.iteration", "range-for visits every entry")
{
  AnyMap m;
  m.set("a", 1);
  m.set("b", 2);
  m.set("c", 3);
  int sum = 0;
  std::size_t count = 0;
  for (auto &[k, v] : m) {
    (void)k;
    sum += std::any_cast<int>(v);
    ++count;
  }
  ICL_TEST_EQ(count, 3u);
  ICL_TEST_EQ(sum, 6);
}

// --- heterogeneous storage ---

ICL_REGISTER_TEST("utils.anymap.heterogeneous", "different keys can hold different types simultaneously")
{
  AnyMap m;
  m.set("i", 7);
  m.set<std::string>("s", "hello");
  m.set("d", 3.14);
  m.set("v", std::vector<int>{9, 8});

  ICL_TEST_EQ(m.get<int>("i"), 7);
  ICL_TEST_EQ(m.get<std::string>("s"), std::string("hello"));
  ICL_TEST_NEAR(m.get<double>("d"), 3.14, 1e-12);
  ICL_TEST_EQ(m.get<std::vector<int>>("v").size(), 2u);
}

// --- move-in semantics ---

ICL_REGISTER_TEST("utils.anymap.move_in", "set accepts rvalue and moves it in")
{
  AnyMap m;
  std::vector<int> big(1000, 42);
  auto *data_before = big.data();
  m.set("big", std::move(big));
  // after move, source vector is empty and the stored one owns the buffer
  ICL_TEST_EQ(big.size(), 0u);
  const auto &stored = m.get<std::vector<int>>("big");
  ICL_TEST_EQ(stored.size(), 1000u);
  // not guaranteed by the standard but overwhelmingly true for a vector of 1000 ints:
  // the buffer pointer should have been transferred intact.
  ICL_TEST_EQ(stored.data(), data_before);
}
