// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/AutoParse.h>
#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace icl::utils::yaml {

  class Document;
  namespace detail {
    ICLUtils_API void parseInto(std::string_view src, Document &doc);
  }

  /// Tags the *inferred* YAML scalar kind per the 1.2 core schema.
  ///
  /// Computed lazily from (scalar bytes, scalar style): quoted scalars
  /// resolve unconditionally to String; plain scalars resolve via the
  /// pattern table in `Node::scalarKind()`.
  enum class ScalarKind : std::uint8_t { Null, Bool, Int, Float, String };

  /// How a scalar was lexed (or should be emitted).  Round-tripped by
  /// the emitter — a double-quoted scalar read in re-emits double-quoted.
  enum class ScalarStyle : std::uint8_t {
    Plain, SingleQuoted, DoubleQuoted, Literal, Folded
  };

  /// Thrown by parse paths.  Carries a source location so the caller
  /// can produce a useful diagnostic.
  class ICLUtils_API ParseError : public ICLException {
    std::size_t m_line;
    std::size_t m_col;
  public:
    ParseError(std::size_t line, std::size_t col, const std::string &what);
    std::size_t line() const noexcept { return m_line; }
    std::size_t col()  const noexcept { return m_col;  }
  };

  /// Thrown by `Node::asStrict<T>()` on kind mismatch and by `scalarView()`
  /// / indexed accessors when the node is of the wrong shape.
  class ICLUtils_API TypeError : public ICLException {
  public:
    explicit TypeError(const std::string &what);
  };

  // ---------------------------------------------------------------------
  // Compile-time predicate: does the strict-mode matrix accept this kind
  // for the target type T?  See the matrix in the project plan.
  // ---------------------------------------------------------------------
  template<class T>
  constexpr bool yaml_accepts_kind(ScalarKind k) noexcept {
    using U = std::decay_t<T>;
    if constexpr (std::is_same_v<U, bool>) {
      return k == ScalarKind::Bool;
    } else if constexpr (std::is_integral_v<U>) {
      return k == ScalarKind::Int;
    } else if constexpr (std::is_floating_point_v<U>) {
      return k == ScalarKind::Int || k == ScalarKind::Float;
    } else if constexpr (std::is_same_v<U, std::string> ||
                         std::is_same_v<U, std::string_view>) {
      // Any non-Null kind stringifies safely; Null must go through optional.
      return k != ScalarKind::Null;
    } else {
      // Registered custom types (Size, Point, Rect, …): accept only a
      // String-kinded source, which the permissive parser then
      // interprets via AutoParse<string_view> → parse<T>.
      return k == ScalarKind::String;
    }
  }

  class Document;

  /// A YAML node: null, scalar, sequence, or mapping.
  ///
  /// Scalars are stored as `std::string_view` into the owning Document's
  /// source buffer or mutation arena — the Node itself owns no bytes.
  /// Kind resolution (`scalarKind()`) is lazy + cached.
  class ICLUtils_API Node {
  public:
    struct ScalarData {
      std::string_view    sv;
      ScalarStyle         style        = ScalarStyle::Plain;
      mutable ScalarKind  cachedKind   = ScalarKind::String;
      mutable bool        kindResolved = false;
      // Optional explicit tag override from `!!str` / `!!int` / `!!bool`
      // / `!!float` / `!!null`.  When set, scalarKind() returns this
      // directly, bypassing content-based resolution.
      std::optional<ScalarKind> explicitTag;
    };
    using Sequence = std::vector<Node>;
    using Mapping  = std::vector<std::pair<std::string_view, Node>>;

    Node() = default;                            // Null
    explicit Node(ScalarData s) : m_value(std::move(s)) {}

    // --- introspection ---
    bool isNull()     const noexcept { return std::holds_alternative<std::monostate>(m_value); }
    bool isScalar()   const noexcept { return std::holds_alternative<ScalarData>(m_value); }
    bool isSequence() const noexcept { return std::holds_alternative<Sequence>(m_value); }
    bool isMapping()  const noexcept { return std::holds_alternative<Mapping>(m_value); }
    std::size_t size() const noexcept;

    ScalarKind       scalarKind()  const;
    ScalarStyle      scalarStyle() const;
    bool             isQuoted()    const;
    std::string_view scalarView()  const;        // throws TypeError if not scalar

    // --- typed scalar access ---
    template<class T> T                as() const;
    template<class T> T                as(T fallback) const noexcept;
    template<class T> T                asStrict() const;
    template<class T> std::optional<T> tryAs() const noexcept;

    // --- sequence access ---
    const Node& at(std::size_t i) const;         // throws TypeError / out-of-range
    Node&       at(std::size_t i);
    Node&       pushBack(Node n);                // auto-converts Null -> Sequence

    // --- mapping access ---
    bool        contains(std::string_view key) const;
    const Node* find(std::string_view key) const;
    Node*       find(std::string_view key);
    const Node& get(std::string_view key) const; // throws if missing or not mapping
    Node&       get(std::string_view key);
    Node&       getOrInsert(std::string_view key); // auto-converts Null -> Mapping

    // operator[] dispatches: size_t → sequence, string_view → mapping
    const Node& operator[](std::size_t i)        const { return at(i); }
    Node&       operator[](std::size_t i)              { return at(i); }
    const Node& operator[](std::string_view key) const { return get(key); }
    Node&       operator[](std::string_view key)       { return getOrInsert(key); }
    const Node& operator[](const char *key)      const { return get(std::string_view(key)); }
    Node&       operator[](const char *key)            { return getOrInsert(std::string_view(key)); }

    // --- iteration ---
    // Use .mapping() / .sequence() to get the underlying container for
    // range-for access.  Throws TypeError on shape mismatch.
    const Mapping&  mapping()  const;
    Mapping&        mapping();
    const Sequence& sequence() const;
    Sequence&       sequence();

    // --- mutation ---
    void setNull()     noexcept { m_value = std::monostate{}; }
    void setScalar(std::string_view sv, ScalarStyle style = ScalarStyle::Plain);
    void setMapping()  { m_value = Mapping{};  }
    void setSequence() { m_value = Sequence{}; }

    // --- internals ---
    const auto& value() const noexcept { return m_value; }
    auto&       value()       noexcept { return m_value; }

  private:
    std::variant<std::monostate, ScalarData, Sequence, Mapping> m_value;
  };

  /// Emitter tuning knobs.
  struct EmitOptions {
    int  indent        = 2;      // spaces per nesting level
    int  flowThreshold = 6;      // seqs of <= N scalars emit as flow (inline `[…]`)
    bool sortKeys      = false;  // default preserves insertion order
  };

  /// Owns the bytes that Node scalars view into.
  ///
  /// Three acquisition modes:
  ///   - `view(sv)` — non-owning, caller must outlive the Document
  ///   - `own(str)` — Document takes ownership of the parse buffer
  ///   - `file(path)` — reads into an owned buffer
  ///   - `empty()` — starts with a Null root, ready for programmatic build
  ///
  /// Mutations that introduce new byte sequences (escape-decoded scalars,
  /// programmatic `setScalar`) land on `m_arena`, a `std::deque<std::string>`
  /// chosen for pointer stability — existing views never invalidate.
  class ICLUtils_API Document {
  public:
    // Factories — implemented in Yaml.cpp once parser lands (Checkpoint 3).
    static Document view(std::string_view src);
    static Document own(std::string src);
    static Document file(const std::string &path);
    static Document empty();

    // Root access
    Node&       root()       noexcept { return m_root; }
    const Node& root() const noexcept { return m_root; }

    // Emit as text.
    std::string emit(EmitOptions opts = {}) const;
    void        emitToFile(const std::string &path, EmitOptions opts = {}) const;

    // Arena hook — interns a string and returns a stable view into it.
    // Used by the parser for escape-decoded scalars and by mutation
    // paths that take owning data.
    std::string_view intern(std::string s);

    // Source buffer access — exposes the bytes Node scalars view into.
    // Empty when the document was built programmatically.
    std::string_view source() const noexcept { return m_view; }

  private:
    friend void detail::parseInto(std::string_view src, Document &doc);

    // m_source is heap-allocated via unique_ptr so that its bytes have
    // stable addresses across Document moves.  std::string's SSO would
    // otherwise invalidate every `string_view` in m_root when Document
    // is returned by value and the compiler doesn't apply NRVO.
    std::unique_ptr<std::string>  m_source;
    std::string_view              m_view;
    std::deque<std::string>       m_arena;
    Node                          m_root;
  };

  // ---------------------------------------------------------------------
  // Inline template methods
  // ---------------------------------------------------------------------

  template<class T>
  T Node::as() const {
    return AutoParse<std::string_view>(scalarView()).template as<T>();
  }

  template<class T>
  T Node::as(T fallback) const noexcept {
    try {
      if (!isScalar()) return fallback;
      return AutoParse<std::string_view>(scalarView()).template as<T>();
    } catch (...) {
      return fallback;
    }
  }

  template<class T>
  T Node::asStrict() const {
    if (!isScalar()) {
      throw TypeError("asStrict: node is not a scalar");
    }
    const ScalarKind k = scalarKind();
    if (!yaml_accepts_kind<T>(k)) {
      throw TypeError("asStrict: scalar kind is not accepted for requested type");
    }
    return as<T>();
  }

  template<class T>
  std::optional<T> Node::tryAs() const noexcept {
    try { return asStrict<T>(); } catch (...) { return std::nullopt; }
  }

}  // namespace icl::utils::yaml
