// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/PluginRegistry.h>
#include <icl/utils/Test.h>
#include <icl/utils/Exception.h>

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace icl::utils;

// ============================================================
// Test fixtures
// ============================================================

// A multi-method "class plugin" used to validate ClassPluginRegistry.
struct DummyPlugin {
  explicit DummyPlugin(std::string n, int v = 0) : name(std::move(n)), value(v) {}
  std::string name;
  int value;
};

// A context type that exercises the applicability predicate path.
struct DepthCtx {
  int depth; // e.g. 8, 16, 32
};

// ============================================================
// Function-plugin alias: basic registration + lookup
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.function.register_and_get",
                  "register a callable and retrieve it by key")
{
  FunctionPluginRegistry<int(int)> reg;
  reg.registerPlugin("double", [](int x) { return 2 * x; });
  reg.registerPlugin("square", [](int x) { return x * x; });

  ICL_TEST_EQ(reg.size(), 2u);
  ICL_TEST_TRUE(reg.has("double"));
  ICL_TEST_TRUE(reg.has("square"));
  ICL_TEST_FALSE(reg.has("missing"));

  const auto* e = reg.get("double");
  ICL_TEST_TRUE(e != nullptr);
  ICL_TEST_EQ(e->payload(21), 42);

  ICL_TEST_TRUE(reg.get("missing") == nullptr);
}

// ============================================================
// Class-plugin alias: factory semantics
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.class.factory_produces_fresh_instances",
                  "ClassPluginRegistry stores factories; each call yields a new instance")
{
  ClassPluginRegistry<DummyPlugin> reg;
  reg.registerPlugin("a", []{ return std::make_unique<DummyPlugin>("a", 1); });
  reg.registerPlugin("b", []{ return std::make_unique<DummyPlugin>("b", 2); });

  const auto* e = reg.get("a");
  ICL_TEST_TRUE(e != nullptr);
  auto inst1 = e->payload();
  auto inst2 = e->payload();
  ICL_TEST_EQ(inst1->name, std::string("a"));
  ICL_TEST_EQ(inst1->value, 1);
  ICL_TEST_TRUE(inst1.get() != inst2.get());  // distinct allocations
}

ICL_REGISTER_TEST("utils.plugin-registry.class.ctor_args",
                  "ClassPluginRegistry factory accepts ctor arguments")
{
  ClassPluginRegistry<DummyPlugin, const std::string&, int> reg;
  reg.registerPlugin("make",
    [](const std::string& n, int v) { return std::make_unique<DummyPlugin>(n, v); });

  auto inst = reg.get("make")->payload("hello", 7);
  ICL_TEST_EQ(inst->name, std::string("hello"));
  ICL_TEST_EQ(inst->value, 7);
}

// ============================================================
// OnDuplicate policies
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.policy.throw_on_duplicate",
                  "default policy throws on duplicate key")
{
  FunctionPluginRegistry<void()> reg;  // default = Throw
  reg.registerPlugin("x", []{});
  ICL_TEST_THROW(reg.registerPlugin("x", []{}), ICLException);
  ICL_TEST_EQ(reg.size(), 1u);
}

ICL_REGISTER_TEST("utils.plugin-registry.policy.keep_first",
                  "KeepFirst silently ignores re-registration")
{
  FunctionPluginRegistry<int()> reg(OnDuplicate::KeepFirst);
  reg.registerPlugin("x", []{ return 1; });
  reg.registerPlugin("x", []{ return 99; });  // ignored
  ICL_TEST_EQ(reg.size(), 1u);
  ICL_TEST_EQ(reg.get("x")->payload(), 1);
}

ICL_REGISTER_TEST("utils.plugin-registry.policy.replace",
                  "Replace overwrites the existing entry")
{
  FunctionPluginRegistry<int()> reg(OnDuplicate::Replace);
  reg.registerPlugin("x", []{ return 1; });
  reg.registerPlugin("x", []{ return 99; });
  ICL_TEST_EQ(reg.size(), 1u);
  ICL_TEST_EQ(reg.get("x")->payload(), 99);
}

