// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/AnyMap.h>
#include <icl/utils/AssignRegistry.h>
#include <icl/utils/AutoParse.h>
#include <icl/utils/Exception.h>
#include <icl/utils/StringUtils.h>
#include <icl/qt/GLCallback.h>
#include <icl/qt/MouseEvent.h>

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <typeinfo>
#include <vector>

namespace icl::qt {
  class MouseHandler;

  /// Heterogeneous keyed store for GUI handles (and anything else a
  /// caller wants to keep per string key).
  ///
  /// Holds a `utils::AnyMap` (`std::map<std::string, std::any>`) for
  /// values and a recursive mutex for serialization; both are held by
  /// `shared_ptr` so shallow-copies of a DataStore share the backing
  /// state — matching the old `MultiTypeMap` semantics that GUI /
  /// GUIImpl relied on.
  ///
  /// All cross-type dispatch (`store["k"] = 42` when the stored value
  /// is a `SliderHandle`, or `int v = store["k"]` in the reverse
  /// direction) goes through `utils::AssignRegistry`.  DataStore
  /// itself owns no rule table.
  class ICLQt_API DataStore {
  public:
    DataStore();
    ~DataStore() = default;

    /// Thrown by `operator[]` when the key isn't present.
    struct KeyNotFoundException : public utils::ICLException {
      KeyNotFoundException(const std::string &key)
        : utils::ICLException("Key not found: " + key) {}
    };

    /// Thrown by `Slot::operator=` / `Slot::as<T>()` when no
    /// `AssignRegistry` rule exists for the requested pair.
    struct UnassignableTypesException : public utils::ICLException {
      UnassignableTypesException(const std::string &tSrc, const std::string &tDst)
        : utils::ICLException("Unable to assign " + tDst + " = " + tSrc) {}
    };

    /// Value-proxy returned from `operator[]` — a lightweight view
    /// over one std::any entry in the underlying `AnyMap`.
    class Slot {
      std::any *m_entry;

      friend class DataStore;

      inline Slot(std::any *entry) : m_entry(entry) {}

      /// Run an `AssignRegistry::dispatch` on the typed payloads
      /// behind two std::any objects, translating a miss into an
      /// `UnassignableTypesException` for API stability.
      ICLQt_API static void assignAny(std::any &dst, std::any &src);

    public:
      /// Assign an instance of `T` into this entry.  Dispatch picks
      /// the registered `Assign<Stored, T>::apply` via the runtime
      /// `AssignRegistry`.
      ///
      /// `std::in_place_type<T>` forces std::any to store exactly
      /// `T` — necessary because some types (e.g. `utils::Any` which
      /// publicly inherits `std::string` and has a templated converting
      /// ctor) otherwise trigger an ambiguous-overload error in
      /// std::any's own templated ctor.
      template<class T>
      inline void operator=(const T &t) {
        std::any src(std::in_place_type<T>, t);
        assignAny(*m_entry, src);
      }

      /// Extract the entry's value as `T`.  Creates a default-
      /// constructed `T` and invokes the registered
      /// `Assign<T, Stored>::apply` rule through a copy of the
      /// stored payload (so the stored value is never mutated).
      template<class T>
      inline T as() const {
        std::any dst(std::in_place_type<T>, T{});
        std::any src = *m_entry;  // copy: dispatch might mutate src in-place
        assignAny(dst, src);
        return std::any_cast<T>(dst);
      }

      /// Implicit conversion to `T`.  Three-stage cascade:
      ///   1. `AutoParse<std::any>` fast path — exact type match,
      ///      numeric widening (e.g. stored int → requested double),
      ///      or parse-from-stored-string.  Handles the common case
      ///      where the stored type is T (or a near-neighbour) without
      ///      paying AssignRegistry dispatch cost.
      ///   2. `as<T>()` — AssignRegistry cross-type conversion (e.g.
      ///      SliderHandle → int, ComboHandle → std::string).
      ///   3. Stream-extractable `T` + assign miss → string round-trip
      ///      via `parse<T>(as<std::string>())`.
      template<class T>
      operator T() const {
        try {
          return utils::AutoParse<std::any>(*m_entry).template as<T>();
        } catch (const utils::ICLException &) {
          // fall through to AssignRegistry path
        }
        if constexpr (utils::is_stream_extractable<T>::value) {
          try {
            return as<T>();
          } catch (const UnassignableTypesException &) {
            return utils::parse<T>(as<std::string>());
          }
        } else {
          return as<T>();
        }
      }

