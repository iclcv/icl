# Curated YAML/JSON parser corpus for icl::utils::yaml

Two subdirectories, each with its own upstream LICENSE file (both MIT).

## yaml-test-suite/

22 hand-picked cases from github.com/yaml/yaml-test-suite, covering the
ICL YAML subset: block mappings/sequences, flow style, comments, simple
scalars (including UTF-8), and simple block scalars. Each file is a meta-
YAML document with fields `name`, `tags`, `yaml`, `tree`, and `json`.

The `yaml` field holds the input; `json` (when present) is the expected
structural equivalent. A test harness loads each file, extracts `yaml` +
`json`, parses the yaml with our library, and compares the resulting
Node tree against the JSON reference.

Deliberately excluded (not in our subset):
- anchors/aliases (`&x` / `*x`)
- custom tags (`!!foo`)
- directives (`%YAML`, `%TAG`)
- multi-doc streams (`---` / `...` between docs)
- YAML 1.1 bool aliases (yes/no/on/off resolving as Bool)
- explicit-key / complex-key / empty-key constructs
- multi-line plain scalars (our plain scalars are single-line)
- pathological edge-cases tagged `edge` or `1.3-err`

Some picks here may still fail on subtle spec rules (e.g. block scalar
chomping indicators, tab tolerance inside content) — that's the point:
they'll tell us where our subset diverges from strict YAML 1.2.

## json-suite/

All 95 `y_*.json` files from github.com/nst/JSONTestSuite — valid JSON
that every conformant JSON parser must accept. Every valid JSON is valid
YAML 1.2 flow-style, so these exercise our flow parser, UTF-8 handling,
number formatting, nesting, and escape decoding across real-world inputs.

Known divergences where our parser will reject valid JSON:
- `y_object_duplicated_key.json` — we reject duplicate mapping keys per
  YAML 1.2 (JSON allows them).
- Any test relying on deeply nested structure (hundreds of levels) may
  hit stack depth before the test resolves.

Counting these as expected failures, the remaining ~93 tests should all
round-trip cleanly.