ICL_REGISTER_TEST("utils.plugin-registry.policy.keep_highest_priority",
                  "KeepHighestPriority: strictly-higher priority wins; ties keep first")
{
  FunctionPluginRegistry<int()> reg(OnDuplicate::KeepHighestPriority);
  // fallback registers first at low prio; primary wins when it registers later
  reg.registerPlugin("ext", []{ return 1; }, "fallback", -10);
  reg.registerPlugin("ext", []{ return 2; }, "primary",   0);
  ICL_TEST_EQ(reg.size(), 1u);
  ICL_TEST_EQ(reg.get("ext")->payload(), 2);
  ICL_TEST_EQ(reg.get("ext")->description, std::string("primary"));

  // reverse order also converges to highest-priority winner
  FunctionPluginRegistry<int()> reg2(OnDuplicate::KeepHighestPriority);
  reg2.registerPlugin("ext", []{ return 2; }, "primary",   0);
  reg2.registerPlugin("ext", []{ return 1; }, "fallback", -10);
  ICL_TEST_EQ(reg2.get("ext")->payload(), 2);

  // tie → first-wins
  FunctionPluginRegistry<int()> reg3(OnDuplicate::KeepHighestPriority);
  reg3.registerPlugin("ext", []{ return 10; }, "a", 0);
  reg3.registerPlugin("ext", []{ return 20; }, "b", 0);
  ICL_TEST_EQ(reg3.get("ext")->payload(), 10);
}

// ============================================================
// Priority-based resolution
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.priority.higher_wins",
                  "resolve() returns highest-priority entry")
{
  FunctionPluginRegistry<int()> reg;
  reg.registerPlugin("low",    []{ return 1; }, "low",    /*prio*/ -10);
  reg.registerPlugin("mid",    []{ return 2; }, "mid",    /*prio*/  0);
  reg.registerPlugin("high",   []{ return 3; }, "high",   /*prio*/ 10);

  const auto* r = reg.resolve();
  ICL_TEST_TRUE(r != nullptr);
  ICL_TEST_EQ(r->key, std::string("high"));
  ICL_TEST_EQ(r->payload(), 3);

  // keys() reflects priority-desc ordering
  auto ks = reg.keys();
  ICL_TEST_EQ(ks.size(), 3u);
  ICL_TEST_EQ(ks[0], std::string("high"));
  ICL_TEST_EQ(ks[2], std::string("low"));
}

ICL_REGISTER_TEST("utils.plugin-registry.priority.tie_preserves_insertion_order",
                  "stable sort: entries with equal priority keep insertion order")
{
  FunctionPluginRegistry<int()> reg;
  reg.registerPlugin("first",  []{ return 1; }, "", 0);
  reg.registerPlugin("second", []{ return 2; }, "", 0);
  reg.registerPlugin("third",  []{ return 3; }, "", 0);

  auto ks = reg.keys();
  ICL_TEST_EQ(ks[0], std::string("first"));
  ICL_TEST_EQ(ks[1], std::string("second"));
  ICL_TEST_EQ(ks[2], std::string("third"));
}

// ============================================================
// Applicability predicates (BackendSelector-style use case)
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.applicability.filters_by_context",
                  "resolve() skips entries whose applicability predicate returns false")
{
  PluginRegistry<std::string, std::function<int()>, DepthCtx> reg;

  auto only8  = [](const DepthCtx& c) { return c.depth ==  8; };
  auto only32 = [](const DepthCtx& c) { return c.depth == 32; };

  // Higher priority but restricted to 32-bit contexts.
  reg.registerPlugin("fast32", []{ return 320; }, "",  10, only32);
  // Lower priority but handles 8-bit.
  reg.registerPlugin("slow8",  []{ return  80; }, "",   0, only8);

  ICL_TEST_EQ(reg.resolve({8})->key,  std::string("slow8"));
  ICL_TEST_EQ(reg.resolve({32})->key, std::string("fast32"));
  ICL_TEST_TRUE(reg.resolve({16}) == nullptr);  // neither applies
}

