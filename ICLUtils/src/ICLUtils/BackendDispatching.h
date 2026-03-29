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

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <functional>

namespace icl {
  namespace utils {

    // ================================================================
    // Backend enum and priority — shared by all Context types
    // ================================================================

    enum class Backend : int { Cpp = 0, Simd = 1, Ipp = 2, OpenCL = 3 };

    inline const Backend backendPriority[] = {
      Backend::OpenCL, Backend::Ipp, Backend::Simd, Backend::Cpp
    };

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
    // Global Registry — type-erased, shared by all Context types
    // ================================================================

    namespace detail {
      struct RegistryEntry {
        Backend backend;
        std::string description;
        std::function<void(void*)> registerInto;
      };

      ICLUtils_API std::map<std::string, std::vector<RegistryEntry>>& globalRegistry();
      ICLUtils_API int addToRegistry(const std::string& key, RegistryEntry entry);
    }

    // ================================================================
    // BackendDispatching<Context> — mixin for classes with
    // backend-dispatched sub-operations.  All dispatch types
    // (BackendSelectorBase, BackendSelector, ApplicabilityFn)
    // are nested inside this template.
    // ================================================================

    template<class Context>
    struct BackendDispatching {
      virtual ~BackendDispatching() = default;

      // ---- Nested types ----

      using ApplicabilityFn = std::function<bool(const Context&)>;

      struct BackendSelectorBase {
        virtual ~BackendSelectorBase() = default;

        std::string name;

        virtual std::vector<Backend> registeredBackends() const = 0;
        virtual Backend bestBackendFor(const Context& ctx) const = 0;
        virtual std::vector<Backend> applicableBackendsFor(const Context& ctx) const = 0;

        std::optional<Backend> forcedBackend;
        void force(Backend b) { forcedBackend = b; }
        void unforce() { forcedBackend = std::nullopt; }
      };

      template<class Sig> struct BackendSelector;

      template<class R, class... Args>
      struct BackendSelector<R(Args...)> : BackendSelectorBase {

        struct ImplBase {
          Backend backend;
          std::string description;
          ApplicabilityFn applicabilityFn;

          virtual R apply(Args... args) = 0;
          virtual ~ImplBase() = default;

          bool applicableTo(const Context& ctx) const {
            return applicabilityFn ? applicabilityFn(ctx) : true;
          }
        };

        template<class F>
        struct Impl : ImplBase {
          F f;
          Impl(F f, Backend b, std::string desc, ApplicabilityFn app)
            : f(std::move(f)) {
            this->backend = b;
            this->description = std::move(desc);
            this->applicabilityFn = std::move(app);
          }
          R apply(Args... args) override {
            return f(std::forward<Args>(args)...);
          }
        };

        using ImplPtr = std::unique_ptr<ImplBase>;
        std::map<Backend, ImplPtr> impls;

        ImplBase* resolve(const Context& ctx) {
          if(this->forcedBackend) {
            auto it = impls.find(*this->forcedBackend);
            if(it != impls.end()) return it->second.get();
          }
          for(Backend b : backendPriority) {
            auto it = impls.find(b);
            if(it != impls.end() && it->second->applicableTo(ctx))
              return it->second.get();
          }
          return nullptr;
        }

        ImplBase* get(Backend b) {
          auto it = impls.find(b);
          return it != impls.end() ? it->second.get() : nullptr;
        }

        R callWith(Backend b, Args... args) {
          return impls.at(b)->apply(std::forward<Args>(args)...);
        }

        template<class F>
        void add(Backend b, F&& f, ApplicabilityFn applicability, std::string description = "") {
          if(description.empty()) description = std::string(backendName(b)) + " fallback";
          impls[b] = std::make_unique<Impl<std::decay_t<F>>>(
            std::forward<F>(f), b, std::move(description), std::move(applicability));
        }

        template<class F>
        void add(Backend b, F&& f, std::string description = "") {
          add(b, std::forward<F>(f), nullptr, std::move(description));
        }

        std::vector<Backend> registeredBackends() const override {
          std::vector<Backend> r;
          for(auto& [b, _] : impls) r.push_back(b);
          return r;
        }

        Backend bestBackendFor(const Context& ctx) const override {
          if(this->forcedBackend && impls.count(*this->forcedBackend)) return *this->forcedBackend;
          for(Backend b : backendPriority) {
            auto it = impls.find(b);
            if(it != impls.end() && it->second->applicableTo(ctx))
              return b;
          }
          return Backend::Cpp;
        }

        std::vector<Backend> applicableBackendsFor(const Context& ctx) const override {
          std::vector<Backend> r;
          for(auto& [b, impl] : impls)
            if(impl->applicableTo(ctx)) r.push_back(b);
          return r;
        }
      };

      // ---- Static registration helpers ----

      template<class Sig, class F>
      static int registerBackend(const std::string& key, Backend b, F&& f,
                                  ApplicabilityFn applicability,
                                  std::string desc = "") {
        return detail::addToRegistry(key, {
          b, desc,
          [f = std::forward<F>(f), applicability, desc, b](void* base) mutable {
            auto* sel = static_cast<BackendSelector<Sig>*>(base);
            sel->add(b, std::move(f), applicability, desc);
          }
        });
      }

      template<class Sig, class Factory>
      static int registerStatefulBackend(const std::string& key, Backend b, Factory&& factory,
                                          ApplicabilityFn applicability,
                                          std::string desc = "") {
        return detail::addToRegistry(key, {
          b, desc,
          [factory = std::forward<Factory>(factory), applicability, desc, b]
          (void* base) {
            auto* sel = static_cast<BackendSelector<Sig>*>(base);
            try {
              auto fn = factory();
              sel->add(b, std::move(fn), applicability, desc);
            } catch(const std::exception&) {}
          }
        });
      }

      template<class Sig>
      static void loadFromRegistry(const std::string& key, BackendSelector<Sig>& sel) {
        if(sel.name.empty()) sel.name = key;
        auto it = detail::globalRegistry().find(key);
        if(it != detail::globalRegistry().end()) {
          for(auto& entry : it->second) {
            entry.registerInto(&sel);
          }
        }
      }

      // ---- Instance methods ----

      void initDispatching(const std::string& className) {
        m_prefix = className + ".";
      }

      std::string qualifiedName(const std::string& shortName) const {
        return m_prefix + shortName;
      }

      std::vector<BackendSelectorBase*> selectors() {
        std::vector<BackendSelectorBase*> result;
        for(auto& sel : m_selectors) result.push_back(sel.get());
        return result;
      }

      BackendSelectorBase* selectorByName(const std::string& shortName) {
        auto it = m_selectorByName.find(shortName);
        return it != m_selectorByName.end() ? it->second : nullptr;
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

      template<class Sig>
      BackendSelector<Sig>& addSelector(const std::string& shortName) {
        std::string fullName = m_prefix + shortName;
        auto sel = std::make_unique<BackendSelector<Sig>>();
        sel->name = fullName;
        auto* ptr = sel.get();
        m_selectorByName[shortName] = ptr;
        m_selectors.push_back(std::move(sel));
        loadFromRegistry<Sig>(fullName, *ptr);
        return *ptr;
      }

      template<class Sig>
      BackendSelector<Sig>& getSelector(const std::string& shortName) {
        return *static_cast<BackendSelector<Sig>*>(m_selectorByName.at(shortName));
      }

    private:
      std::string m_prefix;
      std::vector<std::unique_ptr<BackendSelectorBase>> m_selectors;
      std::map<std::string, BackendSelectorBase*> m_selectorByName;
    };

  } // namespace utils
} // namespace icl
