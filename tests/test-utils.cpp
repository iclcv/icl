// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Test.h>
#include <icl/utils/Size.h>
#include <icl/utils/Point.h>
#include <icl/utils/Rect.h>
#include <icl/utils/Range.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Random.h>

using namespace icl::utils;

// --- Size ---

ICL_REGISTER_TEST("utils.size.default", "default ctor is 0x0")
{
  Size s;
  ICL_TEST_EQ(s.width, 0);
  ICL_TEST_EQ(s.height, 0);
}

ICL_REGISTER_TEST("utils.size.ctor", "value construction")
{
  Size s(640, 480);
  ICL_TEST_EQ(s.width, 640);
  ICL_TEST_EQ(s.height, 480);
}

ICL_REGISTER_TEST("utils.size.equality", "operator==")
{
  ICL_TEST_EQ(Size(10,20), Size(10,20));
  ICL_TEST_NE(Size(10,20), Size(20,10));
}

ICL_REGISTER_TEST("utils.size.dim", "getDim()")
{
  Size s(100, 50);
  ICL_TEST_EQ(s.getDim(), 5000);
}

// --- Point ---

ICL_REGISTER_TEST("utils.point.default", "default ctor is 0,0")
{
  Point p;
  ICL_TEST_EQ(p.x, 0);
  ICL_TEST_EQ(p.y, 0);
}

ICL_REGISTER_TEST("utils.point.arithmetic", "operator+")
{
  Point a(3, 4);
  Point b(10, 20);
  Point c = a + b;
  ICL_TEST_EQ(c.x, 13);
  ICL_TEST_EQ(c.y, 24);
}

// --- Rect ---

ICL_REGISTER_TEST("utils.rect.contains", "contains point")
{
  Rect r(10, 10, 100, 100);
  ICL_TEST_TRUE(r.contains(50, 50));
  ICL_TEST_FALSE(r.contains(5, 5));
}

// --- Range ---

ICL_REGISTER_TEST("utils.range.contains", "value in range")
{
  Range32f r(0.0f, 1.0f);
  ICL_TEST_TRUE(r.contains(0.5f));
  ICL_TEST_FALSE(r.contains(1.5f));
}

// --- StringUtils ---

ICL_REGISTER_TEST("utils.string.toLower", "toLower")
{
  ICL_TEST_EQ(toLower("Hello World"), std::string("hello world"));
}

ICL_REGISTER_TEST("utils.string.toUpper", "toUpper")
{
  ICL_TEST_EQ(toUpper("Hello World"), std::string("HELLO WORLD"));
}

ICL_REGISTER_TEST("utils.string.parse_int", "parse<int>")
{
  ICL_TEST_EQ(parse<int>("42"), 42);
  ICL_TEST_EQ(parse<int>("-7"), -7);
}

ICL_REGISTER_TEST("utils.string.parse_float", "parse<float>")
{
  ICL_TEST_NEAR(parse<float>("3.14"), 3.14f, 0.001f);
}

// --- Random ---

ICL_REGISTER_TEST("utils.random.gauss", "gaussRandom stays reasonable")
{
  for(int i = 0; i < 1000; ++i){
    double v = gaussRandom(0.0, 1.0);
    ICL_TEST_TRUE(v > -10.0 && v < 10.0);
  }
}

ICL_REGISTER_TEST("utils.random.uniform", "random within range")
{
  for(int i = 0; i < 1000; ++i){
    double v = random(-5.0, 5.0);
    ICL_TEST_TRUE(v >= -5.0 && v <= 5.0);
  }
}

// --- ClippedCast ---

#include <icl/utils/ClippedCast.h>
#include <icl/core/Types.h>
using namespace icl;

ICL_REGISTER_TEST("utils.clipped_cast.u8_to_f32", "unsigned char to float (was UB on ARM)")
{
  auto f = [](icl8u v){ return clipped_cast<icl8u,icl32f>(v); };
  ICL_TEST_NEAR(f(0), 0.0f, 0.01f);
  ICL_TEST_NEAR(f(128), 128.0f, 0.01f);
  ICL_TEST_NEAR(f(255), 255.0f, 0.01f);
}

ICL_REGISTER_TEST("utils.clipped_cast.s16_to_f32", "signed short to float")
{
  auto f = [](icl16s v){ return clipped_cast<icl16s,icl32f>(v); };
  ICL_TEST_NEAR(f(-1000), -1000.0f, 0.01f);
  ICL_TEST_NEAR(f(0), 0.0f, 0.01f);
  ICL_TEST_NEAR(f(1000), 1000.0f, 0.01f);
}

ICL_REGISTER_TEST("utils.clipped_cast.f32_to_u8", "float to unsigned char clamps")
{
  auto f = [](icl32f v){ return clipped_cast<icl32f,icl8u>(v); };
  ICL_TEST_EQ(f(0.0f), (icl8u)0);
  ICL_TEST_EQ(f(128.0f), (icl8u)128);
  ICL_TEST_EQ(f(255.0f), (icl8u)255);
  ICL_TEST_EQ(f(-100.0f), (icl8u)0);
  ICL_TEST_EQ(f(999.0f), (icl8u)255);
}

ICL_REGISTER_TEST("utils.clipped_cast.f32_to_s16", "float to signed short clamps")
{
  auto f = [](icl32f v){ return clipped_cast<icl32f,icl16s>(v); };
  ICL_TEST_EQ(f(100.0f), (icl16s)100);
  ICL_TEST_EQ(f(-40000.0f), (icl16s)-32768);
  ICL_TEST_EQ(f(40000.0f), (icl16s)32767);
}

ICL_REGISTER_TEST("utils.clipped_cast.int_to_int", "integer narrowing clamps")
{
  auto f8 = [](int v){ return clipped_cast<int,icl8u>(v); };
  ICL_TEST_EQ(f8(300), (icl8u)255);
  ICL_TEST_EQ(f8(-10), (icl8u)0);
  ICL_TEST_EQ(f8(42), (icl8u)42);
  auto fs = [](icl16s v){ return clipped_cast<icl16s,icl8u>(v); };
  ICL_TEST_EQ(fs(-1), (icl8u)0);
}
