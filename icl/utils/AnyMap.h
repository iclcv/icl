// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>

#include <any>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace icl::utils {

  /// Type-erased heterogeneous string-keyed map.
  ///
  /// Thin typed wrapper around `std::unordered_map<std::string, std::any>`.
  /// Callers `set()` values of any copyable type and `get<T>()` them back
  /// with the compile-time-specified type; wrong-type access throws
  /// `std::bad_any_cast`, missing keys throw `std::out_of_range`.
  ///
  /// Intended as the modern replacement for `utils::MultiTypeMap` — same
  /// use-case (heterogeneous keyed storage) but no `void*`, no RTTI-string
  /// comparisons, no `static T _NULL` sentinel, no array/value bit-hack
  /// (store a `std::vector<T>` inside the `std::any` if you need an array).
  class AnyMap {
    std::unordered_map<std::string, std::any> m_data;

  public:
    /// Store a copy of `value` under `key`.  Overwrites any existing entry
    /// (including entries of a different type).
    template<typename T>
    void set(const std::string &key, T value) {
      m_data[key] = std::move(value);
    }

    /// Access as `T&`.
    /// @throws std::out_of_range if `key` is not present.
    /// @throws std::bad_any_cast if the stored type is not `T`.
    template<typename T>
    T &get(const std::string &key) {
      auto it = m_data.find(key);
      if (it == m_data.end()) throw std::out_of_range("AnyMap: no such key: " + key);
      return std::any_cast<T&>(it->second);
    }

    template<typename T>
    const T &get(const std::string &key) const {
      auto it = m_data.find(key);
      if (it == m_data.end()) throw std::out_of_range("AnyMap: no such key: " + key);
      return std::any_cast<const T&>(it->second);
    }

    /// Non-throwing access.  Returns nullptr if `key` is missing OR the
    /// stored type is not `T`.  The returned pointer is valid until the
    /// corresponding entry is erased or overwritten.
    template<typename T>
    T *tryGet(const std::string &key) noexcept {
      auto it = m_data.find(key);
      if (it == m_data.end()) return nullptr;
      return std::any_cast<T>(&it->second);
    }

    template<typename T>
    const T *tryGet(const std::string &key) const noexcept {
      auto it = m_data.find(key);
      if (it == m_data.end()) return nullptr;
      return std::any_cast<const T>(&it->second);
    }

    /// True iff a value is stored under `key` (regardless of its type).
    bool contains(const std::string &key) const noexcept {
      return m_data.find(key) != m_data.end();
    }

    /// True iff a value of exactly type `T` is stored under `key`.
    template<typename T>
    bool containsAs(const std::string &key) const noexcept {
      auto it = m_data.find(key);
      return it != m_data.end() && it->second.type() == typeid(T);
    }

    /// Type-info of the value stored under `key`.
    /// @throws std::out_of_range if `key` is not present.
    const std::type_info &typeOf(const std::string &key) const {
      auto it = m_data.find(key);
      if (it == m_data.end()) throw std::out_of_range("AnyMap: no such key: " + key);
      return it->second.type();
    }

    /// Remove the entry under `key`.  Returns true if one was removed.
    bool erase(const std::string &key) noexcept {
      return m_data.erase(key) > 0;
    }

    /// Remove all entries.
    void clear() noexcept { m_data.clear(); }

    std::size_t size() const noexcept { return m_data.size(); }
    bool empty() const noexcept { return m_data.empty(); }

    /// Iteration exposes raw `(std::string, std::any)` pairs.
    auto begin()       { return m_data.begin(); }
    auto end()         { return m_data.end();   }
    auto begin() const { return m_data.begin(); }
    auto end()   const { return m_data.end();   }
  };

}  // namespace icl::utils
