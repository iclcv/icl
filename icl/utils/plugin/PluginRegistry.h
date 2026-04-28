// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace icl::utils {

  /// Tag type used as the Context template parameter when the registry
  /// does not need applicability predicates. Pass `{}` to resolve().
  struct NoContext {};

  namespace plugin_registry_detail {
    /// SFINAE helper: is `os << t` well-formed?
    template <class, class = void>
    struct is_streamable : std::false_type {};
    template <class T>
    struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>()
                                                 << std::declval<const T&>())>>
      : std::true_type {};
  } // namespace plugin_registry_detail

  // ================================================================
  // PluginRegistry — one primitive for all named-factory registries.
  //
  // Design follows BackendSelector's entry shape:
  //   - keyed entries (one entry per Key)
  //   - each entry carries an optional applicability predicate,
  //     description, and priority (higher = preferred)
  //   - lookup either exact by Key (`get`) or by priority+applicability
  //     across all entries (`resolve`)
  //
  // Payload is entirely caller-defined: for class-plugins it's typically
  // `std::function<std::unique_ptr<T>(CtorArgs...)>`; for function-plugins
  // it's `std::function<Signature>`. The two aliases below capture the
  // canonical forms.
  //
  // Use via one of the aliases (ClassPluginRegistry, FunctionPluginRegistry)
  // plus the ICL_REGISTER_PLUGIN macro for self-registration at library
  // load time.
  // ================================================================

  template <class Context>
  using PluginApplicabilityFn = std::function<bool(const Context&)>;

  /// Registration policy for duplicate keys.
  enum class OnDuplicate {
    Throw,                //!< registerPlugin() throws ICLException on duplicate
    KeepFirst,            //!< silently ignore re-registrations of the same key
    Replace,              //!< overwrite the existing entry
    KeepHighestPriority,  //!< keep whichever entry has the strictly-higher
                          //!< priority; on ties behaves like KeepFirst. Used
                          //!< by FileWriter/FileGrabber to let libpng (prio 0)
                          //!< take precedence over ImageMagick (prio -10) for
                          //!< overlapping extensions, deterministically across
                          //!< dyld static-init orderings.
  };

  template <class Key, class Payload, class Context = NoContext>
  class PluginRegistry {
  public:
    using KeyType         = Key;
    using PayloadType     = Payload;
    using ContextType     = Context;
    using ApplicabilityFn = PluginApplicabilityFn<Context>;

    /// A registered entry. Returned from get() / resolve() / entries().
    struct Entry {
      Key             key;
      Payload         payload;
      std::string     description;
      ApplicabilityFn applicability;  //!< empty = always applicable
      int             priority = 0;   //!< higher = preferred
    };

    explicit PluginRegistry(OnDuplicate onDuplicate = OnDuplicate::Throw)
      : m_onDuplicate(onDuplicate) {}

    PluginRegistry(const PluginRegistry&) = delete;
    PluginRegistry& operator=(const PluginRegistry&) = delete;

    // ------------------------------------------------------------------
    // Registration
    // ------------------------------------------------------------------

    /// Register with optional metadata. Thread-safe.
    /// Behaviour on duplicate keys is governed by the ctor's OnDuplicate.
    void registerPlugin(Key key,
                        Payload payload,
                        std::string description = {},
                        int priority = 0,
                        ApplicabilityFn applicability = {}) {
      std::scoped_lock lock(m_mutex);

      auto it = findByKey(key);
      if (it != m_entries.end()) {
        switch (m_onDuplicate) {
          case OnDuplicate::Throw: {
            std::ostringstream s;
            s << "PluginRegistry: key already registered";
            if constexpr (plugin_registry_detail::is_streamable<Key>::value) {
              s << ": " << key;
            }
            throw ICLException(s.str());
          }
          case OnDuplicate::KeepFirst:
            return;
          case OnDuplicate::Replace:
            m_entries.erase(it);
            break;
          case OnDuplicate::KeepHighestPriority:
            if (priority > it->priority) {
              m_entries.erase(it);
              break;
            }
            return;  // existing priority wins (including ties)
        }
      }

      m_entries.push_back(Entry{
          std::move(key), std::move(payload),
          std::move(description), std::move(applicability), priority});
      sortByPriorityDesc();
    }

    /// Remove an entry by key. Returns true if removed.
    bool unregisterPlugin(const Key& key) {
      std::scoped_lock lock(m_mutex);
      auto it = findByKey(key);
      if (it == m_entries.end()) return false;
      m_entries.erase(it);
      if (m_forced && *m_forced == key) m_forced.reset();
      return true;
    }

    // ------------------------------------------------------------------
    // Lookup
    // ------------------------------------------------------------------

    /// Exact-match by key. Returns nullptr if not registered.
    /// The returned pointer stays valid until the entry is removed /
    /// replaced / the registry is destroyed.
    [[nodiscard]] const Entry* get(const Key& key) const {
      std::scoped_lock lock(m_mutex);
      auto it = findByKey(key);
      return it == m_entries.end() ? nullptr : &*it;
    }

    /// Variant of get() that throws on missing key. Convenience for
    /// classic registries whose consumers typically expect exact-match
    /// lookups to succeed (e.g. CompressionPlugin by codec name).
    [[nodiscard]] const Entry& getOrThrow(const Key& key) const {
      if (const Entry* e = get(key)) return *e;
      std::ostringstream s;
      s << "PluginRegistry::getOrThrow: no entry for key";
      if constexpr (plugin_registry_detail::is_streamable<Key>::value) {
        s << " '" << key << "'";
      }
      throw ICLException(s.str());
    }

    /// Highest-priority applicable entry for the given context.
    /// Honors forcedKey() if set (returns that key's entry directly,
    /// ignoring applicability). Returns nullptr if none applicable.
    [[nodiscard]] const Entry* resolve(const Context& ctx = {}) const {
      std::scoped_lock lock(m_mutex);
      if (m_forced) {
        auto it = findByKey(*m_forced);
        if (it != m_entries.end()) return &*it;
      }
      for (const auto& e : m_entries) {
        if (!e.applicability || e.applicability(ctx)) return &e;
      }
      return nullptr;
    }

    /// Variant of resolve() that throws on no match.
    [[nodiscard]] const Entry& resolveOrThrow(const Context& ctx = {}) const {
      if (const Entry* e = resolve(ctx)) return *e;
      throw ICLException("PluginRegistry::resolve: no applicable entry");
    }

    // ------------------------------------------------------------------
    // Introspection
    // ------------------------------------------------------------------

    [[nodiscard]] bool has(const Key& key) const {
      std::scoped_lock lock(m_mutex);
      return findByKey(key) != m_entries.end();
    }

    [[nodiscard]] std::size_t size() const {
      std::scoped_lock lock(m_mutex);
      return m_entries.size();
    }

    [[nodiscard]] bool empty() const { return size() == 0; }

    /// All registered keys, in priority-descending order.
    [[nodiscard]] std::vector<Key> keys() const {
      std::scoped_lock lock(m_mutex);
      std::vector<Key> out;
      out.reserve(m_entries.size());
      for (const auto& e : m_entries) out.push_back(e.key);
      return out;
    }

    /// Snapshot of all entries, priority-descending.
    [[nodiscard]] std::vector<Entry> entries() const {
      std::scoped_lock lock(m_mutex);
      return m_entries;
    }

    // ------------------------------------------------------------------
    // Forced-key override (primarily for testing / debugging)
    // ------------------------------------------------------------------

    void setForced(Key key) {
      std::scoped_lock lock(m_mutex);
      m_forced = std::move(key);
    }

    void clearForced() {
      std::scoped_lock lock(m_mutex);
      m_forced.reset();
    }

    [[nodiscard]] std::optional<Key> forcedKey() const {
      std::scoped_lock lock(m_mutex);
      return m_forced;
    }

    /// Remove all entries. Primarily for tests.
    void clear() {
      std::scoped_lock lock(m_mutex);
      m_entries.clear();
      m_forced.reset();
    }

  private:
    typename std::vector<Entry>::iterator findByKey(const Key& key) {
      return std::find_if(m_entries.begin(), m_entries.end(),
                          [&](const Entry& e) { return e.key == key; });
    }
    typename std::vector<Entry>::const_iterator findByKey(const Key& key) const {
      return std::find_if(m_entries.cbegin(), m_entries.cend(),
                          [&](const Entry& e) { return e.key == key; });
    }

    /// Stable sort by priority desc (preserves insertion order on ties).
    void sortByPriorityDesc() {
      std::stable_sort(m_entries.begin(), m_entries.end(),
                       [](const Entry& a, const Entry& b) {
                         return a.priority > b.priority;
                       });
    }

    mutable std::mutex  m_mutex;
    OnDuplicate         m_onDuplicate;
    std::vector<Entry>  m_entries;
    std::optional<Key>  m_forced;
  };

  // ================================================================
  // Canonical aliases
  // ================================================================

  /// Payload is a callable of the given signature. Use for plugins
  /// whose entire contract is a single function (e.g. ImageOutput::send,
  /// file writers, file readers).
  template <class Signature>
  using FunctionPluginRegistry =
      PluginRegistry<std::string, std::function<Signature>>;

  /// Payload is a factory that produces a fresh std::unique_ptr<T>.
  /// Use for plugins that are multi-method objects (e.g. CompressionPlugin,
  /// Grabber, PointCloudGrabber).
  template <class T, class... CtorArgs>
  using ClassPluginRegistry =
      PluginRegistry<std::string,
                     std::function<std::unique_ptr<T>(CtorArgs...)>>;

  // ================================================================
  // Self-registration macro
  //
  // Uses __attribute__((constructor, used)) — the only macOS-portable
  // way to guarantee invocation at dyld time without risk of the object
  // being dead-stripped. See Session 47 notes in CLAUDE.md.
  //
  // TAG must be a C identifier unique within the translation unit
  // (typically "backendname" or "domain_backendname"). The variadic
  // arguments are forwarded as-is to registerPlugin(...), so the caller
  // supplies (key, payload, [description, [priority, [applicability]]]).
  //
  // Example:
  //   ICL_REGISTER_PLUGIN(CompressionRegister::instance(), jpeg,
  //                       "jpeg",
  //                       []{ return std::make_unique<JpegPlugin>(); },
  //                       "JPEG lossy codec");
  // ================================================================

  #define ICL_PLUGIN_REGISTRY_CONCAT2(a, b) a##b
  #define ICL_PLUGIN_REGISTRY_CONCAT(a, b) ICL_PLUGIN_REGISTRY_CONCAT2(a, b)

  #define ICL_REGISTER_PLUGIN(REGISTRY_EXPR, TAG, ...)                    \
    extern "C" __attribute__((constructor, used)) void                    \
    ICL_PLUGIN_REGISTRY_CONCAT(iclRegisterPlugin_,                        \
      ICL_PLUGIN_REGISTRY_CONCAT(TAG##_, __LINE__))() {                   \
      (REGISTRY_EXPR).registerPlugin(__VA_ARGS__);                        \
    }

} // namespace icl::utils