      /// Thread-local string-view conversion — routes through
      /// `as<string>()` so callers can pass `gui["k"]` to APIs that
      /// take a `std::string_view` without round-tripping through a
      /// temporary.  The returned view is valid until the next
      /// conversion on the same thread.
      operator std::string_view() const {
        static thread_local std::string buf;
        buf = as<std::string>();
        return buf;
      }

      /// RTTI of the currently-stored value.
      const std::type_info &getTypeID() const { return m_entry->type(); }

      // Handle-verb dispatchers.  Each one does a direct
      // type-cascade over the stored `std::any` in
      // `qt/HandleVerbDispatch.cpp` — no smuggling through
      // AssignRegistry.  Each verb is a no-op (with ERROR_LOG) for
      // handle types that don't expose the method.

      /// Trigger a `render` on the stored handle.  Reachable handles:
      /// Image, Draw, Draw3D, Plot, FPS.
      ICLQt_API void render();

      /// Forward a GLCallback pointer to the stored handle's `link`
      /// method.  Reachable: DrawHandle3D only.
      ICLQt_API void link(GLCallback *cb);

      /// Forward a MouseHandler pointer to the stored handle's
      /// `install` path.  Reachable: Image / Draw / Draw3D.
      /// Multiply-inherited MouseHandler subclasses (e.g.
      /// `DefineQuadrangleMouseHandler`) are handled by the
      /// ordinary derived-to-base conversion at the call site.
      ICLQt_API void install(MouseHandler *data);

      /// Convenience — install a plain lambda / std::function.  The
      /// body wraps it in an internal MouseHandler subclass.
      ICLQt_API void install(std::function<void(const MouseEvent &)> f);

      /// Register a simple void-callback.  Reachable on most
      /// value-carrying handles.
      ICLQt_API void registerCallback(const std::function<void()> &cb);

      /// Register a "complex" callback that receives the
      /// originating handle key.
      ICLQt_API void registerCallback(const std::function<void(const std::string &)> &cb);

      /// Enable the underlying widget (handle-specific).
      ICLQt_API void enable();

      /// Disable the underlying widget.
      ICLQt_API void disable();

      /// Drop every callback the handle currently holds.
      ICLQt_API void removeCallbacks();
    };

    /// Store a `T` under `id`, default-constructing if omitted.
    /// Returns a reference into the AnyMap's stable (map-node-based)
    /// storage — callers can hold `&allocValue<T>(...)` for the
    /// entry's lifetime.
    template<class T>
    inline T &allocValue(const std::string &id, const T &val = T()) {
      m_store->template set<T>(id, val);
      return m_store->template get<T>(id);
    }

    /// Access as `T&`.  Throws via AnyMap on key miss or type
    /// mismatch.  The `typeCheck` parameter is accepted for
    /// API compatibility with the old MultiTypeMap and ignored —
    /// AnyMap always type-checks.
    template<class T>
    inline T &getValue(const std::string &id, bool typeCheck = true) {
      (void)typeCheck;
      return m_store->template get<T>(id);
    }

    template<class T>
    inline const T &getValue(const std::string &id, bool typeCheck = true) const {
      (void)typeCheck;
      return m_store->template get<T>(id);
    }

    /// True iff `id` is present (any type).
    bool contains(const std::string &id) const { return m_store->contains(id); }

    /// True iff `id` is present AND stores exactly `T`.
    template<class T>
    bool checkType(const std::string &id) const { return m_store->template containsAs<T>(id); }

    /// Remove every entry.
    void clear() { m_store->clear(); }

    /// Serialize operations across DataStore consumers.  Recursive
    /// so the same thread can re-acquire (e.g. during nested GUI
    /// creation).  Held by shared_ptr so shallow-copies share the
    /// lock.
    void lock() const { m_mutex->lock(); }
    void unlock() const { m_mutex->unlock(); }

    /// Return a proxy-view over the value at `key`.
    /// @throws KeyNotFoundException if `key` is missing.
    Slot operator[](const std::string &key);

    /// Convenience: collect values from many keys into a vector.
    /// Each key is extracted via `operator[]` and implicitly
    /// converted to `T`.
    template<class T>
    std::vector<T> collect(const std::vector<std::string> &keys) {
      std::vector<T> v(keys.size());
      for (unsigned int i = 0; i < keys.size(); ++i) v[i] = operator[](keys[i]);
      return v;
    }

  protected:
    /// Shared typed map.  Held by `shared_ptr` so that copy-assigning
    /// a DataStore (done by `GUI::operator=`) shares one backing map
    /// across all GUI references.
    std::shared_ptr<utils::AnyMap> m_store;

    /// Shared recursive mutex backing `lock()`/`unlock()`.
    mutable std::shared_ptr<std::recursive_mutex> m_mutex;
  };

}  // namespace icl::qt
