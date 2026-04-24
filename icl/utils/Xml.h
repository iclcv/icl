// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

/// \file
/// Slim XML 1.0-subset parser, emitter, and tree representation.
///
/// Parse is zero-copy for element names, attribute names, and raw
/// attribute / text content — the corresponding views point directly
/// into the source buffer.  Programmatic construction owns its own
/// bytes via Document's `m_stringArena` (stable-address string pool).
/// Entity-decoded strings are materialised lazily into the same
/// arena so repeat `textDecoded()` / `value()` calls on the same
/// attribute / element return stable views.
///
/// Performance (Apple-Silicon arm64, release -O3, small ~500 B
/// Primitive3DFilter-shaped config + scaled 50 KB config, median
/// of 50 runs — see `benchmarks/bench-xml.cpp`):
///
///   Benchmark                  icl::utils::xml    pugixml
///   ----------------------------------------------------------------------
///   parse   ~500 B                 1.0 us            0.5 us
///   parse   ~50 KB               108.0 us           49.5 us
///   traverse (parse + walk)        1.0 us            1.0 us
///   xpath (predicate union)        1.0 us            1.0 us
///   emit (round-trip)              1.0 us              —
///
/// Notes on the perf path:
///  * `parseElement`'s content-text scan uses a 16-byte SIMD
///    find-first-'<' (sse2neon on arm64).  This was the only
///    inner-loop spot where SIMD was a measured net win — SIMD-
///    ified skipWs and attr-value scans both regressed because
///    typical runs are <16 bytes.
///  * Node storage is a page-backed bump allocator (`NodeArena`,
///    64 KB pages) — replacing the initial `std::deque<ElementNode>`
///    cut parse_large from 167 us to 108 us.  Attributes hang off
///    each element as a singly-linked list (head + tail pointers),
///    not a `std::vector`, so no per-element vector growth / relocation.
///
/// Remaining ~2x gap vs pugi on raw parse is down to pugi's
/// decades-tuned per-byte scanners + `const char*` walks (no
/// string_view wrappers in the hot path).  Traversal and XPath
/// are tied.  Absolute numbers are microseconds for config-sized
/// inputs — well below any realistic config-load budget.
///
/// Accepted subset:
///   * Elements (start, end, self-closing) with attributes.
///   * Text content + CDATA sections.
///   * Built-in char refs: &amp; &lt; &gt; &quot; &apos;.
///   * Numeric char refs: &#NNN; &#xNNN;.
///   * Comments — skipped.
///   * PIs and DOCTYPE — skipped leniently in the prolog.
///
/// Deliberately not supported:
///   * Namespaces (`ns:name` parses as an opaque element name).
///   * DTD validation, external entities, custom entity definitions.
///   * XML Schema, non-UTF-8 encodings.
///   * Mixed-content ordering — if an element has both text and
///     child elements, all text is concatenated into a single
///     `text()` view and children are preserved in document order.
///     The interleaving between text and children is NOT round-
///     tripped.  Neither of ICL's current XML consumers has mixed
///     content; if a future caller needs it, switch the child-list
///     representation from `(Element* first, Element* next)` to a
///     tagged `std::vector<Child>`.
///
/// See `xml-config-plan.md` for the project-level plan and
/// `project_xml_config.md` memory for status.

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace icl::utils::xml {

  class Document;
  class Element;
  class Attribute;
  class NodeSet;

  namespace detail {

    /// One attribute on an element.  `name` and `valueRaw` are views
    /// into the Document's source buffer (parse path) or string
    /// arena (mutation path) — the AttributeNode owns no bytes.
    /// Attributes hang off their owning element as a singly-linked
    /// list (head + tail on the element), so appending is O(1) and
    /// no per-element std::vector is paid for.  Both this struct and
    /// `ElementNode` are trivially destructible — the NodeArena
    /// frees their backing pages without running dtors.
    struct AttributeNode {
      std::string_view name;
      std::string_view valueRaw;
      /// Lazy entity-decoded cache — filled on first `value()` call.
      mutable std::string_view valueDecodedCache;
      mutable bool             valueDecodedResolved = false;
      AttributeNode           *next = nullptr;
    };

    /// One element in the tree.  Allocated from the Document's
    /// `NodeArena` (page-backed bump allocator → stable addresses)
    /// so the linked-list pointers stay valid for the Document's
    /// lifetime.
    struct ElementNode {
      std::string_view           name;

      /// Direct text content; see the `mixed content` note in the
      /// file-level comment for edge-case semantics.
      std::string_view           text;
      mutable std::string_view   textDecodedCache;
      mutable bool               textDecodedResolved = false;

      /// Singly-linked attribute list.  `lastAttribute` enables O(1)
      /// append during parse / setAttribute; iteration is via
      /// `firstAttribute`'s next-chain.
      AttributeNode             *firstAttribute = nullptr;
      AttributeNode             *lastAttribute  = nullptr;

      /// Element-only linked-list of children.
      ElementNode               *parent      = nullptr;
      ElementNode               *firstChild  = nullptr;
      ElementNode               *lastChild   = nullptr;
      ElementNode               *nextSibling = nullptr;
    };

    /// Page-based bump allocator for ElementNode + AttributeNode.
    /// Pages are never reclaimed until the Document is destroyed —
    /// `free()` is a no-op, only page-wholesale release on dtor.
    /// Both node types are trivially destructible, so no dtor
    /// dispatch is needed on release.  Size tuned so a ~50 KB
    /// XML config (the scaled benchmark) fits in a handful of pages.
    class ICLUtils_API NodeArena {
    public:
      NodeArena();
      ~NodeArena();
      NodeArena(NodeArena&&) noexcept;
      NodeArena& operator=(NodeArena&&) noexcept;
      NodeArena(const NodeArena&) = delete;
      NodeArena& operator=(const NodeArena&) = delete;

      ElementNode  *allocElement();
      AttributeNode *allocAttribute();

    private:
      static constexpr std::size_t kPageBytes = 64 * 1024;
      struct Page {
        std::unique_ptr<char[]> buf;
        std::size_t             used = 0;
      };
      std::vector<Page>          m_pages;
      char                      *m_cur = nullptr;   // bump pointer into current page
      char                      *m_end = nullptr;   // one-past-end of current page

      void *alloc(std::size_t bytes, std::size_t align);
      void  newPage(std::size_t atLeast);
    };

    ICLUtils_API void parseInto(std::string_view src, Document &doc);
  }

  /// Thrown by parse paths.  Carries a source location so callers can
  /// produce a useful diagnostic.  Format mirrors `yaml::ParseError`.
  class ICLUtils_API ParseError : public ICLException {
    std::size_t m_line;
    std::size_t m_col;
  public:
    ParseError(std::size_t line, std::size_t col, const std::string &what);
    std::size_t line() const noexcept { return m_line; }
    std::size_t col()  const noexcept { return m_col;  }
  };

  /// Emitter tuning knobs.
  struct EmitOptions {
    int  indent       = 2;      ///< spaces per nesting level
    bool prologue     = true;   ///< emit `<?xml version="1.0" ...?>` header
    bool selfCloseEmpty = true; ///< `<x/>` vs `<x></x>` for empty-content elements
  };

  /// Handle to one attribute on an Element.  Pointer-sized; empty when
  /// the attribute is absent.  Views into source or the Document's
  /// string arena — never copies raw bytes on construction.
  class ICLUtils_API Attribute {
  public:
    Attribute() = default;
    explicit Attribute(const detail::AttributeNode *n,
                       const Document *doc) noexcept
      : m_node(n), m_doc(doc) {}

    bool empty() const noexcept { return m_node == nullptr; }
    explicit operator bool() const noexcept { return !empty(); }

    /// Attribute name, zero-copy view.
    std::string_view name() const noexcept;
    /// Raw, undecoded value as it appeared in the source.  `&amp;`
    /// stays as the literal five bytes.
    std::string_view valueRaw() const noexcept;
    /// Entity-decoded value.  First call may intern into the
    /// Document's arena; subsequent calls return the same view.
    std::string_view value() const;

    /// Numeric-conversion helpers (parse the decoded value).  `fb` is
    /// returned on empty attribute or parse failure; no exception.
    int      asInt(int fb = 0) const noexcept;
    float    asFloat(float fb = 0.0f) const noexcept;
    double   asDouble(double fb = 0.0) const noexcept;
    bool     asBool(bool fb = false) const noexcept;

  private:
    const detail::AttributeNode *m_node = nullptr;
    const Document              *m_doc  = nullptr;
    friend class Element;
  };

  /// Iterable linked-list range of child elements — returned by
  /// `Element::children()` and `Element::children(name)`.  Attribute
  /// iteration is deliberately omitted in v1: every real use site
  /// looks attributes up by name (`Element::attribute`), and the
  /// vector-backed storage would force a separate class shape.
  class ICLUtils_API ElementRange {
  public:
    class Iterator {
    public:
      Iterator() = default;
      Iterator(detail::ElementNode *n, const Document *doc, std::string_view filter) noexcept
        : m_n(n), m_doc(doc), m_filter(filter) {}

      Element operator*() const noexcept;
      Iterator& operator++() noexcept;
      bool operator==(const Iterator &o) const noexcept { return m_n == o.m_n; }
      bool operator!=(const Iterator &o) const noexcept { return m_n != o.m_n; }
    private:
      detail::ElementNode *m_n      = nullptr;
      const Document      *m_doc    = nullptr;
      std::string_view     m_filter;
    };

    ElementRange(detail::ElementNode *first, const Document *doc,
                 std::string_view filter = {}) noexcept
      : m_first(first), m_doc(doc), m_filter(filter) {}

    Iterator begin() const noexcept;
    Iterator end()   const noexcept { return Iterator(nullptr, m_doc, m_filter); }
    bool empty() const noexcept { return begin() == end(); }

  private:
    detail::ElementNode *m_first = nullptr;
    const Document      *m_doc   = nullptr;
    std::string_view     m_filter;
  };

  /// Handle to one element in a Document's tree.  Pointer-sized;
  /// empty when the element is absent (e.g. `parent.child("nope")`).
  class ICLUtils_API Element {
  public:
    Element() = default;
    explicit Element(detail::ElementNode *n, const Document *doc) noexcept
      : m_node(n), m_doc(doc) {}

    bool empty() const noexcept { return m_node == nullptr; }
    explicit operator bool() const noexcept { return !empty(); }

    /// Element name (the `foo` in `<foo>`), zero-copy view.
    std::string_view name() const noexcept;

    // ---------------- navigation ----------------
    /// First child element (element-only; text nodes are never
    /// returned from navigation — use `text()` / `textDecoded()` for
    /// text content).  Empty if no child element exists.
    Element firstChild() const noexcept;
    /// Next sibling element.  Empty at end-of-list.
    Element nextSibling() const noexcept;
    /// First child element whose name equals `n`.  Empty if none.
    Element child(std::string_view n) const noexcept;
    /// Next sibling element whose name equals `n`.  Empty if none.
    Element nextSibling(std::string_view n) const noexcept;

    /// Iterate all child elements.
    ElementRange children() const noexcept;
    /// Iterate child elements filtered by name.
    ElementRange children(std::string_view n) const noexcept;

    // ---------------- attributes ----------------
    /// Attribute lookup by name.  Empty if absent.
    Attribute attribute(std::string_view n) const noexcept;
    /// Number of attributes on this element.
    std::size_t attributeCount() const noexcept;
    /// Attribute at index `i` (source order), or empty if out of range.
    Attribute  attributeAt(std::size_t i) const noexcept;

    // ---------------- text content --------------
    /// Raw (undecoded) text content of this element.  For leaf
    /// elements like `<FOV>56</FOV>` this returns the view `"56"`.
    /// For mixed content (element with text AND children) the parser
    /// concatenates all text chunks into a single stable view; the
    /// interleaving with child elements is lost.
    std::string_view text() const noexcept;
    /// Entity-decoded text.  First call may intern into the
    /// Document's arena.
    std::string_view textDecoded() const;

    // ---------------- mutation ------------------
    /// Append a new empty child element.  `n` is copied into the
    /// Document's arena — caller's buffer lifetime doesn't matter.
    Element appendChild(std::string_view n);
    /// Set (or overwrite) an attribute.  Both name and value are
    /// copied into the Document's arena.  Raw value is stored as-is;
    /// the emitter escapes on output.
    void setAttribute(std::string_view n, std::string_view v);
    /// Replace direct text content.  Copied into the arena.  Raw
    /// (undecoded) semantics — emitter will escape on output.
    void setText(std::string_view v);

    // ---------------- XPath (Phase 7) -----------
    /// Evaluate an XPath expression starting at this element.
    /// Throws `ParseError` on malformed expressions.
    NodeSet    selectAll(std::string_view xpath) const;
    /// First match of `selectAll`, empty if no match.
    Element    selectOne(std::string_view xpath) const;
    /// Terminal `@name` path — returns the selected attribute.
    Attribute  selectAttr(std::string_view xpath) const;

    // ---------------- internals -----------------
    detail::ElementNode *rawNode() const noexcept { return m_node; }
    const Document      *document() const noexcept { return m_doc; }

  private:
    detail::ElementNode *m_node = nullptr;
    const Document      *m_doc  = nullptr;
  };

  /// Result of `Element::selectAll(xpath)` — list of matched elements.
  class ICLUtils_API NodeSet {
  public:
    NodeSet() = default;
    explicit NodeSet(std::vector<Element> v) : m_items(std::move(v)) {}

    std::size_t size() const noexcept { return m_items.size(); }
    bool        empty() const noexcept { return m_items.empty(); }
    Element     operator[](std::size_t i) const { return m_items[i]; }

    auto begin() const noexcept { return m_items.begin(); }
    auto end()   const noexcept { return m_items.end();   }

  private:
    std::vector<Element> m_items;
  };

  /// Owns the source bytes + node storage for an XML tree.
  ///
  /// Acquisition modes mirror `yaml::Document`:
  ///   * `parse(sv)` — parses a view; caller must keep the bytes
  ///     alive for the Document's lifetime (non-owning source).
  ///   * `parseOwned(str)` — Document takes ownership of the parse
  ///     buffer (heap-allocated via unique_ptr for address stability
  ///     across Document moves).
  ///   * `parseFile(path)` — reads into an owned buffer.
  ///   * `empty()` — starts with a null root, ready for programmatic
  ///     build via `root().appendChild(...)`.
  ///
  /// Mutations that introduce new byte sequences (entity-decoded
  /// values, programmatic `appendChild` / `setAttribute` / `setText`)
  /// land on `m_stringArena`, a `std::deque<std::string>` chosen for
  /// pointer stability — existing views never invalidate.  Element
  /// nodes land in `m_elementArena`, a `std::deque<ElementNode>` for
  /// the same reason.
  class ICLUtils_API Document {
  public:
    Document();
    ~Document();
    Document(Document&&) noexcept;
    Document& operator=(Document&&) noexcept;
    Document(const Document&) = delete;              // trees would need deep copy
    Document& operator=(const Document&) = delete;

    // Factories
    static Document parse     (std::string_view src);
    static Document parseOwned(std::string src);
    static Document parseFile (const std::string &path);
    static Document empty();

    // Root access
    Element root() noexcept;
    Element root() const noexcept;
    bool    hasRoot() const noexcept;

    /// Set the root element by name.  Valid only on an empty
    /// Document; throws if a root already exists.
    Element setRoot(std::string_view name);

    // Emit as text.
    std::string emit(EmitOptions opts = {}) const;
    void        emitToFile(const std::string &path, EmitOptions opts = {}) const;

    /// Arena hooks — intern a string and return a stable view into
    /// it.  Used by the parser for entity-decoded content and by
    /// mutation paths for caller-owned data.
    std::string_view intern(std::string s);

    /// Allocate a fresh ElementNode in the node arena and return a
    /// pointer to it.  Internal API — public because the parser /
    /// mutators live outside this class.
    detail::ElementNode*  allocElement();

    /// Allocate a fresh AttributeNode.  Used by the parser and by
    /// `Element::setAttribute` to append to the linked attribute list.
    detail::AttributeNode* allocAttribute();

    /// Install `root` as the document root.  Internal API — called
    /// by the parser once it finishes the top-level element.
    void setRootNode(detail::ElementNode *root) noexcept;

    /// Source buffer view (empty for programmatically-built docs).
    std::string_view source() const noexcept { return m_view; }

  private:
    friend void detail::parseInto(std::string_view src, Document &doc);

    std::unique_ptr<std::string>          m_source;       // stable bytes for `parseOwned`
    std::string_view                      m_view;         // view into m_source or caller buf
    std::deque<std::string>               m_stringArena;  // interned strings (mutation / decode)
    detail::NodeArena                     m_nodeArena;    // page-backed node pool
    detail::ElementNode                  *m_root = nullptr;
  };

}  // namespace icl::utils::xml
