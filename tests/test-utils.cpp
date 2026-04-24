// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/ConfigFile.h>
#include <icl/utils/Configurable.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/utils/Point.h>
#include <icl/utils/Random.h>
#include <icl/utils/Range.h>
#include <icl/utils/Rect.h>
#include <icl/utils/Size.h>
#include <icl/utils/StringUtils.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

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

// ---------------------------------------------------------------------------
// ConfigFile — YAML-backed hierarchical key/value store
// ---------------------------------------------------------------------------

namespace {

  // Writes `cfg` to a unique temp file path, returns the path.  The file
  // is deleted by TempCfgPath's dtor.
  struct TempCfgPath {
    std::filesystem::path path;
    TempCfgPath() {
      path = std::filesystem::temp_directory_path() /
             ("icl-test-configfile-" + std::to_string(std::random_device{}()) + ".yaml");
    }
    ~TempCfgPath() { std::error_code ec; std::filesystem::remove(path, ec); }
    std::string str() const { return path.string(); }
  };

}

ICL_REGISTER_TEST("utils.configfile.basic_set_get", "set / get round-trip in memory")
{
  ConfigFile cfg;
  cfg.set<int>("general.params.threshold", 7);
  cfg.set<double>("general.params.value", 6.45);
  cfg.set<std::string>("general.params.filename", "hallo.txt");
  cfg.set<bool>("debug", true);

  ICL_TEST_EQ(cfg.get<int>("general.params.threshold"), 7);
  ICL_TEST_NEAR(cfg.get<double>("general.params.value"), 6.45, 1e-9);
  ICL_TEST_EQ(cfg.get<std::string>("general.params.filename"), std::string("hallo.txt"));
  ICL_TEST_TRUE(cfg.get<bool>("debug"));
}

ICL_REGISTER_TEST("utils.configfile.missing_key_throws", "get<T> throws EntryNotFoundException")
{
  ConfigFile cfg;
  ICL_TEST_THROW(cfg.get<int>("not.there"), ConfigFile::EntryNotFoundException);
}

ICL_REGISTER_TEST("utils.configfile.get_default", "get with fallback returns default on missing key")
{
  ConfigFile cfg;
  cfg.set<int>("present", 42);
  ICL_TEST_EQ(cfg.get<int>("present", 99), 42);
  ICL_TEST_EQ(cfg.get<int>("absent",  99), 99);
}

ICL_REGISTER_TEST("utils.configfile.registered_types_roundtrip",
                  "Size / Point / Rect / Range32f round-trip through set+get")
{
  ConfigFile cfg;
  cfg.set<Size>   ("res",   Size(640, 480));
  cfg.set<Point>  ("pos",   Point(10, 20));
  cfg.set<Rect>   ("roi",   Rect(5, 5, 100, 200));
  cfg.set<Range32f>("range", Range32f(0.0f, 1.0f));
  ICL_TEST_EQ(cfg.get<Size>("res"),     Size(640, 480));
  ICL_TEST_EQ(cfg.get<Point>("pos"),    Point(10, 20));
  ICL_TEST_EQ(cfg.get<Rect>("roi"),     Rect(5, 5, 100, 200));
  Range32f r = cfg.get<Range32f>("range");
  ICL_TEST_NEAR(r.minVal, 0.0f, 1e-6f);
  ICL_TEST_NEAR(r.maxVal, 1.0f, 1e-6f);
}

ICL_REGISTER_TEST("utils.configfile.save_load_roundtrip",
                  "save to file, reload into a second instance, values match")
{
  TempCfgPath p;
  {
    ConfigFile cfg;
    cfg.set<int>("general.params.threshold", 7);
    cfg.set<double>("general.params.value", 6.45);
    cfg.set<std::string>("general.params.filename", "hallo.txt");
    cfg.set<Size>("cam.res", Size(640, 480));
    cfg.save(p.str());
  }
  ConfigFile cfg2(p.str());
  ICL_TEST_EQ(cfg2.get<int>("general.params.threshold"), 7);
  ICL_TEST_NEAR(cfg2.get<double>("general.params.value"), 6.45, 1e-9);
  ICL_TEST_EQ(cfg2.get<std::string>("general.params.filename"), std::string("hallo.txt"));
  ICL_TEST_EQ(cfg2.get<Size>("cam.res"), Size(640, 480));
}

ICL_REGISTER_TEST("utils.configfile.save_emits_nested_yaml",
                  "dotted-path keys serialize to nested YAML mappings")
{
  TempCfgPath p;
  {
    ConfigFile cfg;
    cfg.set<int>("a.b.c", 42);
    cfg.save(p.str());
  }
  std::ifstream f(p.str());
  std::ostringstream ss; ss << f.rdbuf();
  const std::string content = ss.str();
  // Expect nested, not flat "a.b.c: 42".
  ICL_TEST_TRUE(content.find("a:") != std::string::npos);
  ICL_TEST_TRUE(content.find("  b:") != std::string::npos);
  ICL_TEST_TRUE(content.find("    c: 42") != std::string::npos);
}

ICL_REGISTER_TEST("utils.configfile.contains_and_find",
                  "contains + regex find work on dotted-path keys")
{
  ConfigFile cfg;
  cfg.set<int>("a.b.x", 1);
  cfg.set<int>("a.b.y", 2);
  cfg.set<int>("a.c.z", 3);
  ICL_TEST_TRUE (cfg.contains("a.b.x"));
  ICL_TEST_FALSE(cfg.contains("a.b.z"));
  auto matches = cfg.find("^a\\.b\\..*");
  ICL_TEST_EQ(matches.size(), size_t(2));
}

