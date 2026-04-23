// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/AutoParse.h>

#include <any>
#include <limits>
#include <string>
#include <string_view>

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
// AutoParse<std::string_view> — view backend
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.autoparse.view.to_int", "view-backend parses int")
{
  AutoParse<std::string_view> ap(std::string_view("42"));
  int i = ap;
  ICL_TEST_EQ(i, 42);
}

ICL_REGISTER_TEST("utils.autoparse.view.to_long", "view-backend parses long")
{
  AutoParse<std::string_view> ap(std::string_view("12345678"));
  long l = ap;
  ICL_TEST_EQ(l, 12345678L);
}

ICL_REGISTER_TEST("utils.autoparse.view.to_unsigned", "view-backend parses unsigned")
{
  AutoParse<std::string_view> ap(std::string_view("4000000000"));
  unsigned u = ap;
  ICL_TEST_EQ(u, 4000000000u);
}

ICL_REGISTER_TEST("utils.autoparse.view.signed_negative", "view-backend parses negative int")
{
  AutoParse<std::string_view> ap(std::string_view("-17"));
  int i = ap;
  ICL_TEST_EQ(i, -17);
}

ICL_REGISTER_TEST("utils.autoparse.view.leading_plus", "view-backend accepts leading '+'")
{
  AutoParse<std::string_view> ap(std::string_view("+42"));
  int i = ap;
  ICL_TEST_EQ(i, 42);
}

ICL_REGISTER_TEST("utils.autoparse.view.whitespace_trim", "view-backend trims whitespace")
{
  AutoParse<std::string_view> ap(std::string_view("  99  "));
  int i = ap;
  ICL_TEST_EQ(i, 99);
}

ICL_REGISTER_TEST("utils.autoparse.view.to_double", "view-backend parses double")
{
  AutoParse<std::string_view> ap(std::string_view("3.14"));
  double d = ap;
  ICL_TEST_NEAR(d, 3.14, 1e-9);
}

ICL_REGISTER_TEST("utils.autoparse.view.to_bool", "view-backend parses bool")
{
  ICL_TEST_TRUE (static_cast<bool>(AutoParse<std::string_view>(std::string_view("true"))));
  ICL_TEST_FALSE(static_cast<bool>(AutoParse<std::string_view>(std::string_view("false"))));
}

ICL_REGISTER_TEST("utils.autoparse.view.str_accessor", "str() returns the view")
{
  AutoParse<std::string_view> ap(std::string_view("xyz"));
  ICL_TEST_EQ(std::string(ap.str()), std::string("xyz"));
}

ICL_REGISTER_TEST("utils.autoparse.view.as_explicit", "as<T>() avoids ambiguity")
{
  AutoParse<std::string_view> ap(std::string_view("17"));
  ICL_TEST_EQ(ap.as<int>(), 17);
  ICL_TEST_NEAR(ap.as<float>(), 17.0f, 1e-6f);
}

ICL_REGISTER_TEST("utils.autoparse.view.from_string", "view-backend ctor from std::string")
{
  std::string owned = "77";
  AutoParse<std::string_view> ap(owned);
  int i = ap;
  ICL_TEST_EQ(i, 77);
}

ICL_REGISTER_TEST("utils.autoparse.view.from_cstr", "view-backend ctor from const char*")
{
  AutoParse<std::string_view> ap("88");
  int i = ap;
  ICL_TEST_EQ(i, 88);
}

ICL_REGISTER_TEST("utils.autoparse.view.parse_throws_on_garbage", "parse<int>(sv) throws on bad input")
{
  AutoParse<std::string_view> ap(std::string_view("not_a_number"));
  ICL_TEST_THROW(ap.as<int>(), ICLException);
}

// ---------------------------------------------------------------------------
// parse<T>(std::string_view) fast-path coverage
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.parse.view.int_family", "from_chars fast paths cover the int family")
{
  ICL_TEST_EQ(parse<short>             (std::string_view("-1")),            static_cast<short>(-1));
  ICL_TEST_EQ(parse<unsigned short>    (std::string_view("65535")),         static_cast<unsigned short>(65535));
  ICL_TEST_EQ(parse<int>               (std::string_view("-2147483648")),   std::numeric_limits<int>::min());
  ICL_TEST_EQ(parse<unsigned int>      (std::string_view("4294967295")),    4294967295u);
  ICL_TEST_EQ(parse<long>              (std::string_view("-100")),          -100L);
  ICL_TEST_EQ(parse<unsigned long>     (std::string_view("100")),           100UL);
  ICL_TEST_EQ(parse<long long>         (std::string_view("-9223372036854775807")), -9223372036854775807LL);
  ICL_TEST_EQ(parse<unsigned long long>(std::string_view("18446744073709551615")), 18446744073709551615ULL);
}

ICL_REGISTER_TEST("utils.parse.view.string_view_identity", "parse<string_view>(sv) is identity")
{
  std::string_view in("hello");
  std::string_view out = parse<std::string_view>(in);
  ICL_TEST_TRUE(in.data() == out.data());
  ICL_TEST_EQ(in.size(), out.size());
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
