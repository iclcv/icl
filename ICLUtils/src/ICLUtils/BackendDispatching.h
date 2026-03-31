/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/BackendDispatching.h             **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>

#include <array>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <stdexcept>
#include <type_traits>

namespace icl {
  namespace utils {

    // ================================================================
    // Backend enum — values encode priority (higher = preferred)
    // ================================================================

    enum class Backend : int { Cpp = 0, Simd = 1, Ipp = 2, OpenCL = 3 };

    inline constexpr int NUM_BACKENDS = 4;

    inline const char* backendName(Backend b) {
      switch(b) {
        case Backend::OpenCL: return "OpenCL";
        case Backend::Ipp:    return "IPP";
        case Backend::Simd:   return "SSE2/NEON";
        case Backend::Cpp:    return "C++";
      }
      return "?";
    }

    // ================================================================
    // BackendSelector — dispatch table for one operation.
    // Stores up to NUM_BACKENDS implementations in a fixed array.
    // ImplBase objects are shared_ptr so selectors can be cloned cheaply.
    // ================================================================

    template<class Context>
    using ApplicabilityFn = std::function<bool(const Context&)>;

    template<class Context>
    struct BackendSelectorBase {
      virtual ~BackendSelectorBase() = default;

      std::string name;
      std::optional<Backend> forcedBackend;

      void force(Backend b) { forcedBackend = b; }
      void unforce() { forcedBackend = std::nullopt; }

      virtual std::unique_ptr<BackendSelectorBase> clone() const = 0;
      virtual std::vector<Backend> registeredBackends() const = 0;
      virtual Backend bestBackendFor(const Context& ctx) const = 0;
      virtual std::vector<Backend> applicableBackendsFor(const Context& ctx) const = 0;
    };

    // Primary template declared so the partial specialization below can
    // decompose Sig = R(Args...) into its return type and argument types.
    template<class Context, class Sig> struct BackendSelector;

    template<class Context, class R, class... Args>
    struct BackendSelector<Context, R(Args...)> : BackendSelectorBase<Context> {

      // --- Per-backend implementation (type-erased callable) ---

      struct ImplBase {
        Backend backend;
        std::string description;
        ApplicabilityFn<Context> applicabilityFn;
        std::function<std::shared_ptr<ImplBase>()> cloneFn;  // set for stateful backends

        ImplBase(Backend b, std::string desc, ApplicabilityFn<Context> app)
          : backend(b), description(std::move(desc)), applicabilityFn(std::move(app)) {}
        virtual ~ImplBase() = default;

        virtual R apply(Args... args) = 0;

        bool applicableTo(const Context& ctx) const {
          return !applicabilityFn || applicabilityFn(ctx);
        }
      };

      template<class F>
      struct Impl : ImplBase {
        F f;
        Impl(F f, Backend b, std::string desc, ApplicabilityFn<Context> app)
          : ImplBase(b, std::move(desc), std::move(app)), f(std::move(f)) {}

        R apply(Args... args) override {
          return f(std::forward<Args>(args)...);
        }
      };

      // --- Storage: shared_ptr so cloning just bumps refcounts ---

      std::array<std::shared_ptr<ImplBase>, NUM_BACKENDS> impls{};

      template<class F>
      void add(Backend b, F&& f, ApplicabilityFn<Context> applicability,
               std::string description = "") {
        if(description.empty()) description = std::string(backendName(b)) + " fallback";
        impls[static_cast<int>(b)] = std::make_shared<Impl<std::decay_t<F>>>(
          std::forward<F>(f), b, std::move(description), std::move(applicability));
      }

      template<class F>
      void add(Backend b, F&& f, std::string description = "") {
        add(b, std::forward<F>(f), nullptr, std::move(description));
      }

