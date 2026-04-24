// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/AutoParse.h>
#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>
#include <icl/utils/StringUtils.h>

#include <cstddef>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
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
      // Self-owned content for Node::operator=(T) and setOwnedScalar().
      // When non-empty, scalarView() returns a view into this; `sv` is
      // unused.  Copying / moving this ScalarData moves the owned bytes
      // with it, so the derived view is always valid for the current
      // Node's lifetime.
      std::string owned;
    };
    using Sequence = std::vector<Node>;

    /// Ordered key/value map with a side-arena for keys whose storage
    /// lifetime must be managed by the Node itself (programmatic inserts,
    /// container assignments).  Parser-emitted entries keep their keys
    /// as views into the Document source — zero copies.
    struct Mapping {
      // `first` may view either into the Document's source buffer OR
      // into one of `ownedKeys`'s stable-address strings, depending on
      // how the entry was inserted.
      std::vector<std::pair<std::string_view, Node>> entries;
      std::deque<std::string>                         ownedKeys;

      Mapping() = default;
      Mapping(Mapping &&) noexcept = default;
      Mapping& operator=(Mapping &&) noexcept = default;
      Mapping(const Mapping &other);           // rebinds owned-key views
      Mapping& operator=(const Mapping &other);

      // Parser / trusted-lifetime path — caller guarantees `key` outlives
      // this Mapping (typically a view into the Document source).
      void emplace_back(std::string_view key, Node value){
        entries.emplace_back(key, std::move(value));
      }
      // Programmatic path — key bytes are copied into `ownedKeys` and the
      // entry's view points at the stable-address copy.
      void emplaceOwned(std::string key, Node value){
        ownedKeys.emplace_back(std::move(key));
        entries.emplace_back(std::string_view(ownedKeys.back()), std::move(value));
      }

      // Iteration + size + indexing, forwarded to `entries`.
      auto        begin()        noexcept { return entries.begin(); }
      auto        end()          noexcept { return entries.end();   }
      auto        begin()  const noexcept { return entries.begin(); }
      auto        end()    const noexcept { return entries.end();   }
      std::size_t size()   const noexcept { return entries.size(); }
      bool        empty()  const noexcept { return entries.empty(); }
      auto&       operator[](std::size_t i)       noexcept { return entries[i]; }
      const auto& operator[](std::size_t i) const noexcept { return entries[i]; }
    };

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

    // operator[] dispatches: integral → sequence index, string_view →
    // mapping key.  The integral overload is templated over any integral
    // type (excluding bool and char-likes) so plain `n[2]` works without
    // an explicit size_t cast, and literal `n[0]` disambiguates cleanly
    // from the const-char*-nullptr path.
    template<class I, class = std::enable_if_t<
               std::is_integral_v<I> &&
               !std::is_same_v<I, bool> &&
               !std::is_same_v<I, char> &&
               !std::is_same_v<I, signed char> &&
               !std::is_same_v<I, unsigned char>>>
    const Node& operator[](I i) const { return at(static_cast<std::size_t>(i)); }
    template<class I, class = std::enable_if_t<
               std::is_integral_v<I> &&
               !std::is_same_v<I, bool> &&
               !std::is_same_v<I, char> &&
               !std::is_same_v<I, signed char> &&
               !std::is_same_v<I, unsigned char>>>
    Node&       operator[](I i)       { return at(static_cast<std::size_t>(i)); }
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
    // View-based — caller must keep `sv` alive.  Used by the parser.
    void setScalar(std::string_view sv, ScalarStyle style = ScalarStyle::Plain);
    // Owning — Node stores `s` internally; always safe.
    void setOwnedScalar(std::string s, ScalarStyle style = ScalarStyle::Plain);
    void setMapping()  { m_value = Mapping{};  }
    void setSequence() { m_value = Sequence{}; }

    // --- assignment from scalar-ish types (owning) ---
    //
    // All `operator=` overloads below take ownership of the resulting
    // string, so the assigned value is always safe regardless of the
    // lifetime of the argument.  For explicit view-based assignment,
    // use `setScalar(sv, style)`.
    //
    // Auto-generated `operator=(const Node&)` / `operator=(Node&&)`
    // still exist for node-to-node assignment.
    Node& operator=(const char *s);
    Node& operator=(const std::string &s);
    Node& operator=(std::string &&s);
    Node& operator=(std::string_view sv);   // copies into owned — safe
    Node& operator=(bool b);

    // --- initializer-list assignment (Level 1) ---
    //
    // Sequence: `node = {1, 2, "three"}` — each element must be
    // constructible-as-Node (uses the converting ctor below).
    Node& operator=(std::initializer_list<Node> seq);
    // Mapping: `node = {{"k", 1}, {"j", "hi"}}` — keys are interned via
    // Mapping::emplaceOwned so lifetime is self-contained.
    Node& operator=(std::initializer_list<std::pair<std::string_view, Node>> map);

    // --- container assignment (Level 2) ---
    template<class T> Node& operator=(const std::vector<T> &v);
    template<class K, class V> Node& operator=(const std::map<K, V> &m);
    template<class K, class V> Node& operator=(const std::unordered_map<K, V> &m);

    // Generic catch-all for any T with a `utils::str(T)` specialization
    // (arithmetic types, ICL geometry types, ...).  SFINAE-excluded for
    // types handled by the explicit overloads and for Node itself, so
    // `node = std::vector<int>{1,2,3}` picks the sequence overload
    // rather than stringifying via CSV.
    template<class T>
    struct _is_generic_scalar_assignable : std::bool_constant<
        !std::is_same_v<std::decay_t<T>, Node> &&
        !std::is_same_v<std::decay_t<T>, std::string> &&
        !std::is_same_v<std::decay_t<T>, std::string_view> &&
        !std::is_same_v<std::decay_t<T>, bool> &&
        !std::is_convertible_v<T, const char *>
    > {};
    // std::vector<T> / std::map<K,V> / std::unordered_map<K,V> are
    // covered by explicit overloads above — opt out here.
    template<class T>              struct _is_generic_scalar_assignable<std::vector<T>>              : std::false_type {};
    template<class K, class V>     struct _is_generic_scalar_assignable<std::map<K, V>>             : std::false_type {};
    template<class K, class V>     struct _is_generic_scalar_assignable<std::unordered_map<K, V>>   : std::false_type {};

    template<class T,
             class = std::enable_if_t<_is_generic_scalar_assignable<std::decay_t<T>>::value>>
    Node& operator=(const T &t);

    // --- converting ctors ---
    //
    // Any T that operator= accepts also constructs a Node:
    //     Node n = 42;
    //     Node s = "hello";
    //     Node m = Size(640, 480);
    // SFINAE-excluded for Node / ScalarData so the compiler-generated
    // copy/move ctors stay in play.
    template<class T,
             class = std::enable_if_t<
               !std::is_same_v<std::decay_t<T>, Node> &&
               !std::is_same_v<std::decay_t<T>, ScalarData>
             >>
    Node(T &&t) : Node() { *this = std::forward<T>(t); }

    // Mapping from ctor-level init-list:
    //     Node n = {{"k", 1}, {"j", 2}};
    // The sequence form is deliberately NOT provided as a ctor — adding
    // `Node(std::initializer_list<Node>)` would make `Node n = {{"k",1}}`
    // ambiguous between "1-element sequence containing a 2-element
    // sub-sequence" and "1-element mapping".  For sequence init use the
    // two-line form:
    //     Node n;  n = {1, 2, 3};
    // or explicit containers:
    //     Node n = std::vector<int>{1, 2, 3};
    Node(std::initializer_list<std::pair<std::string_view, Node>> map) : Node() { *this = map; }

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

  template<class T, class E>
  Node& Node::operator=(const T &t){
    setOwnedScalar(::icl::utils::str(t));
    return *this;
  }

  template<class T>
  Node& Node::operator=(const std::vector<T> &v){
    setSequence();
    auto &seq = sequence();
    seq.reserve(v.size());
    for(const auto &e : v) seq.emplace_back(Node(e));
    return *this;
  }

  template<class K, class V>
  Node& Node::operator=(const std::map<K, V> &m){
    setMapping();
    auto &map = mapping();
    for(const auto &kv : m){
      map.emplaceOwned(::icl::utils::str(kv.first), Node(kv.second));
    }
    return *this;
  }

  template<class K, class V>
  Node& Node::operator=(const std::unordered_map<K, V> &m){
    setMapping();
    auto &map = mapping();
    for(const auto &kv : m){
      map.emplaceOwned(::icl::utils::str(kv.first), Node(kv.second));
    }
    return *this;
  }

}  // namespace icl::utils::yaml
