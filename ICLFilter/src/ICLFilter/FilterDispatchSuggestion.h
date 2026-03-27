// Suggestion: how the template mechanics could work
// Builds on NewFilterDispatchingMechanismAndLogic.h
//
// Key insight: AlgoSwitch is templated on the FUNCTION SIGNATURE of the sub-op.
// This makes AlgoBase's virtual apply() type-safe per sub-op, without needing
// to type-erase the arguments.
//
// Each algo callable works at the Image level (not Img<T>). Depth dispatch
// happens INSIDE the algo implementation via visit/visitWith. This means the
// algo interface is uniform regardless of pixel type.

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <ICLCore/Image.h>

namespace icl::filter {

    enum class Backend { Cpp, Simd, Ipp, OpenCL };

    // Priority order for cascaded selection (highest priority first)
    inline constexpr Backend backendPriority[] = {
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
    // AlgoSwitchBase — non-templated base for introspection + testing
    // ================================================================
    struct AlgoSwitchBase {
        virtual ~AlgoSwitchBase() = default;

        std::string name;  // e.g. "ThresholdOp.ltVal"

        // Introspection: which backends are registered?
        virtual std::vector<Backend> registeredBackends() const = 0;

        // Introspection: which backend would handle this input?
        virtual Backend bestBackendFor(core::depth d, const core::ImgParams& p) const = 0;

        // Introspection: which backends support a given depth+params?
        virtual std::vector<Backend> supportedBackendsFor(core::depth d, const core::ImgParams& p) const = 0;

        // Testing: force a specific backend (nullopt = use cascade)
        std::optional<Backend> forcedBackend;
        void force(Backend b) { forcedBackend = b; }
        void unforce() { forcedBackend = std::nullopt; }
    };

    // ================================================================
    // AlgoSwitch<Sig> — typed dispatch table for one sub-operation
    // ================================================================
    // Sig is the function signature, e.g.:
    //   void(const Image& src, Image& dst, double threshold, double value)
    //
    // This resolves the "AlgoSwitch<?>" question: ? = the sub-op's signature.

    template<class Sig>
    struct AlgoSwitch;

    template<class R, class... Args>
    struct AlgoSwitch<R(Args...)> : AlgoSwitchBase {

        // Type-safe algo interface for this specific signature
        struct AlgoBase {
            std::string description;
            Backend backend;
            virtual R apply(Args... args) = 0;
            virtual bool supports(core::depth d, const core::ImgParams& p) const = 0;
            virtual ~AlgoBase() = default;
        };

        using AlgoPtr = std::unique_ptr<AlgoBase>;

        // Wrapper that type-erases any callable matching the signature
        template<class F>
        struct Algo : AlgoBase {
            F f;
            std::function<bool(core::depth, const core::ImgParams&)> supportsFn;

            Algo(F f, std::function<bool(core::depth, const core::ImgParams&)> sup,
                 Backend b, std::string desc)
                : f(std::move(f)), supportsFn(std::move(sup))
            {
                this->backend = b;
                this->description = std::move(desc);
            }

            R apply(Args... args) override {
                return f(std::forward<Args>(args)...);
            }
            bool supports(core::depth d, const core::ImgParams& p) const override {
                return supportsFn(d, p);
            }
        };

        // ---- The dispatch table ----
        std::map<Backend, AlgoPtr> algos;

        // Cascaded selection: walk priority order, return first that supports.
        // If forcedBackend is set, use that directly (for testing).
        AlgoBase* getBest(core::depth d, const core::ImgParams& params) {
            if(forcedBackend) {
                auto it = algos.find(*forcedBackend);
                if(it != algos.end()) return it->second.get();
            }
            for(Backend b : backendPriority) {
                auto it = algos.find(b);
                if(it != algos.end() && it->second->supports(d, params))
                    return it->second.get();
            }
            return nullptr; // shouldn't happen if Cpp is always registered
        }

        // Direct access to a specific backend (for testing)
        AlgoBase* get(Backend b) {
            auto it = algos.find(b);
            return it != algos.end() ? it->second.get() : nullptr;
        }

        // Convenience: dispatch and call in one step
        R operator()(core::depth d, const core::ImgParams& params, Args... args) {
            return getBest(d, params)->apply(std::forward<Args>(args)...);
        }

        // Direct call to a specific backend (for testing / cross-validation)
        R callWith(Backend b, Args... args) {
            return algos.at(b)->apply(std::forward<Args>(args)...);
        }

        // ---- Registration (called from constructor or self-registering .cpp files) ----
        template<class F>
        void add(Backend b, F&& f,
                 std::function<bool(core::depth, const core::ImgParams&)> supports,
                 std::string description = "") {
            algos[b] = std::make_unique<Algo<std::decay_t<F>>>(
                std::forward<F>(f), std::move(supports), b, std::move(description));
        }

        // Convenience: register an algo that supports all depths
        template<class F>
        void add(Backend b, F&& f, std::string description = "") {
            add(b, std::forward<F>(f),
                [](core::depth, const core::ImgParams&) { return true; },
                std::move(description));
        }

        // ---- Introspection (AlgoSwitchBase interface) ----
        std::vector<Backend> registeredBackends() const override {
            std::vector<Backend> result;
            for(auto& [b, _] : algos) result.push_back(b);
            return result;
        }

        Backend bestBackendFor(core::depth d, const core::ImgParams& p) const override {
            for(Backend b : backendPriority) {
                auto it = algos.find(b);
                if(it != algos.end() && it->second->supports(d, p))
                    return b;
            }
            return Backend::Cpp;
        }

        std::vector<Backend> supportedBackendsFor(core::depth d, const core::ImgParams& p) const override {
            std::vector<Backend> result;
            for(auto& [b, algo] : algos)
                if(algo->supports(d, p)) result.push_back(b);
            return result;
        }
    };

    // ================================================================
    // Global Registry — enables self-registration from backend .cpp files
    // ================================================================
    // Backend files can register algo factories without being #included
    // by the filter. The filter constructor calls loadFromRegistry().
    //
    // Usage from backend .cpp:
    //   REGISTER_ALGO("ThresholdOp.ltVal", Backend::Ipp, ipp_ltval_fn, supports_fn, "IPP 8u/16s/32f");
    //
    // Usage from filter constructor:
    //   ltVal.loadFromRegistry("ThresholdOp.ltVal");

    namespace detail {
        struct RegistryEntry {
            Backend backend;
            std::string description;
            std::function<bool(core::depth, const core::ImgParams&)> supports;
            // Type-erased factory: called with an AlgoSwitchBase* to register into
            std::function<void(AlgoSwitchBase*)> registerInto;
        };

        inline std::map<std::string, std::vector<RegistryEntry>>& globalRegistry() {
            static std::map<std::string, std::vector<RegistryEntry>> reg;
            return reg;
        }

        inline void registerAlgo(const std::string& key, RegistryEntry entry) {
            globalRegistry()[key].push_back(std::move(entry));
        }
    }

    // Typed registration helper: captures the callable + signature match
    template<class Sig, class F>
    void registerAlgo(const std::string& key, Backend b, F&& f,
                      std::function<bool(core::depth, const core::ImgParams&)> supports,
                      std::string desc = "") {
        detail::registerAlgo(key, {
            b, desc, supports,
            [f = std::forward<F>(f), supports, desc, b](AlgoSwitchBase* base) mutable {
                auto* sw = static_cast<AlgoSwitch<Sig>*>(base);
                sw->add(b, std::move(f), supports, desc);
            }
        });
    }

    // Load all registered algos for a given key into an AlgoSwitch
    template<class Sig>
    void loadFromRegistry(const std::string& key, AlgoSwitch<Sig>& sw) {
        sw.name = key;
        auto it = detail::globalRegistry().find(key);
        if(it != detail::globalRegistry().end()) {
            for(auto& entry : it->second) {
                entry.registerInto(&sw);
            }
        }
    }

    // Macro for convenient self-registration from backend .cpp files
    #define ICL_REGISTER_ALGO(KEY, SIG, BACKEND, FN, SUPPORTS, DESC) \
        static const bool _icl_reg_##__LINE__ = (                    \
            icl::filter::registerAlgo<SIG>(KEY, BACKEND, FN, SUPPORTS, DESC), true)

