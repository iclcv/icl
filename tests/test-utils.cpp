// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Test.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Rect.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Random.h>

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