      /// Add a stateful backend. Factory is called once per clone to create fresh state.
      /// Factory signature: () -> callable(Args...) -> R
      template<class Factory>
      void addStateful(Backend b, Factory&& factory,
                       ApplicabilityFn<Context> applicability,
                       std::string description = "") {
        if(description.empty()) description = std::string(backendName(b)) + " fallback";
        auto fn = factory();
        auto impl = std::make_shared<Impl<std::decay_t<decltype(fn)>>>(
          std::move(fn), b, description, applicability);
        impl->cloneFn = [factory = std::decay_t<Factory>(std::forward<Factory>(factory)),
                         b, description, applicability]() -> std::shared_ptr<ImplBase> {
          auto fn = factory();
          return std::make_shared<Impl<std::decay_t<decltype(fn)>>>(
            std::move(fn), b, description, applicability);
        };
        impls[static_cast<int>(b)] = std::move(impl);
      }

      // --- Resolution: iterate highest-priority first ---

      ImplBase* resolve(const Context& ctx) const {
        if(this->forcedBackend) {
          auto* p = impls[static_cast<int>(*this->forcedBackend)].get();
          if(p) return p;
        }
        for(int i = NUM_BACKENDS - 1; i >= 0; --i) {
          auto* p = impls[i].get();
          if(p && p->applicableTo(ctx)) return p;
        }
        return nullptr;
      }

      ImplBase* resolveOrThrow(const Context& ctx) const {
        auto* r = resolve(ctx);
        if(!r) throw std::logic_error("no applicable backend for '" + this->name + "'");
        return r;
      }

      ImplBase* get(Backend b) const {
        return impls[static_cast<int>(b)].get();
      }

      // --- Clone: stateful backends get fresh state, stateless share ImplBase ---

      std::unique_ptr<BackendSelectorBase<Context>> clone() const override {
        auto c = std::make_unique<BackendSelector>();
        c->name = this->name;
        for(int i = 0; i < NUM_BACKENDS; ++i) {
          if(impls[i]) {
            c->impls[i] = impls[i]->cloneFn ? impls[i]->cloneFn() : impls[i];
          }
        }
        return c;
      }

      // --- Introspection (tests / cross-validation) ---

      std::vector<Backend> registeredBackends() const override {
        std::vector<Backend> r;
        for(int i = 0; i < NUM_BACKENDS; ++i)
          if(impls[i]) r.push_back(static_cast<Backend>(i));
        return r;
      }

      Backend bestBackendFor(const Context& ctx) const override {
        auto* r = resolve(ctx);
        return r ? r->backend : Backend::Cpp;
      }

      std::vector<Backend> applicableBackendsFor(const Context& ctx) const override {
        std::vector<Backend> r;
        for(int i = 0; i < NUM_BACKENDS; ++i)
          if(impls[i] && impls[i]->applicableTo(ctx))
            r.push_back(static_cast<Backend>(i));
        return r;
      }
    };

    // ================================================================
    // BackendDispatching<Context> — owns multiple BackendSelectors.
    //
    // Two construction modes:
    //   1. Default — empty, add selectors via addSelector() (prototypes, singletons)
    //   2. Clone   — copies selectors from a prototype, sharing ImplBase objects
    //                but with independent forcedBackend state
    // ================================================================

    template<class Context>
    struct BackendDispatching {
      virtual ~BackendDispatching() = default;

      /// Default constructor — for prototypes and singletons
      BackendDispatching() = default;

      /// Clone constructor — creates independent selectors sharing ImplBase objects.
      /// Use as mandatory base-class initializer in filter constructors.
      explicit BackendDispatching(const BackendDispatching& proto) {
        for(auto& sel : proto.m_selectors) {
          m_selectors.push_back(sel->clone());
        }
      }

      // Disable copy-assignment (cloning is explicit via constructor)
      BackendDispatching& operator=(const BackendDispatching&) = delete;
      BackendDispatching(BackendDispatching&&) = default;
      BackendDispatching& operator=(BackendDispatching&&) = default;

      // ---- Selector setup (prototypes / singletons) ----