    // ================================================================
    // DispatchingFilter — mixin for filters with inspectable sub-ops
    // ================================================================
    struct DispatchingFilter {
        virtual ~DispatchingFilter() = default;
        virtual std::vector<AlgoSwitchBase*> switches() = 0;

        // Print all backends for all sub-ops (debugging/introspection)
        void printBackends() {
            for(auto* sw : switches()) {
                std::cout << sw->name << ":";
                for(auto b : sw->registeredBackends())
                    std::cout << " " << backendName(b);
                std::cout << "\n";
            }
        }

        // Force all switches to a specific backend (for testing)
        void forceAll(Backend b) {
            for(auto* sw : switches()) sw->force(b);
        }
        void unforceAll() {
            for(auto* sw : switches()) sw->unforce();
        }

        // Enumerate all valid backend combinations for a given depth+params.
        // Returns one vector<Backend> per switch, containing only backends
        // that support the input. Use for exhaustive cross-validation testing.
        //
        // Example usage in tests:
        //   auto combos = allBackendCombinations(src.getDepth(), src.getParams());
        //   forEachCombination(combos, [&](const std::vector<Backend>& combo) {
        //       auto sws = switches();
        //       for(size_t i = 0; i < sws.size(); ++i) sws[i]->force(combo[i]);
        //       Image dst = op.apply(src);
        //       // compare dst against reference ...
        //       unforceAll();
        //   });
        std::vector<std::vector<Backend>>
        allBackendCombinations(core::depth d, const core::ImgParams& p) {
            std::vector<std::vector<Backend>> perSwitch;
            for(auto* sw : switches())
                perSwitch.push_back(sw->supportedBackendsFor(d, p));
            return perSwitch;
        }