ICL_REGISTER_TEST("utils.plugin-registry.applicability.empty_predicate_always_applies",
                  "an entry without an applicability predicate is always eligible")
{
  PluginRegistry<std::string, std::function<int()>, DepthCtx> reg;
  reg.registerPlugin("universal", []{ return 7; });  // no applicability
  ICL_TEST_EQ(reg.resolve({8})->payload(),  7);
  ICL_TEST_EQ(reg.resolve({42})->payload(), 7);
}

// ============================================================
// Forced override
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.forced.overrides_priority_and_applicability",
                  "setForced() makes resolve() return that key regardless of predicates")
{
  PluginRegistry<std::string, std::function<int()>, DepthCtx> reg;
  auto only32 = [](const DepthCtx& c) { return c.depth == 32; };
  reg.registerPlugin("fast32", []{ return 320; }, "", 10, only32);
  reg.registerPlugin("slow",   []{ return   1; }, "",  0);

  // In 8-bit context "fast32" wouldn't normally apply, but forcing wins.
  reg.setForced("fast32");
  ICL_TEST_EQ(reg.resolve({8})->key, std::string("fast32"));

  reg.clearForced();
  ICL_TEST_EQ(reg.resolve({8})->key, std::string("slow"));
}

// ============================================================
// Unregister + clear
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.get_or_throw",
                  "getOrThrow returns entry on hit, throws on miss")
{
  FunctionPluginRegistry<int()> reg;
  reg.registerPlugin("present", []{ return 7; });

  ICL_TEST_EQ(reg.getOrThrow("present").payload(), 7);
  ICL_TEST_THROW(reg.getOrThrow("missing"), ICLException);
}

ICL_REGISTER_TEST("utils.plugin-registry.unregister",
                  "unregisterPlugin removes an entry and reports success")
{
  FunctionPluginRegistry<void()> reg;
  reg.registerPlugin("a", []{});
  reg.registerPlugin("b", []{});

  ICL_TEST_TRUE(reg.unregisterPlugin("a"));
  ICL_TEST_FALSE(reg.has("a"));
  ICL_TEST_TRUE(reg.has("b"));
  ICL_TEST_FALSE(reg.unregisterPlugin("a"));  // already gone
}

ICL_REGISTER_TEST("utils.plugin-registry.unregister_clears_forced",
                  "removing the forced key clears the forced override")
{
  FunctionPluginRegistry<int()> reg;
  reg.registerPlugin("k", []{ return 1; });
  reg.setForced("k");
  ICL_TEST_TRUE(reg.forcedKey().has_value());
  reg.unregisterPlugin("k");
  ICL_TEST_FALSE(reg.forcedKey().has_value());
}

// ============================================================
// Thread-safety smoke test
// ============================================================

ICL_REGISTER_TEST("utils.plugin-registry.threading.concurrent_register_and_lookup",
                  "concurrent registration + lookup does not crash and preserves invariants")
{
  FunctionPluginRegistry<int()> reg(OnDuplicate::Replace);

  constexpr int N_THREADS  = 8;
  constexpr int N_PER_THREAD = 200;

  std::atomic<int> lookups{0};
  std::vector<std::thread> workers;

  for (int t = 0; t < N_THREADS; ++t) {
    workers.emplace_back([&, t] {
      for (int i = 0; i < N_PER_THREAD; ++i) {
        std::string key = "t" + std::to_string(t) + "_i" + std::to_string(i);
        int val = t * 1000 + i;
        reg.registerPlugin(key, [val] { return val; });
        // Interleave lookups.
        if (const auto* e = reg.get(key)) {
          if (e->payload() == val) lookups.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }
  for (auto& w : workers) w.join();

  ICL_TEST_EQ(static_cast<int>(reg.size()), N_THREADS * N_PER_THREAD);
  ICL_TEST_EQ(lookups.load(), N_THREADS * N_PER_THREAD);
}
