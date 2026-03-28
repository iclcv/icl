/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/BackendDispatch.h                  **
** Module : ICLCore                                                **
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
#include <ICLCore/Image.h>

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <functional>

namespace icl {
  namespace core {

    /// Backend identifiers, ordered by priority (highest = OpenCL)
    enum class Backend : int { Cpp = 0, Simd = 1, Ipp = 2, OpenCL = 3 };

    /// Walk order for cascaded selection (highest priority first)
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

    /// Applicability check: takes the source Image so backends
    /// can check depth, ROI, size, channels, etc.
    using ApplicabilityFn = std::function<bool(const Image&)>;

    /// Predefined applicability: matches if source depth is any of the listed types.
    ///   applicableTo<icl8u, icl32f>  — matches depth8u or depth32f
    template<class... Ts>
    bool applicableTo(const Image& src) {
      depth d = src.getDepth();
      return ((d == getDepth<Ts>()) || ...);
    }

    // ================================================================
    // BackendSelectorBase — non-templated base for introspection
    // ================================================================
    struct ICLCore_API BackendSelectorBase {
      virtual ~BackendSelectorBase() = default;

      std::string name;

      /// Which backends are registered?
      virtual std::vector<Backend> registeredBackends() const = 0;

      /// Which backend would handle this image?
      virtual Backend bestBackendFor(const Image& src) const = 0;

      /// All backends that are applicable for this image
      virtual std::vector<Backend> applicableBackendsFor(const Image& src) const = 0;

      /// Force a specific backend (for testing). nullopt = use cascade.
      std::optional<Backend> forcedBackend;
      void force(Backend b) { forcedBackend = b; }
      void unforce() { forcedBackend = std::nullopt; }
    };

    // ================================================================
    // BackendSelector<Sig> — typed dispatch table for one sub-operation
    // ================================================================
    //
    // Template parameter is the function signature of the sub-op:
    //   BackendSelector<void(const Image&, Image&, double, double)>
    //
    template<class Sig> struct BackendSelector;

    template<class R, class... Args>
    struct BackendSelector<R(Args...)> : BackendSelectorBase {

      /// Type-safe virtual interface for this sub-op's signature
      struct ImplBase {
        Backend backend;
        std::string description;
        ApplicabilityFn applicabilityFn;

        virtual R apply(Args... args) = 0;
        virtual ~ImplBase() = default;

        bool applicableTo(const Image& src) const {
          return applicabilityFn ? applicabilityFn(src) : true;
        }
      };

      /// Wraps any callable matching the signature
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

      // ---- The dispatch table ----
      std::map<Backend, ImplPtr> impls;

      /// Cascaded selection: respects forcedBackend, then walks priority.
      ImplBase* resolve(const Image& src) {
        if(forcedBackend) {
          auto it = impls.find(*forcedBackend);
          if(it != impls.end()) return it->second.get();
        }
        for(Backend b : backendPriority) {
          auto it = impls.find(b);
          if(it != impls.end() && it->second->applicableTo(src))
            return it->second.get();
        }
        return nullptr;
      }

      /// Direct access to a specific backend (for testing)
      ImplBase* get(Backend b) {
        auto it = impls.find(b);
        return it != impls.end() ? it->second.get() : nullptr;
      }

      /// Call a specific backend directly (for cross-validation testing)
      R callWith(Backend b, Args... args) {
        return impls.at(b)->apply(std::forward<Args>(args)...);
      }

      // ---- Registration ----

      template<class F>
      void add(Backend b, F&& f, ApplicabilityFn applicability, std::string description = "") {
        if(description.empty()) description = std::string(backendName(b)) + " fallback";
        impls[b] = std::make_unique<Impl<std::decay_t<F>>>(
          std::forward<F>(f), b, std::move(description), std::move(applicability));
      }

      /// Register an impl that is applicable to all inputs
      template<class F>
      void add(Backend b, F&& f, std::string description = "") {
        add(b, std::forward<F>(f), nullptr, std::move(description));
      }

      // ---- Introspection ----

      std::vector<Backend> registeredBackends() const override {
        std::vector<Backend> r;
        for(auto& [b, _] : impls) r.push_back(b);
        return r;
      }

      Backend bestBackendFor(const Image& src) const override {
        if(forcedBackend && impls.count(*forcedBackend)) return *forcedBackend;
        for(Backend b : backendPriority) {
          auto it = impls.find(b);
          if(it != impls.end() && it->second->applicableTo(src))
            return b;
        }
        return Backend::Cpp;
      }

      std::vector<Backend> applicableBackendsFor(const Image& src) const override {
        std::vector<Backend> r;
        for(auto& [b, impl] : impls)
          if(impl->applicableTo(src)) r.push_back(b);
        return r;
      }
    };

    // ================================================================
    // Global Registry — enables self-registration from backend .cpp files
    // ================================================================

    namespace detail {
      struct RegistryEntry {
        Backend backend;
        std::string description;
        ApplicabilityFn applicability;
        /// Type-erased callback: registers the impl into a given BackendSelectorBase*
        std::function<void(BackendSelectorBase*)> registerInto;
      };

