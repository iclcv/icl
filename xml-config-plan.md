# In-House XML Library Plan

Replace the 15,218-LOC vendored `icl/utils/detail/pugi/` with an
ICL-authored `icl::utils::xml` lib modeled on the recently-landed
`icl::utils::yaml`.  Both pugi consumers in ICL are read-only:
`icl/geom/Primitive3DFilter.cpp` (ICL-authored pointcloud-filter XML)
and `icl/io/detail/grabbers/OptrisGrabber.cpp` (external Optris
calibration XML).  Goal: pugi gone, net deletion in the thousands of
lines, API close enough to pugi that migration is nearly mechanical.

## Scope

**In:** well-formed XML 1.0 — elements, attributes, text, CDATA,
built-in char refs (`&amp; &lt; &gt; &quot; &apos;`) + numeric
(`&#NNN;` `&#xNNN;`), comments (skipped), PIs / DOCTYPE (skipped
leniently), UTF-8 encoding.

**Out (deliberately):** namespaces, DTD validation, custom entity
definitions, XSD, XPath engine (see Phase 7), mixed-document streams.

## Shape (mirrors `icl/utils/Yaml.h`)

```
icl/utils/Xml.h              — public façade (Document, Element, Attribute, ParseError)
icl/utils/Xml.cpp            — Document glue + thin forwarders
icl/utils/detail/xml/
  XmlParser.{h,cpp}          — tokenizer + tree builder, zero-copy
  XmlEmitter.{h,cpp}         — deterministic serializer with escape handling
  XmlArena.{h,cpp}           — node pool (stable-address element storage)
```

Namespace: `icl::utils::xml`.  Same `detail::parseInto(src, doc)`
seam as the Yaml lib.

## DOM model

- `Document` owns the source buffer (for `parseFile`) + a bump arena
  of `Element` nodes.
- `Element` is a handle to an arena-resident node.  Nodes linked as
  `parent / firstChild / nextSibling` — no per-element vector.
  Names are `std::string_view` into source (parse path) or arena
  (mutation path).
- `Attribute` is a `{name_sv, value_sv}` pair stored inline on the
  element as a small-vector.  Raw `valueRaw()` + decoded `value()` /
  `asInt()` / `asFloat()` / `asBool()`.
- `Element::text()` returns the first text-child's raw view;
  `textDecoded()` resolves entity refs.

Zero-copy invariants mirror Yaml: parser never allocates per-token;
only arena allocations for the node tree itself.  Mutation path
copies into arena.

## Public API sketch

```cpp
namespace icl::utils::xml {
  class ParseError : public ICLException { ... };

  class Document {
    static Document parse(std::string_view src);
    static Document parseFile(const std::string &path);
    Element root() const;                  // empty Element if parse failed
    bool    hasRoot() const;
    std::string emit() const;              // round-trips parsed + mutated trees
  };

  class Element {
    std::string_view name() const;
    bool             empty() const;
    explicit         operator bool() const { return !empty(); }

    Element          firstChild() const;
    Element          nextSibling() const;
    Element          child(std::string_view name) const;
    Element          nextSibling(std::string_view name) const;
    auto             children() const;                      // range
    auto             children(std::string_view name) const; // filtered range

    Attribute        attribute(std::string_view name) const;
    auto             attributes() const;                    // range

    std::string_view text() const;         // raw
    std::string      textDecoded() const;  // entities resolved

    // mutation (for future image-editor pipeline save, etc.)
    Element          appendChild(std::string_view name);
    void             setAttribute(std::string_view, std::string_view);
    void             setText(std::string_view);
  };

  class Attribute {
    std::string_view name() const;
    std::string_view valueRaw() const;
    std::string      value() const;        // decoded
    int              asInt(int fb=0) const;
    float            asFloat(float fb=0.0f) const;
    double           asDouble(double fb=0.0) const;
    bool             asBool(bool fb=false) const;
    bool             empty() const;
    explicit operator bool() const { return !empty(); }
  };
}
```

Close enough to pugi that Primitive3DFilter migration is mostly
`s/pugi::/xml::/` + XPath rewrite.

## Phasing

**Scheduling:** user directive — implement the full XML suite
(Phases 1, 2, 7) *before* starting consumer integration (Phases 3–4).
Pugi stays in tree for Phase 5 benchmarking; Phase 6 deletes it last.

### Phase 1 — parser + DOM + read-only API

Hand-rolled tokenizer: character scan, element/attr/text/comment/CDATA
states.  Explicit `ParseError(line, col, msg)` like `yaml::ParseError`.
Entity decoder (5 built-ins + numeric).  ~600 LOC + ~150 LOC tests.

