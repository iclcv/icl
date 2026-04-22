// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>
#include <icl/utils/StringUtils.h>

#include <any>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace icl::utils {

  /// Short-lived conversion proxy returned from index-style accessors
  /// like `ProgArg::operator[]` or `DataStore::operator[]`.
  ///
  /// Two specializations:
  ///
  ///   - `AutoParse<std::string>` — string-backed.  `operator T()` calls
  ///     `parse<T>(payload)` for any stream-extractable T.  This is what
  ///     `ProgArg::operator[](int)` returns: `int x = pa[0]` parses.
  ///
  ///   - `AutoParse<std::any>` — any-backed.  `operator T()` tries an
  ///     exact `std::any_cast<T>` first, then numeric widening (if both
  ///     stored and target are arithmetic), then — if the stored value
  ///     is itself a `std::string` — falls back to `parse<T>` on that
  ///     string.
  ///
  /// Discipline: AutoParse is a *conversion proxy*, not a storage type.
  /// It is meant to be consumed immediately as `T x = ap` or `ap.as<T>()`.
  /// Do not store, do not put into containers, do not use as a function
  /// parameter type.  Use `std::string` / `std::any` directly for those
  /// roles.
  template<class Backend>
  class AutoParse;


  /// String-backed specialization.  See `AutoParse` for overview.
  ///
  /// Publicly inherits `std::string` so that *all* normal string
  /// operations (concatenation with `+`, comparison, `empty()`, stream
  /// insertion, passing to functions that take `const std::string&`,
  /// etc.) work without per-operator overloads.  On top of that, the
  /// templated `operator T()` parses the string into any stream-
  /// extractable T — enabling `int x = pa[0]` and friends.
  ///
  /// The inheritance is the same trick `utils::Any` used; it is safe
  /// here *because* of AutoParse's discipline rule (never stored,
  /// never a function parameter type, never in a container).  The
  /// ambiguity-with-std::any problem that motivated replacing Any
  /// cannot arise for a proxy that is always consumed in the same
  /// full-expression it is created in.
  template<>
  class AutoParse<std::string> : public std::string {
    // Exclude char-like types and std::string from the template
    // `operator T()` — these are reachable through the `std::string`
    // base (implicit derived-to-base conversion) and must not compete.
    template<class T>
    static constexpr bool is_parse_target =
        !std::is_same_v<T, char> &&
        !std::is_same_v<T, signed char> &&
        !std::is_same_v<T, unsigned char> &&
        !std::is_same_v<T, char*> &&
        !std::is_same_v<T, const char*> &&
        !std::is_same_v<T, std::string> &&
        !std::is_same_v<T, std::string_view>;

  public:
    AutoParse() = default;
    explicit AutoParse(std::string v) : std::string(std::move(v)) {}

    /// Implicit conversion to T via `parse<T>`, for types *other than*
    /// the string-like ones that the std::string base already handles.
    template<class T, class = std::enable_if_t<is_parse_target<T>>>
    operator T() const { return parse<T>(*this); }

    /// Explicit conversion — avoids implicit-operator ambiguity in
    /// contexts like `foo(ap)` where `foo` has multiple overloads.
    template<class T>
    T as() const { return parse<T>(*this); }

    /// Access the underlying string directly.  Equivalent to slicing to
    /// the `std::string` base; exists for symmetry with the `any`
    /// specialization.
    const std::string &str() const { return *this; }
  };


  /// Any-backed specialization.  See `AutoParse` for overview.
  template<>
  class AutoParse<std::any> {
    std::any m_value;

    // Try to extract stored-as-U into an arithmetic T via static_cast.
    template<class T, class U>
    bool try_widen(T &out) const {
      if (auto *p = std::any_cast<U>(&m_value)) {
        out = static_cast<T>(*p);
        return true;
      }
      return false;
    }

    // Numeric widening cascade.  Only instantiated for arithmetic T.
    // Covers the types ICL actually stores in std::any heterogeneously.
    template<class T>
    bool numeric_widen(T &out) const {
      return try_widen<T, bool>(out)
          || try_widen<T, char>(out)
          || try_widen<T, signed char>(out)
          || try_widen<T, unsigned char>(out)
          || try_widen<T, short>(out)
          || try_widen<T, unsigned short>(out)
          || try_widen<T, int>(out)
          || try_widen<T, unsigned int>(out)
          || try_widen<T, long>(out)
          || try_widen<T, unsigned long>(out)
          || try_widen<T, long long>(out)
          || try_widen<T, unsigned long long>(out)
          || try_widen<T, float>(out)
          || try_widen<T, double>(out)
          || try_widen<T, long double>(out);
    }

    // For T = std::string: if stored is a stream-insertable U, format it.
    template<class U>
    bool try_stringify(std::string &out) const {
      if (auto *p = std::any_cast<U>(&m_value)) {
        out = str(*p);
        return true;
      }
      return false;
    }

    bool numeric_stringify(std::string &out) const {
      return try_stringify<bool>(out)
          || try_stringify<char>(out)
          || try_stringify<signed char>(out)
          || try_stringify<unsigned char>(out)
          || try_stringify<short>(out)
          || try_stringify<unsigned short>(out)
          || try_stringify<int>(out)
          || try_stringify<unsigned int>(out)
          || try_stringify<long>(out)
          || try_stringify<unsigned long>(out)
          || try_stringify<long long>(out)
          || try_stringify<unsigned long long>(out)
          || try_stringify<float>(out)
          || try_stringify<double>(out)
          || try_stringify<long double>(out);
    }

  public:
    AutoParse() = default;
    explicit AutoParse(std::any v) : m_value(std::move(v)) {}

    /// Access the underlying std::any directly.
    const std::any &any() const { return m_value; }

    /// RTTI of the currently-stored payload.
    const std::type_info &type() const { return m_value.type(); }

    /// Whether the payload is non-empty.
    bool has_value() const { return m_value.has_value(); }

    /// Implicit conversion cascade:
    ///   1. Exact type match (`std::any_cast<T>`).
    ///   2. Numeric widening (if T is arithmetic and a common numeric
    ///      type is stored) — avoids a string round-trip.
    ///   3. If the payload is a `std::string` and T is stream-extractable,
    ///      `parse<T>(payload)`.
    ///   4. (T == std::string special case) if payload is a stream-
    ///      insertable numeric type, `str(payload)`.
    /// Throws `ICLException` if none apply.
    template<class T>
    operator T() const {
      // 1. Exact type match.
      if (auto *p = std::any_cast<T>(&m_value)) return *p;

      // 4. Requested std::string — stringify common numeric types.
      if constexpr (std::is_same_v<T, std::string>) {
        std::string out;
        if (numeric_stringify(out)) return out;
      }

      // 2. Numeric widening.
      if constexpr (std::is_arithmetic_v<T>) {
        T out{};
        if (numeric_widen<T>(out)) return out;
      }

      // 3. String → parse<T>.
      if constexpr (is_stream_extractable<T>::value) {
        if (auto *s = std::any_cast<std::string>(&m_value)) {
          return parse<T>(*s);
        }
      }

      throw ICLException(std::string("AutoParse<std::any>: cannot convert stored type '")
                         + (m_value.has_value() ? m_value.type().name() : "<empty>")
                         + "' to requested type");
    }

    /// Explicit conversion — avoids implicit-operator ambiguity.
    template<class T>
    T as() const { return static_cast<T>(*this); }
  };

}  // namespace icl::utils