      /// Enum-keyed addSelector — asserts enum value matches insertion index.
      /// Selector name derived via ADL toString(K) for error messages.
      template<class Sig, class K,
               typename std::enable_if<std::is_enum<K>::value, int>::type = 0>
      BackendSelector<Context, Sig>& addSelector(K key) {
        if(static_cast<size_t>(key) != m_selectors.size())
          throw std::logic_error("addSelector: enum value " + std::to_string(static_cast<int>(key))
            + " does not match insertion index " + std::to_string(m_selectors.size()));
        auto sel = std::make_unique<BackendSelector<Context, Sig>>();
        sel->name = toString(key);
        auto* ptr = sel.get();
        m_selectors.push_back(std::move(sel));
        return *ptr;
      }

      // ---- Selector lookup ----

      /// Enum/index-based O(1) lookup
      template<class Sig, class K,
               typename std::enable_if<std::is_enum<K>::value || std::is_integral<K>::value, int>::type = 0>
      BackendSelector<Context, Sig>& getSelector(K key) {
        return *static_cast<BackendSelector<Context, Sig>*>(m_selectors[static_cast<size_t>(key)].get());
      }

      /// Backend proxy — binds a Backend enum so registration calls don't repeat it.
      /// Usage: auto cpp = proto.backends(Backend::Cpp); cpp.add<Sig>(Op::x, fn, ...);
      struct BackendProxy {
        BackendDispatching* self;
        Backend backend;

        template<class Sig, class K, class F,
                 typename std::enable_if<std::is_enum<K>::value, int>::type = 0>
        void add(K key, F&& f, ApplicabilityFn<Context> applicability,
                 std::string description = "") {
          self->template getSelector<Sig>(key).add(backend, std::forward<F>(f),
                                                   std::move(applicability), std::move(description));
        }

        template<class Sig, class K, class F,
                 typename std::enable_if<std::is_enum<K>::value, int>::type = 0>
        void add(K key, F&& f, std::string description = "") {
          self->template getSelector<Sig>(key).add(backend, std::forward<F>(f), std::move(description));
        }

        template<class Sig, class K, class Factory,
                 typename std::enable_if<std::is_enum<K>::value, int>::type = 0>
        void addStateful(K key, Factory&& factory,
                         ApplicabilityFn<Context> applicability,
                         std::string description = "") {
          self->template getSelector<Sig>(key).addStateful(backend, std::forward<Factory>(factory),
                                                           std::move(applicability), std::move(description));
        }
      };

      BackendProxy backends(Backend b) { return {this, b}; }

      /// Enum-keyed lookup returning base (introspection / tests)
      template<class K,
               typename std::enable_if<std::is_enum<K>::value, int>::type = 0>
      BackendSelectorBase<Context>* selector(K key) {
        return m_selectors[static_cast<size_t>(key)].get();
      }

      // ---- Bulk operations (tests / cross-validation) ----

      std::vector<BackendSelectorBase<Context>*> selectors() {
        std::vector<BackendSelectorBase<Context>*> result;
        for(auto& sel : m_selectors) result.push_back(sel.get());
        return result;
      }

      void forceAll(Backend b) {
        for(auto& sel : m_selectors) sel->force(b);
      }

      void unforceAll() {
        for(auto& sel : m_selectors) sel->unforce();
      }

      std::vector<std::vector<Backend>>
      allBackendCombinations(const Context& ctx) {
        std::vector<std::vector<Backend>> result;
        for(auto& sel : m_selectors)
          result.push_back(sel->applicableBackendsFor(ctx));
        return result;
      }

      template<class Fn>
      void forEachCombination(const std::vector<std::vector<Backend>>& perSelector, Fn&& fn) {
        std::vector<Backend> combo(perSelector.size());
        auto sels = selectors();
        std::function<void(size_t)> recurse = [&](size_t idx) {
          if(idx == perSelector.size()) {
            for(size_t i = 0; i < sels.size(); ++i) sels[i]->force(combo[i]);
            fn(combo);
            return;
          }
          for(Backend b : perSelector[idx]) {
            combo[idx] = b;
            recurse(idx + 1);
          }
        };
        recurse(0);
        for(auto* sel : sels) sel->unforce();
      }

    private:
      std::vector<std::unique_ptr<BackendSelectorBase<Context>>> m_selectors;
    };

  } // namespace utils
} // namespace icl