ICL_REGISTER_TEST("utils.configfile.set_prefix",
                  "setPrefix transparently scopes subsequent get/set")
{
  ConfigFile cfg;
  cfg.setPrefix("section.");
  cfg.set<int>("x", 10);
  cfg.set<int>("y", 20);
  ICL_TEST_EQ(cfg.get<int>("x"), 10);
  ICL_TEST_EQ(cfg.get<int>("y"), 20);
  // Prefix cleared: the full dotted path is the canonical key.
  cfg.setPrefix("");
  ICL_TEST_EQ(cfg.get<int>("section.x"), 10);
  ICL_TEST_TRUE(cfg.contains("section.y"));
}

ICL_REGISTER_TEST("utils.configfile.data_proxy_assign_and_cast",
                  "operator[] returns Data proxy that assigns and casts")
{
  ConfigFile cfg;
  cfg["k.v"] = 99;                              // Data::operator=<int>
  cfg["k.s"] = std::string("hello");            // string overload
  int         v = cfg["k.v"];                   // Data::operator int()
  std::string s = cfg["k.s"];
  ICL_TEST_EQ(v, 99);
  ICL_TEST_EQ(s, std::string("hello"));
}

ICL_REGISTER_TEST("utils.configfile.restriction_in_memory",
                  "setRestriction / getRestriction stored in-memory, not persisted")
{
  TempCfgPath p;
  {
    ConfigFile cfg;
    cfg.set<int>("threshold", 128);
    cfg.setRestriction("threshold", ConfigFile::KeyRestriction(0.0, 255.0));
    const auto *r = cfg.getRestriction("threshold");
    ICL_TEST_TRUE(r != nullptr);
    ICL_TEST_TRUE(r->hasRange);
    ICL_TEST_NEAR(r->max, 255.0, 1e-9);
    cfg.save(p.str());
  }
  // Reload — restriction is NOT in the YAML file, so the reloaded
  // entry has the scalar value but no restriction attached.
  ConfigFile cfg2(p.str());
  ICL_TEST_EQ(cfg2.get<int>("threshold"), 128);
  ICL_TEST_TRUE(cfg2.getRestriction("threshold") == nullptr);
}

// --- Configurable: callback registration / removal ---

namespace {
  struct TestConf : public Configurable {
    TestConf(){
      addProperty("gain", prop::Range{.min=0, .max=100}, 0);
    }
    // Expose add/removeChild so the child-set tests can fire the channel.
    using Configurable::addChildConfigurable;
    using Configurable::removeChildConfigurable;
  };
}

ICL_REGISTER_TEST("utils.configurable.registerCallback.fires", "callback fires on setPropertyValue")
{
  TestConf c;
  int hits = 0;
  c.registerCallback([&](const Configurable::Property &){ ++hits; });
  c.setPropertyValue("gain", 42);
  ICL_TEST_EQ(hits, 1);
}

ICL_REGISTER_TEST("utils.configurable.registerCallback.returns_distinct_tokens", "each registration yields a fresh non-zero token")
{
  TestConf c;
  auto t1 = c.registerCallback([](const Configurable::Property &){});
  auto t2 = c.registerCallback([](const Configurable::Property &){});
  auto t3 = c.onChildSetChanged([]{});
  ICL_TEST_TRUE(t1 != 0);
  ICL_TEST_TRUE(t2 != 0);
  ICL_TEST_TRUE(t3 != 0);
  ICL_TEST_TRUE(t1 != t2);
  ICL_TEST_TRUE(t2 != t3);
}

ICL_REGISTER_TEST("utils.configurable.removeCallback.unregisters", "removed callback no longer fires")
{
  TestConf c;
  int a = 0, b = 0;
  auto tokA = c.registerCallback([&](const Configurable::Property &){ ++a; });
  c.registerCallback([&](const Configurable::Property &){ ++b; });

  c.setPropertyValue("gain", 10);
  ICL_TEST_EQ(a, 1);
  ICL_TEST_EQ(b, 1);

  c.removeCallback(tokA);
  c.setPropertyValue("gain", 20);
  ICL_TEST_EQ(a, 1);   // stayed — callback removed
  ICL_TEST_EQ(b, 2);   // fired again
}

ICL_REGISTER_TEST("utils.configurable.removeCallback.zero_and_stale_are_noop", "remove(0) and remove(stale) don't throw")
{
  TestConf c;
  c.removeCallback(0);                    // never registered
  auto tok = c.registerCallback([](const Configurable::Property &){});
  c.removeCallback(tok);
  c.removeCallback(tok);                  // second remove with same token: no-op
  c.removeCallback(Configurable::CallbackToken{99999});  // never-issued token
  // If we got here without crashing, the test passes.
  ICL_TEST_TRUE(true);
}

ICL_REGISTER_TEST("utils.configurable.onChildSetChanged.fires_and_removes", "child-set callback fires on add/remove and can be removed")
{
  TestConf parent;
  TestConf child;
  int fires = 0;
  auto tok = parent.onChildSetChanged([&]{ ++fires; });

  parent.addChildConfigurable(&child, "child");
  ICL_TEST_EQ(fires, 1);

  parent.removeChildConfigurable(&child);
  ICL_TEST_EQ(fires, 2);

  parent.removeChildSetCallback(tok);
  parent.addChildConfigurable(&child, "child");
  ICL_TEST_EQ(fires, 2);   // stayed — callback removed
  parent.removeChildConfigurable(&child);
}