      ICLCore_API std::map<std::string, std::vector<RegistryEntry>>& globalRegistry();
      ICLCore_API int addToRegistry(const std::string& key, RegistryEntry entry);
    }

    /// Typed registration: captures callable + signature match
    template<class Sig, class F>
    int registerBackend(const std::string& key, Backend b, F&& f,
                        ApplicabilityFn applicability, std::string desc = "") {
      return detail::addToRegistry(key, {
        b, desc, applicability,
        [f = std::forward<F>(f), applicability, desc, b](BackendSelectorBase* base) mutable {
          auto* sel = static_cast<BackendSelector<Sig>*>(base);
          sel->add(b, std::move(f), applicability, desc);
        }
      });
    }

    /// Stateful backend registration: factory is called once per filter instance.
    /// Factory must return a callable matching Sig (typically a lambda capturing
    /// a shared_ptr to its own state).
    ///
    /// Example:
    ///   registerStatefulBackend<MySig>("Op.sub", Backend::OpenCL, []() {
    ///       auto state = std::make_shared<MyOCLState>();
    ///       return [state](const Image& src, Image& dst, ...) {
    ///           state->apply(src, dst, ...);
    ///       };
    ///   }, applicableTo<icl8u, icl32f>, "OpenCL description");
    ///
    template<class Sig, class Factory>
    int registerStatefulBackend(const std::string& key, Backend b, Factory&& factory,
                                ApplicabilityFn applicability, std::string desc = "") {
      return detail::addToRegistry(key, {
        b, desc, applicability,
        [factory = std::forward<Factory>(factory), applicability, desc, b]
        (BackendSelectorBase* base) {
          auto* sel = static_cast<BackendSelector<Sig>*>(base);
          try {
            auto fn = factory();
            sel->add(b, std::move(fn), applicability, desc);
          } catch(const std::exception&) {
            // Backend unavailable (e.g. no OpenCL device) — skip silently
          }
        }
      });
    }

    /// Load all registered backends for a given key into a BackendSelector
    template<class Sig>
    void loadFromRegistry(const std::string& key, BackendSelector<Sig>& sel) {
      if(sel.name.empty()) sel.name = key;
      auto it = detail::globalRegistry().find(key);
      if(it != detail::globalRegistry().end()) {
        for(auto& entry : it->second) {
          entry.registerInto(&sel);
        }
      }
    }

    /// Macro for self-registration from backend .cpp files
    #define ICL_REGISTER_BACKEND(KEY, SIG, BACKEND, FN, APPLICABILITY, DESC) \
      static const int _icl_backend_reg_##__COUNTER__ =                      \
        icl::core::registerBackend<SIG>(KEY, BACKEND, FN, APPLICABILITY, DESC)

    // ================================================================
    // Dispatching — mixin for classes with backend-dispatched sub-ops
    // ================================================================
    //
    // Owns all BackendSelector instances via internal storage.
    // Subclasses use addSelector<Sig>(name) in their constructor and
    // getSelector<Sig>(name) in apply(). No BackendSelector members
    // leak into the subclass header.

    struct ICLCore_API Dispatching {
      virtual ~Dispatching();

      /// Set the scope prefix (e.g. "ThresholdOp").
      /// addSelector("ltVal") becomes "ThresholdOp.ltVal" in the registry.
      void initDispatching(const std::string& className);

      /// Qualified name for a selector (prefix + short name)
      std::string qualifiedName(const std::string& shortName) const;

      /// All registered selectors
      std::vector<BackendSelectorBase*> selectors();

      /// Look up a selector by short name (returns nullptr if not found)
      BackendSelectorBase* selectorByName(const std::string& shortName);

      /// Force all selectors to one backend
      void forceAll(Backend b);

      /// Clear all forced backends
      void unforceAll();

      /// Per-selector list of applicable backends for given input
      std::vector<std::vector<Backend>>
      allBackendCombinations(const Image& src);

      /// Iterate all cartesian products of backend combinations.
      /// combo[i] = backend for selectors()[i].
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

    protected:
      /// Create a new BackendSelector, load self-registered backends, return typed ref.
      template<class Sig>
      BackendSelector<Sig>& addSelector(const std::string& shortName) {
        std::string fullName = m_prefix + shortName;
        auto sel = std::make_unique<BackendSelector<Sig>>();
        sel->name = fullName;
        auto* ptr = static_cast<BackendSelector<Sig>*>(sel.get());
        m_selectorByName[shortName] = ptr;
        m_selectors.push_back(std::move(sel));
        loadFromRegistry<Sig>(fullName, *ptr);
        return *ptr;
      }

      /// Retrieve a previously added selector by short name.
      template<class Sig>
      BackendSelector<Sig>& getSelector(const std::string& shortName) {
        return *static_cast<BackendSelector<Sig>*>(m_selectorByName.at(shortName));
      }

    private:
      std::string m_prefix;
      std::vector<std::unique_ptr<BackendSelectorBase>> m_selectors;
      std::map<std::string, BackendSelectorBase*> m_selectorByName;
    };

  } // namespace core
} // namespace icl
