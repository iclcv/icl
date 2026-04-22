// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/AutoParse.h>

#include <any>
#include <string>

using namespace icl::utils;

// ---------------------------------------------------------------------------
// AutoParse<std::string> — string backend
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.autoparse.string.to_int", "string-backend parses int")
{
  AutoParse<std::string> ap("42");
  int i = ap;
  ICL_TEST_EQ(i, 42);
}

ICL_REGISTER_TEST("utils.autoparse.string.to_double", "string-backend parses double")
{
  AutoParse<std::string> ap("3.14");
  double d = ap;
  ICL_TEST_NEAR(d, 3.14, 1e-9);
}

ICL_REGISTER_TEST("utils.autoparse.string.to_bool", "string-backend parses bool")
{
  ICL_TEST_TRUE(static_cast<bool>(AutoParse<std::string>("true")));
  ICL_TEST_FALSE(static_cast<bool>(AutoParse<std::string>("false")));
}

ICL_REGISTER_TEST("utils.autoparse.string.to_string", "string-backend returns string identity")
{
  AutoParse<std::string> ap("hello");
  std::string s = ap;
  ICL_TEST_EQ(s, std::string("hello"));
}

ICL_REGISTER_TEST("utils.autoparse.string.str_accessor", "str() returns raw payload")
{
  AutoParse<std::string> ap("xyz");
  ICL_TEST_EQ(ap.str(), std::string("xyz"));
}

ICL_REGISTER_TEST("utils.autoparse.string.as_explicit", "as<T>() avoids ambiguity")
{
  AutoParse<std::string> ap("17");
  ICL_TEST_EQ(ap.as<int>(), 17);
  ICL_TEST_NEAR(ap.as<float>(), 17.0f, 1e-6f);
}

// ---------------------------------------------------------------------------
// AutoParse<std::any> — any backend, exact-match path
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.autoparse.any.exact_int", "any-backend exact int")
{
  AutoParse<std::any> ap(std::any(42));
  int i = ap;
  ICL_TEST_EQ(i, 42);
}

ICL_REGISTER_TEST("utils.autoparse.any.exact_string", "any-backend exact string")
{
  AutoParse<std::any> ap(std::any(std::string("hello")));
  std::string s = ap;
  ICL_TEST_EQ(s, std::string("hello"));
}

// ---------------------------------------------------------------------------
// AutoParse<std::any> — numeric widening
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.autoparse.any.int_to_double", "any-backend widens int to double")
{
  AutoParse<std::any> ap(std::any(42));
  double d = ap;
  ICL_TEST_NEAR(d, 42.0, 1e-9);
}

ICL_REGISTER_TEST("utils.autoparse.any.float_to_double", "any-backend widens float to double")
{
  AutoParse<std::any> ap(std::any(3.5f));
  double d = ap;
  ICL_TEST_NEAR(d, 3.5, 1e-6);
}

ICL_REGISTER_TEST("utils.autoparse.any.double_to_int", "any-backend narrows double to int")
{
  AutoParse<std::any> ap(std::any(3.9));
  int i = ap;
  ICL_TEST_EQ(i, 3);  // truncation, static_cast semantics
}

ICL_REGISTER_TEST("utils.autoparse.any.bool_to_int", "any-backend converts bool to int")
{
  AutoParse<std::any> ap(std::any(true));
  int i = ap;
  ICL_TEST_EQ(i, 1);
}

ICL_REGISTER_TEST("utils.autoparse.any.long_to_int", "any-backend narrows long to int")
{
  AutoParse<std::any> ap(std::any(static_cast<long>(12345)));
  int i = ap;
  ICL_TEST_EQ(i, 12345);
}

// ---------------------------------------------------------------------------
// AutoParse<std::any> — string-in-any → parse fallback
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.autoparse.any.string_parsed_to_int", "any-backend parses stored string")
{
  AutoParse<std::any> ap(std::any(std::string("99")));
  int i = ap;
  ICL_TEST_EQ(i, 99);
}

ICL_REGISTER_TEST("utils.autoparse.any.string_parsed_to_double", "any-backend parses stored string to double")
{
  AutoParse<std::any> ap(std::any(std::string("2.718")));
  double d = ap;
  ICL_TEST_NEAR(d, 2.718, 1e-9);
}

// ---------------------------------------------------------------------------
// AutoParse<std::any> — numeric → std::string stringification
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.autoparse.any.int_to_string", "any-backend stringifies stored int")
{
  AutoParse<std::any> ap(std::any(42));
  std::string s = ap;
  ICL_TEST_EQ(s, std::string("42"));
}

ICL_REGISTER_TEST("utils.autoparse.any.float_to_string", "any-backend stringifies stored float")
{
  AutoParse<std::any> ap(std::any(1.5f));
  std::string s = ap;
  // float→string via ostream; accept "1.5"
  ICL_TEST_EQ(s, std::string("1.5"));
}

// ---------------------------------------------------------------------------
// AutoParse<std::any> — failure modes
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.autoparse.any.empty_throws", "converting empty any throws")
{
  AutoParse<std::any> ap;
  ICL_TEST_THROW(ap.as<int>(), ICLException);
}

ICL_REGISTER_TEST("utils.autoparse.any.type_and_has_value", "metadata accessors")
{
  AutoParse<std::any> empty;
  ICL_TEST_FALSE(empty.has_value());

  AutoParse<std::any> with(std::any(42));
  ICL_TEST_TRUE(with.has_value());
  ICL_TEST_TRUE(with.type() == typeid(int));
}