        // Helper: iterate all cartesian products of backend combinations.
        // Calls fn(combo) for each combination where combo[i] is the backend
        // for switches()[i].
        template<class Fn>
        void forEachCombination(const std::vector<std::vector<Backend>>& perSwitch, Fn&& fn) {
            std::vector<Backend> combo(perSwitch.size());
            std::function<void(size_t)> recurse = [&](size_t idx) {
                if(idx == perSwitch.size()) { fn(combo); return; }
                for(Backend b : perSwitch[idx]) {
                    combo[idx] = b;
                    recurse(idx + 1);
                }
            };
            recurse(0);
        }
    };

    // ================================================================
    // EXAMPLE: ThresholdOp (sketch of how it would look)
    // ================================================================
    //
    // --- ThresholdOp.h (public, unchanged except adding DispatchingFilter) ---
    //
    //   class ThresholdOp : public UnaryOp, public DispatchingFilter { ... };
    //
    // --- ThresholdOp.cpp ---
    //
    //   // Sub-op signature: all threshold sub-ops share this
    //   using ThreshSig = void(const core::Image& src, core::Image& dst,
    //                          double threshold, double value);
    //
    //   // C++ fallback (always registered inline — the one algo we always have)
    //   static void cpp_ltval(const Image& src, Image& dst, double t, double v) {
    //       src.visitWith(dst, [&](const auto& s, auto& d) {
    //           using T = typename std::remove_reference_t<decltype(s)>::type;
    //           T tt = static_cast<T>(t), vv = static_cast<T>(v);
    //           visitROILinesPerChannelWith(s, d, [tt, vv](const T* sp, T* dp, int, int w) {
    //               for(int i = 0; i < w; ++i) dp[i] = sp[i] < tt ? vv : sp[i];
    //           });
    //       });
    //   }
    //
    //   ThresholdOp::ThresholdOp() {
    //       // Always-present fallback
    //       ltVal.add(Backend::Cpp, cpp_ltval, "C++ fallback for all depths");
    //       // Pull in any self-registered backends (IPP, SSE, OpenCL)
    //       loadFromRegistry("ThresholdOp.ltVal", ltVal);
    //       loadFromRegistry("ThresholdOp.gtVal", gtVal);
    //   }
    //
    //   void ThresholdOp::apply(const Image& src, Image& dst) {
    //       if(!prepare(dst, src)) return;
    //       switch(m_eType) {
    //           case ltVal:
    //               ltVal(src.getDepth(), src.getParams(), src, dst, m_threshold, m_value);
    //               break;
    //           // ...
    //       }
    //   }
    //
    // --- ThresholdOp_Ipp.cpp (self-registering, no #include of ThresholdOp.h needed) ---
    //
    //   #include <ICLFilter/FilterDispatch.h>  // for ICL_REGISTER_ALGO
    //   #ifdef ICL_HAVE_IPP
    //   #include <ipp.h>
    //
    //   static void ipp_ltval(const Image& src, Image& dst, double t, double v) {
    //       src.visitWith(dst, [&](const auto& s, auto& d) {
    //           using T = typename std::remove_reference_t<decltype(s)>::type;
    //           if constexpr (std::is_same_v<T, icl8u>)
    //               ippiThreshold_LTVal_8u_C1R(/* ... */);
    //           else if constexpr (std::is_same_v<T, icl32f>)
    //               ippiThreshold_LTVal_32f_C1R(/* ... */);
    //           // other depths: shouldn't be called (supports() prevents it)
    //       });
    //   }
    //
    //   ICL_REGISTER_ALGO(
    //       "ThresholdOp.ltVal",
    //       ThreshSig,
    //       Backend::Ipp,
    //       ipp_ltval,
    //       [](depth d, const ImgParams&) {
    //           return d == depth8u || d == depth16s || d == depth32f;
    //       },
    //       "IPP threshold LTVal (8u/16s/32f)"
    //   );
    //
    //   #endif
    //
    // ================================================================

} // namespace icl::filter