Corpus tests: well-formed fixtures, malformed diagnostics, CDATA,
nested elements, self-closing, attribute quoting (single + double),
whitespace trimming in element content vs. CDATA preservation.

### Phase 2 — emitter

Deterministic output.  Escape attribute values (`" & < >`), escape
text (`& < >`), preserve CDATA blocks.  Round-trip tests via
parse→emit→parse fixpoint.  Indentation knob (like YamlEmitter's).
~200 LOC + tests.

### Phase 3 — Primitive3DFilter migration (interim, no XPath)

Rewrite the single XPath as an explicit children walk:

```cpp
static constexpr std::array actionKinds = {
  "remove","setpos","color","label","intensity","filterdepthimg"
};
for (auto action : rootNode.children()) {
  auto n = action.name();
  if (std::find(actionKinds.begin(), actionKinds.end(), n) == actionKinds.end())
    continue;
  // ... existing per-action-name dispatch
}
```
Everything else is `s/pugi::/xml::/`.  ~30-line diff.

### Phase 4 — OptrisGrabber migration (interim, no XPath)

```cpp
auto fovEl = doc.root()
  .child("Temperature").child("Optics").child("OpticsDef").child("FOV");
if (fovEl) fov = parse<int>(fovEl.text());
```
~5-line diff.

### Phase 5 — benchmarks (pugi still in tree)

Same shape as the Yaml benchmark: parse of small (~500 B —
Primitive3DFilter config) + scaled (~50 KB generated) against pugi.
Pugi stays vendored for this phase so both parsers run in the same
binary.  Pin the numbers in the `Xml.h` header doc comment, like
`Yaml.h` did.

### Phase 6 — delete pugi

Once benchmark numbers are pinned: `rm -r icl/utils/detail/pugi/`,
drop pugi includes, update `meson.build`.  **−15,218 LOC** gone.

### Phase 7 — XPath subset (second run)

Layered additively on top of the DOM so first run is already a
complete replacement; XPath is pure ergonomics.

**Supported subset** (covers both consumers + headroom):

| Feature | Example |
|---|---|
| Absolute path | `/CaliData/Temperature/Optics/OpticsDef/FOV` |
| Relative path | `primitivegroup/group` |
| Wildcard step | `/pointcloudfilter/*` |
| Descendant | `//FOV` |
| Self predicate | `[self::remove or self::setpos or …]` |
| Attr test predicate | `[@id='foo']` |
| Attr existence | `[@id]` |
| Index predicate | `[1]` `[2]` (1-based) |
| Attribute-axis terminal step | `@name` |
| `or` / `and` inside predicates | Primitive3DFilter's union query |

**Out:** namespaces, reverse axes (`parent::`, `ancestor::`, …),
arithmetic, variable refs, union operator `|` between paths,
most built-in functions (optional whitelist: `last()`, `position()`,
`count()`).

**Shape:**
```
icl/utils/detail/xml/
  XPathLexer.{h,cpp}        — token stream (path steps, predicates, operators)
  XPathParser.{h,cpp}       — AST: Path { Step{axis, nameTest, predicates[]} }
  XPathEvaluator.{h,cpp}    — walks AST over an Element, returns NodeSet
```

**Public surface:**
```cpp
class Element {
  ...
  NodeSet   selectAll (std::string_view xpath) const;
  Element   selectOne (std::string_view xpath) const;   // first match or empty
  Attribute selectAttr(std::string_view xpath) const;   // @-terminal paths
};

class NodeSet {
  std::size_t size() const;
  Element     operator[](std::size_t) const;
  auto        begin() const; auto end() const;          // range-for
};
```

AST:
```cpp
struct NameTest { enum { Name, Star } kind; std::string_view name; };
struct Predicate {
  // variant: SelfIs{name}, AttrEq{name, value}, AttrExists{name},
  //          Index{n}, BoolOr{lhs,rhs}, BoolAnd{lhs,rhs}
};
struct Step {
  enum class Axis { Child, Descendant, Attribute, Self } axis;
  NameTest test;
  std::vector<Predicate> predicates;
};
struct Path { bool absolute; std::vector<Step> steps; };
```

Eval is a straight recursion: take the current node-set, for each
node expand along `axis`, apply name test, filter by predicates,
feed to next step.  Small and mechanical.  ~900 LOC total (lexer
~150, parser ~300, evaluator ~200, tests ~250).

**After Phase 7** both consumer call sites collapse back to
one-liners matching the original pugi shape:

```cpp
// Primitive3DFilter
auto actionNodes = doc.root().selectAll(
  "/pointcloudfilter/*[self::remove or self::setpos or self::color "
  "or self::label or self::intensity or self::filterdepthimg]");
for (auto actionNode : actionNodes) { ... }

// OptrisGrabber
auto fovEl = doc.root()
  .selectOne("/CaliData/Temperature/Optics/OpticsDef/FOV");
if (fovEl) fov = parse<int>(fovEl.text());
```

**XPath risks**

- Predicate boolean precedence (`or` < `and`) — grammar test covers
  `a or b and c` explicitly.
- String-equality semantics: pugi compares raw (undecoded) `@x='foo'`.
  We decode before compare — cleaner and matches caller intuition;
  documented difference.
- Wildcard step on large trees fine for Primitive3DFilter-scale; no
  short-circuit optimization in v1.

## Tests

Fixture files under `tests/fixtures/xml/`:
- `well_formed/*.xml` — minimal, nested, attribute edge cases, CDATA,
  comments, PI/DOCTYPE prologue, UTF-8 content.
- `malformed/*.xml` — unclosed tags, mismatched close, bad entity ref,
  missing quote — each with expected line/col in the error.
- `round_trip/*.xml` — parse → emit → parse byte-equal on canonical
  subset.
- Integration: a real `pointcloudfilter.xml` + Primitive3DFilter parse
  test; a stub Optris CaliData file + the FOV extract.
- Phase 7 adds XPath-specific tests: axis coverage, name test literal
  + wildcard, predicate combinations, absolute vs relative, parser
  error messages.

## Risk + open questions

1. **Entity references in attribute values** — Primitive3DFilter
   configs never use them in practice, but XML spec requires
   normalization.  Implement the standard rules; not optional.
2. **Whitespace handling** — `xml:space="preserve"` is a real thing
   but unused by our consumers; skip in v1, document.
3. **Encoding** — UTF-8 only.  Reject non-UTF-8 at parse time with a
   clear error.
4. **Optris file fragility** — vendor-controlled; if Optris ever adds
   CDATA or namespaces we lose.  Mitigation: catch `ParseError` at the
   call site and warn rather than abort — camera operates without the
   XML-derived FOV.
5. **Emitter in v1?** — kept in because the image-editor pipeline-save
   TODO wants either XML or YAML serialization; cost is ~200 LOC.

## Rough size estimate

- Phase 1–2: ~800 LOC + ~250 LOC tests
- Phase 3: ~30-line diff in Primitive3DFilter
- Phase 4: ~5-line diff in OptrisGrabber
- Phase 5: −15,218 LOC (pugi) + −3 LOC (includes)
- Phase 6: benchmark harness ~100 LOC
- Phase 7: ~900 LOC (parser+evaluator+tests)

**Net after Phase 5**: net deletion in the thousands of lines even
with the full in-house parser + emitter in the tree.

**Net after Phase 7**: still a strong deletion — XPath adds ~900 LOC,
pugi's XPath engine alone was ~5000 LOC of the 15k we removed.

## How to maintain this plan

- Check phase boxes below as they land, with commit hashes.
- Sections that complete move into `CONTINUE.md` as Session summaries.
- Open questions resolve into explicit design notes inline, not
  sidebars.

### Phase status

- [x] Phase 1 — parser + DOM + read-only API (landed; 29 tests green)
- [x] Phase 2 — emitter (landed; round-trip fixpoint test green)
- [x] Phase 7 — XPath subset (landed early per user directive; 13 tests green)
- [x] Phase 3 — Primitive3DFilter migration (icl/geom/Primitive3DFilter.cpp now on icl::utils::xml + selectAll for the action-dispatch query; 866/866 tests green)
- [x] Phase 4 — OptrisGrabber migration (one-liner `selectOne("/CaliData/Temperature/Optics/OpticsDef/FOV")`)
- [x] Phase 5 — benchmarks pinned in `Xml.h` (~3.5× behind pugi on raw parse, tied on XPath/traverse; microseconds for config-sized inputs)
- [x] Phase 6 — `icl/utils/detail/pugi/` deleted (~15,218 LOC removed); all 866 tests still green

### Future — optional perf pass

- [ ] **SIMD-accelerate the "find next delimiter" scans.** The ~4×
  parse-throughput gap vs pugi is almost certainly dominated by
  character-class scans (`while(peek() != '<') advance()` for
  content; similar for attribute-value bodies and comment/CDATA
  terminators) walked one byte at a time.  Pugi uses 16-byte SSE
  `_mm_cmpestri`-style bulk scans via its `scan_for_set` predicates.
  A targeted pass would swap the hot inner loops for SIMD bulk
  scans (SSE + NEON via sse2neon, the same pattern `icl/utils/simd`
  already hosts).  Config-sized inputs (our actual workload) are
  already microseconds, so this is pure optimisation, not a
  correctness item.  Defer until we hit a real ceiling.
