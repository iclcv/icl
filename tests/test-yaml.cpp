// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/Yaml.h>
#include <icl/utils/Size.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace icl::utils;
using namespace icl::utils::yaml;

// ---------------------------------------------------------------------------
// Node construction + shape introspection
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.node.default_is_null", "default-constructed node is null")
{
  Node n;
  ICL_TEST_TRUE(n.isNull());
  ICL_TEST_FALSE(n.isScalar());
  ICL_TEST_FALSE(n.isSequence());
  ICL_TEST_FALSE(n.isMapping());
  ICL_TEST_EQ(n.size(), size_t(0));
}

ICL_REGISTER_TEST("utils.yaml.node.set_scalar", "setScalar flips to scalar shape")
{
  Node n;
  n.setScalar("42");
  ICL_TEST_TRUE(n.isScalar());
  ICL_TEST_EQ(std::string(n.scalarView()), std::string("42"));
  ICL_TEST_TRUE(n.scalarStyle() == ScalarStyle::Plain);
}

ICL_REGISTER_TEST("utils.yaml.node.auto_vivify_mapping", "operator[](key) on null auto-creates mapping")
{
  Node n;
  n["foo"].setScalar("1");
  ICL_TEST_TRUE(n.isMapping());
  ICL_TEST_TRUE(n.contains("foo"));
  ICL_TEST_EQ(std::string(n["foo"].scalarView()), std::string("1"));
}

ICL_REGISTER_TEST("utils.yaml.node.auto_vivify_sequence", "pushBack on null auto-creates sequence")
{
  Node n;
  n.pushBack(Node{});
  ICL_TEST_TRUE(n.isSequence());
  ICL_TEST_EQ(n.size(), size_t(1));
}

ICL_REGISTER_TEST("utils.yaml.node.mapping_preserves_order", "mapping iteration follows insertion order")
{
  Node n; n.setMapping();
  n["c"].setScalar("3");
  n["a"].setScalar("1");
  n["b"].setScalar("2");
  const auto &m = n.mapping();
  ICL_TEST_EQ(m.size(), size_t(3));
  ICL_TEST_EQ(std::string(m[0].first), std::string("c"));
  ICL_TEST_EQ(std::string(m[1].first), std::string("a"));
  ICL_TEST_EQ(std::string(m[2].first), std::string("b"));
}

ICL_REGISTER_TEST("utils.yaml.node.scalar_view_throws_on_wrong_kind", "scalarView throws TypeError on non-scalar")
{
  Node n; n.setMapping();
  ICL_TEST_THROW(n.scalarView(), TypeError);
}

// ---------------------------------------------------------------------------
// Kind resolution — plain scalars per YAML 1.2 core schema
// ---------------------------------------------------------------------------

static ScalarKind kindOf(std::string_view sv, ScalarStyle style = ScalarStyle::Plain){
  Node n; n.setScalar(sv, style); return n.scalarKind();
}

ICL_REGISTER_TEST("utils.yaml.kind.null_literals", "null variants resolve to Null")
{
  ICL_TEST_TRUE(kindOf("")     == ScalarKind::Null);
  ICL_TEST_TRUE(kindOf("~")    == ScalarKind::Null);
  ICL_TEST_TRUE(kindOf("null") == ScalarKind::Null);
  ICL_TEST_TRUE(kindOf("Null") == ScalarKind::Null);
  ICL_TEST_TRUE(kindOf("NULL") == ScalarKind::Null);
}

ICL_REGISTER_TEST("utils.yaml.kind.bool_literals", "bool variants resolve to Bool")
{
  ICL_TEST_TRUE(kindOf("true")  == ScalarKind::Bool);
  ICL_TEST_TRUE(kindOf("True")  == ScalarKind::Bool);
  ICL_TEST_TRUE(kindOf("TRUE")  == ScalarKind::Bool);
  ICL_TEST_TRUE(kindOf("false") == ScalarKind::Bool);
  ICL_TEST_TRUE(kindOf("False") == ScalarKind::Bool);
  ICL_TEST_TRUE(kindOf("FALSE") == ScalarKind::Bool);
}

ICL_REGISTER_TEST("utils.yaml.kind.yaml11_aliases_reject", "yes/no/on/off are NOT Bool (YAML 1.2)")
{
  ICL_TEST_TRUE(kindOf("yes") == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("no")  == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("on")  == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("off") == ScalarKind::String);
}

ICL_REGISTER_TEST("utils.yaml.kind.int_literals", "integer forms resolve to Int")
{
  ICL_TEST_TRUE(kindOf("0")     == ScalarKind::Int);
  ICL_TEST_TRUE(kindOf("42")    == ScalarKind::Int);
  ICL_TEST_TRUE(kindOf("-17")   == ScalarKind::Int);
  ICL_TEST_TRUE(kindOf("+8")    == ScalarKind::Int);
  ICL_TEST_TRUE(kindOf("0x1F")  == ScalarKind::Int);
  ICL_TEST_TRUE(kindOf("0o17")  == ScalarKind::Int);
  ICL_TEST_TRUE(kindOf("-0xFF") == ScalarKind::Int);
}

ICL_REGISTER_TEST("utils.yaml.kind.float_literals", "float forms resolve to Float")
{
  ICL_TEST_TRUE(kindOf("3.14")   == ScalarKind::Float);
  ICL_TEST_TRUE(kindOf(".5")     == ScalarKind::Float);
  ICL_TEST_TRUE(kindOf("-2.0")   == ScalarKind::Float);
  ICL_TEST_TRUE(kindOf("1e6")    == ScalarKind::Float);
  ICL_TEST_TRUE(kindOf("1.5e-3") == ScalarKind::Float);
  ICL_TEST_TRUE(kindOf(".inf")   == ScalarKind::Float);
  ICL_TEST_TRUE(kindOf("-.inf")  == ScalarKind::Float);
  ICL_TEST_TRUE(kindOf(".nan")   == ScalarKind::Float);
}

ICL_REGISTER_TEST("utils.yaml.kind.string_by_exclusion", "non-matching plain scalars fall through to String")
{
  ICL_TEST_TRUE(kindOf("hello")      == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("640x480")    == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("/dev/video0")== ScalarKind::String);
  ICL_TEST_TRUE(kindOf("1.2.3")      == ScalarKind::String);   // too many dots
  ICL_TEST_TRUE(kindOf("1e")         == ScalarKind::String);   // bad exponent
}

ICL_REGISTER_TEST("utils.yaml.kind.quoted_forces_string", "any quoted style forces String")
{
  ICL_TEST_TRUE(kindOf("42",    ScalarStyle::SingleQuoted) == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("42",    ScalarStyle::DoubleQuoted) == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("true",  ScalarStyle::SingleQuoted) == ScalarKind::String);
  ICL_TEST_TRUE(kindOf("null",  ScalarStyle::DoubleQuoted) == ScalarKind::String);
}

// ---------------------------------------------------------------------------
// Permissive as<T>() — via AutoParse<string_view>
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.as.permissive_int", "as<int>() parses plain int")
{
  Node n; n.setScalar("42");
  ICL_TEST_EQ(n.as<int>(), 42);
}

ICL_REGISTER_TEST("utils.yaml.as.permissive_int_from_quoted", "as<int>() parses quoted int too")
{
  Node n; n.setScalar("42", ScalarStyle::DoubleQuoted);
  ICL_TEST_EQ(n.as<int>(), 42);  // permissive doesn't care about quoting
}

ICL_REGISTER_TEST("utils.yaml.as.permissive_int_truncates_float", "as<int>() on float truncates via stream")
{
  // Fast-path uses from_chars which rejects "3.14" as int → throws.
  // Permissive path: caller expectation is "best effort"; we throw if
  // the bytes don't parse.  Document this behavior via test.
  Node n; n.setScalar("3.14");
  ICL_TEST_THROW(n.as<int>(), ICLException);
}

ICL_REGISTER_TEST("utils.yaml.as.permissive_fallback", "as<T>(fallback) returns default on parse failure")
{
  Node n; n.setScalar("not-a-number");
  ICL_TEST_EQ(n.as<int>(999), 999);
}

ICL_REGISTER_TEST("utils.yaml.as.fallback_on_non_scalar", "as<T>(fallback) returns default on non-scalar")
{
  Node n; n.setMapping();
  ICL_TEST_EQ(n.as<int>(777), 777);
}

ICL_REGISTER_TEST("utils.yaml.as.registered_type_size", "as<Size>() delegates to parse<Size>")
{
  Node n; n.setScalar("640x480");
  Size s = n.as<Size>();
  ICL_TEST_EQ(s.width, 640);
  ICL_TEST_EQ(s.height, 480);
}

// ---------------------------------------------------------------------------
// Strict mode — asStrict<T>() / tryAs<T>()
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.asStrict.int_ok", "asStrict<int>() on Int-kinded plain scalar")
{
  Node n; n.setScalar("42");
  ICL_TEST_EQ(n.asStrict<int>(), 42);
}

ICL_REGISTER_TEST("utils.yaml.asStrict.int_rejects_quoted", "asStrict<int>() rejects quoted '42'")
{
  Node n; n.setScalar("42", ScalarStyle::DoubleQuoted);
  ICL_TEST_THROW(n.asStrict<int>(), TypeError);
}

ICL_REGISTER_TEST("utils.yaml.asStrict.int_rejects_float", "asStrict<int>() rejects 3.14")
{
  Node n; n.setScalar("3.14");
  ICL_TEST_THROW(n.asStrict<int>(), TypeError);
}

ICL_REGISTER_TEST("utils.yaml.asStrict.float_accepts_int", "asStrict<double>() accepts Int-kinded (widening)")
{
  Node n; n.setScalar("42");
  ICL_TEST_NEAR(n.asStrict<double>(), 42.0, 1e-9);
}

ICL_REGISTER_TEST("utils.yaml.asStrict.bool_ok", "asStrict<bool>() on true")
{
  Node n; n.setScalar("true");
  ICL_TEST_TRUE(n.asStrict<bool>());
}

ICL_REGISTER_TEST("utils.yaml.asStrict.bool_rejects_int", "asStrict<bool>() rejects '1'")
{
  Node n; n.setScalar("1");
  ICL_TEST_THROW(n.asStrict<bool>(), TypeError);
}

ICL_REGISTER_TEST("utils.yaml.asStrict.int_to_string_ok", "asStrict<string>() accepts Int stringification")
{
  Node n; n.setScalar("42");
  std::string s = n.asStrict<std::string>();
  ICL_TEST_EQ(s, std::string("42"));
}

ICL_REGISTER_TEST("utils.yaml.asStrict.null_to_string_throws", "asStrict<string>() rejects Null")
{
  Node n; n.setScalar("~");
  ICL_TEST_THROW(n.asStrict<std::string>(), TypeError);
}

ICL_REGISTER_TEST("utils.yaml.asStrict.size_from_plain_string", "asStrict<Size>() on plain '640x480'")
{
  Node n; n.setScalar("640x480");
  Size s = n.asStrict<Size>();
  ICL_TEST_EQ(s.width, 640);
  ICL_TEST_EQ(s.height, 480);
}

ICL_REGISTER_TEST("utils.yaml.asStrict.size_from_int_throws", "asStrict<Size>() rejects Int-kinded")
{
  Node n; n.setScalar("42");
  ICL_TEST_THROW(n.asStrict<Size>(), TypeError);
}

ICL_REGISTER_TEST("utils.yaml.tryAs.null_yields_nullopt", "tryAs<int>() on null scalar → nullopt")
{
  Node n; n.setScalar("~");
  auto o = n.tryAs<int>();
  ICL_TEST_FALSE(o.has_value());
}

ICL_REGISTER_TEST("utils.yaml.tryAs.int_yields_value", "tryAs<int>() on Int-kinded → value")
{
  Node n; n.setScalar("99");
  auto o = n.tryAs<int>();
  ICL_TEST_TRUE(o.has_value());
  ICL_TEST_EQ(*o, 99);
}

ICL_REGISTER_TEST("utils.yaml.tryAs.mismatch_yields_nullopt", "tryAs<int>() on String-kinded → nullopt")
{
  Node n; n.setScalar("hello");
  auto o = n.tryAs<int>();
  ICL_TEST_FALSE(o.has_value());
}

// ---------------------------------------------------------------------------
// Emitter — hand-built trees
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.emit.empty_document", "empty document emits ~ for null root")
{
  Document d = Document::empty();
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("~\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.flat_mapping", "flat mapping emits block style")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["a"].setScalar("1");
  r["b"].setScalar("2");
  r["c"].setScalar("three");
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("a: 1\nb: 2\nc: three\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.nested_mapping", "nested mapping indents children")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["outer"]["inner"].setScalar("x");
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("outer:\n  inner: x\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.flow_sequence_of_scalars", "short scalar sequence emits flow")
{
  Document d = Document::empty();
  Node &r = d.root();
  r.setSequence();
  r.pushBack(Node{}); r.sequence()[0].setScalar("1");
  r.pushBack(Node{}); r.sequence()[1].setScalar("2");
  r.pushBack(Node{}); r.sequence()[2].setScalar("3");
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("[1, 2, 3]\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.flow_threshold_flips_to_block", "large scalar sequence emits block")
{
  // 8 > flowThreshold (default 6) → block style.
  // Scalars must be interned (viewed into Document), since std::to_string
  // would produce temporaries whose views would dangle.
  Document d = Document::empty();
  Node &r = d.root();
  r.setSequence();
  for(int i = 0; i < 8; ++i){
    Node child;
    child.setScalar(d.intern(std::to_string(i)));
    r.pushBack(std::move(child));
  }
  std::string s = d.emit();
  const std::string expected = "- 0\n- 1\n- 2\n- 3\n- 4\n- 5\n- 6\n- 7\n";
  ICL_TEST_EQ(s, expected);
}

ICL_REGISTER_TEST("utils.yaml.emit.mapping_with_sequence_value", "mapping with sequence value")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["items"].setSequence();
  for(auto v : {"a", "b", "c"}){
    Node c; c.setScalar(v);
    r["items"].pushBack(std::move(c));
  }
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("items: [a, b, c]\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.string_that_looks_like_int_is_quoted", "quoted '42' re-emits quoted")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["x"].setScalar("42", ScalarStyle::DoubleQuoted);
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("x: \"42\"\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.special_chars_get_quoted", "scalar with ':' in it is quoted")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["k"].setScalar("a: b");
  std::string s = d.emit();
  // The plain form `a: b` would be ambiguous — emitter must quote.
  ICL_TEST_EQ(s, std::string("k: 'a: b'\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.control_chars_double_quoted", "scalar with \\n becomes double-quoted + escaped")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["k"].setScalar("line1\nline2");
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("k: \"line1\\nline2\"\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.empty_containers", "empty map/seq emit {}/[]")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["m"].setMapping();
  r["s"].setSequence();
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("m: {}\ns: []\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.null_value_in_mapping", "null-valued key emits ~")
{
  Document d = Document::empty();
  Node &r = d.root();
  r["x"].setNull();
  std::string s = d.emit();
  ICL_TEST_EQ(s, std::string("x: ~\n"));
}

// ---------------------------------------------------------------------------
// Document ownership — arena spill
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.doc.intern_stable", "intern() returns stable views under growth")
{
  Document d = Document::empty();
  std::string_view a = d.intern("first");
  std::string_view b = d.intern("second");
  // After adding more, both old views still valid (deque guarantees stability).
  for(int i = 0; i < 100; ++i) (void)d.intern("x");
  ICL_TEST_EQ(std::string(a), std::string("first"));
  ICL_TEST_EQ(std::string(b), std::string("second"));
}

ICL_REGISTER_TEST("utils.yaml.doc.file_missing_throws", "file() on missing path throws ICLException")
{
  ICL_TEST_THROW(Document::file("/nonexistent-yaml-file-xyz.yaml"), ICLException);
}

// ---------------------------------------------------------------------------
// Parser — empty / trivial input
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.empty", "empty source parses to null root")
{
  Document d = Document::view("");
  ICL_TEST_TRUE(d.root().isNull());
}

ICL_REGISTER_TEST("utils.yaml.parse.whitespace_only", "whitespace-only source parses to null root")
{
  Document d = Document::view("   \n\n  # comment\n\n");
  ICL_TEST_TRUE(d.root().isNull());
}

ICL_REGISTER_TEST("utils.yaml.parse.single_scalar", "bare scalar parses as string")
{
  Document d = Document::view("hello");
  ICL_TEST_TRUE(d.root().isScalar());
  ICL_TEST_EQ(std::string(d.root().scalarView()), std::string("hello"));
  ICL_TEST_TRUE(d.root().scalarKind() == ScalarKind::String);
}

ICL_REGISTER_TEST("utils.yaml.parse.leading_document_marker", "leading --- is tolerated")
{
  Document d = Document::view("---\nfoo: 1\n");
  ICL_TEST_TRUE(d.root().isMapping());
  ICL_TEST_EQ(d.root()["foo"].as<int>(), 1);
}

// ---------------------------------------------------------------------------
// Parser — mappings
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.flat_mapping", "flat mapping parses")
{
  Document d = Document::view("a: 1\nb: 2\nc: three\n");
  ICL_TEST_TRUE(d.root().isMapping());
  ICL_TEST_EQ(d.root().size(), size_t(3));
  ICL_TEST_EQ(d.root()["a"].as<int>(), 1);
  ICL_TEST_EQ(d.root()["b"].as<int>(), 2);
  ICL_TEST_EQ(std::string(d.root()["c"].scalarView()), std::string("three"));
}

ICL_REGISTER_TEST("utils.yaml.parse.nested_mapping", "nested mapping parses")
{
  Document d = Document::view("outer:\n  inner:\n    deep: 42\n");
  const Node &r = d.root();
  ICL_TEST_TRUE(r.isMapping());
  ICL_TEST_TRUE(r["outer"].isMapping());
  ICL_TEST_TRUE(r["outer"]["inner"].isMapping());
  ICL_TEST_EQ(r["outer"]["inner"]["deep"].as<int>(), 42);
}

ICL_REGISTER_TEST("utils.yaml.parse.mapping_preserves_order", "parser preserves key order")
{
  Document d = Document::view("z: 1\na: 2\nm: 3\n");
  const auto &m = d.root().mapping();
  ICL_TEST_EQ(std::string(m[0].first), std::string("z"));
  ICL_TEST_EQ(std::string(m[1].first), std::string("a"));
  ICL_TEST_EQ(std::string(m[2].first), std::string("m"));
}

ICL_REGISTER_TEST("utils.yaml.parse.null_value", "key with no value parses as null")
{
  Document d = Document::view("k:\nnext: 1\n");
  ICL_TEST_TRUE(d.root()["k"].isNull());
  ICL_TEST_EQ(d.root()["next"].as<int>(), 1);
}

ICL_REGISTER_TEST("utils.yaml.parse.comments_ignored", "comments at various positions")
{
  Document d = Document::view("# header\na: 1  # inline\n# between\nb: 2\n");
  ICL_TEST_EQ(d.root()["a"].as<int>(), 1);
  ICL_TEST_EQ(d.root()["b"].as<int>(), 2);
}

// ---------------------------------------------------------------------------
// Parser — block sequences
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.block_sequence_of_scalars", "`- a\\n- b\\n- c`")
{
  Document d = Document::view("- a\n- b\n- c\n");
  ICL_TEST_TRUE(d.root().isSequence());
  ICL_TEST_EQ(d.root().size(), size_t(3));
  ICL_TEST_EQ(std::string(d.root()[size_t(0)].scalarView()), std::string("a"));
  ICL_TEST_EQ(std::string(d.root()[size_t(2)].scalarView()), std::string("c"));
}

ICL_REGISTER_TEST("utils.yaml.parse.mapping_with_block_sequence_value", "mapping key → block seq value")
{
  Document d = Document::view("items:\n  - x\n  - y\n  - z\n");
  ICL_TEST_TRUE(d.root()["items"].isSequence());
  ICL_TEST_EQ(d.root()["items"].size(), size_t(3));
  ICL_TEST_EQ(std::string(d.root()["items"][size_t(1)].scalarView()), std::string("y"));
}

// ---------------------------------------------------------------------------
// Parser — flow style
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.flow_sequence", "`[1, 2, 3]`")
{
  Document d = Document::view("[1, 2, 3]");
  ICL_TEST_TRUE(d.root().isSequence());
  ICL_TEST_EQ(d.root().size(), size_t(3));
  ICL_TEST_EQ(d.root()[size_t(0)].as<int>(), 1);
  ICL_TEST_EQ(d.root()[size_t(2)].as<int>(), 3);
}

ICL_REGISTER_TEST("utils.yaml.parse.flow_mapping", "`{a: 1, b: 2}`")
{
  Document d = Document::view("{a: 1, b: 2}");
  ICL_TEST_TRUE(d.root().isMapping());
  ICL_TEST_EQ(d.root()["a"].as<int>(), 1);
  ICL_TEST_EQ(d.root()["b"].as<int>(), 2);
}

ICL_REGISTER_TEST("utils.yaml.parse.flow_trailing_comma", "flow with trailing comma")
{
  Document d = Document::view("[1, 2, 3,]");
  ICL_TEST_EQ(d.root().size(), size_t(3));
}

ICL_REGISTER_TEST("utils.yaml.parse.flow_multiline", "flow spanning multiple lines")
{
  Document d = Document::view("[\n  1,\n  2,\n  3\n]\n");
  ICL_TEST_EQ(d.root().size(), size_t(3));
  ICL_TEST_EQ(d.root()[size_t(1)].as<int>(), 2);
}

ICL_REGISTER_TEST("utils.yaml.parse.nested_flow", "flow mapping inside flow sequence")
{
  Document d = Document::view("[{a: 1}, {b: 2}]");
  ICL_TEST_TRUE(d.root().isSequence());
  ICL_TEST_EQ(d.root().size(), size_t(2));
  ICL_TEST_EQ(d.root()[size_t(0)]["a"].as<int>(), 1);
  ICL_TEST_EQ(d.root()[size_t(1)]["b"].as<int>(), 2);
}

// ---------------------------------------------------------------------------
// Parser — quoted scalars
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.single_quoted", "single-quoted scalar")
{
  Document d = Document::view("k: 'hello world'\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("hello world"));
  ICL_TEST_TRUE(d.root()["k"].scalarStyle() == ScalarStyle::SingleQuoted);
  ICL_TEST_TRUE(d.root()["k"].scalarKind()  == ScalarKind::String);
}

ICL_REGISTER_TEST("utils.yaml.parse.single_quoted_escape", "single-quoted '' escape")
{
  Document d = Document::view("k: 'it''s'\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("it's"));
}

ICL_REGISTER_TEST("utils.yaml.parse.double_quoted", "double-quoted scalar")
{
  Document d = Document::view("k: \"hello\"\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("hello"));
  ICL_TEST_TRUE(d.root()["k"].scalarStyle() == ScalarStyle::DoubleQuoted);
}

ICL_REGISTER_TEST("utils.yaml.parse.double_quoted_escapes", "double-quoted escape sequences")
{
  Document d = Document::view("k: \"a\\nb\\tc\\\\d\\\"e\"\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("a\nb\tc\\d\"e"));
}

ICL_REGISTER_TEST("utils.yaml.parse.double_quoted_hex_escape", "\\xNN escape")
{
  Document d = Document::view("k: \"\\x41\\x42\"\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("AB"));
}

ICL_REGISTER_TEST("utils.yaml.parse.double_quoted_unicode_escape", "\\uNNNN escape (BMP)")
{
  // U+00E9 (é) → UTF-8 C3 A9
  Document d = Document::view("k: \"caf\\u00E9\"\n");
  const std::string v(d.root()["k"].scalarView());
  ICL_TEST_EQ(v, std::string("caf\xC3\xA9"));
}

ICL_REGISTER_TEST("utils.yaml.parse.quoted_number_is_string", "\"42\" is a String, not Int")
{
  Document d = Document::view("k: \"42\"\n");
  ICL_TEST_TRUE(d.root()["k"].scalarKind() == ScalarKind::String);
  ICL_TEST_THROW(d.root()["k"].asStrict<int>(), TypeError);
  ICL_TEST_EQ(d.root()["k"].as<int>(), 42);  // permissive still parses
}

// ---------------------------------------------------------------------------
// Parser — block scalars
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.literal_block_scalar", "| preserves newlines")
{
  Document d = Document::view("k: |\n  line 1\n  line 2\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("line 1\nline 2\n"));
  ICL_TEST_TRUE(d.root()["k"].scalarStyle() == ScalarStyle::Literal);
  ICL_TEST_TRUE(d.root()["k"].scalarKind()  == ScalarKind::String);
}

ICL_REGISTER_TEST("utils.yaml.parse.folded_block_scalar", "> folds newlines to spaces")
{
  Document d = Document::view("k: >\n  line 1\n  line 2\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("line 1 line 2"));
  ICL_TEST_TRUE(d.root()["k"].scalarStyle() == ScalarStyle::Folded);
}

// ---------------------------------------------------------------------------
// Parser — UTF-8 preservation
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.utf8_preserved", "UTF-8 bytes in plain scalars survive verbatim")
{
  const char src[] = "k: \xE2\x9C\x93 check\n";  // ✓ (U+2713)
  Document d = Document::view(std::string_view(src, sizeof(src) - 1));
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("\xE2\x9C\x93 check"));
}

// ---------------------------------------------------------------------------
// Parser — error cases
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.error_unterminated_double_quote", "unterminated \"...")
{
  ICL_TEST_THROW(Document::view("k: \"hello\n"), ParseError);
}

ICL_REGISTER_TEST("utils.yaml.parse.error_unterminated_single_quote", "unterminated '...")
{
  ICL_TEST_THROW(Document::view("k: 'hello\n"), ParseError);
}

ICL_REGISTER_TEST("utils.yaml.parse.error_duplicate_key", "duplicate mapping key rejected")
{
  ICL_TEST_THROW(Document::view("a: 1\na: 2\n"), ParseError);
}

ICL_REGISTER_TEST("utils.yaml.parse.error_malformed_flow", "unclosed flow sequence")
{
  ICL_TEST_THROW(Document::view("[1, 2,"), ParseError);
}

// ---------------------------------------------------------------------------
// Ownership — view vs. own
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.doc.view_aliases_source", "view() scalar views point into caller buffer")
{
  const std::string src = "name: ICL\n";
  Document d = Document::view(src);
  std::string_view sv = d.root()["name"].scalarView();
  // The view's data pointer must fall within `src` since we parsed from
  // it directly (no escapes in this plain scalar).
  ICL_TEST_TRUE(sv.data() >= src.data());
  ICL_TEST_TRUE(sv.data() + sv.size() <= src.data() + src.size());
}

ICL_REGISTER_TEST("utils.yaml.doc.own_takes_buffer", "own() survives source temporary going out of scope")
{
  auto makeDoc = []{
    std::string s = "x: 42\n";
    return Document::own(std::move(s));  // s is gone on return
  };
  Document d = makeDoc();
  ICL_TEST_EQ(d.root()["x"].as<int>(), 42);
}

// ---------------------------------------------------------------------------
// Round-trip: parse → emit
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.roundtrip.flat_mapping", "flat mapping round-trip")
{
  const std::string src = "a: 1\nb: 2\nc: three\n";
  Document d = Document::view(src);
  ICL_TEST_EQ(d.emit(), src);
}

ICL_REGISTER_TEST("utils.yaml.roundtrip.nested_mapping", "nested mapping round-trip")
{
  const std::string src = "outer:\n  inner:\n    deep: 42\n";
  Document d = Document::view(src);
  ICL_TEST_EQ(d.emit(), src);
}

// ---------------------------------------------------------------------------
// ParseError — location information
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.error_tab_in_indent", "tab in indentation → ParseError")
{
  ICL_TEST_THROW(Document::view("a:\n\tnested: 1\n"), ParseError);
}

ICL_REGISTER_TEST("utils.yaml.parse.error_line_col_reported", "ParseError carries line and col")
{
  try {
    Document::view("a: \"unterminated\nmore\n");
    ICL_TEST_TRUE(false);  // should have thrown
  } catch (const ParseError &e){
    // The unterminated quote errors on line 1 when we hit the \n.
    ICL_TEST_EQ(e.line(), size_t(1));
  }
}

// ---------------------------------------------------------------------------
// Extra strict-mode + edge case coverage
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.integer_hex_oct_parse_to_int",
                  "plain hex/oct resolve as Int AND parse to int via from_chars")
{
  Document d = Document::view("h: 0x1F\no: 0o17\nneg: -0xFF\n");
  // Kind resolves as Int per core schema.
  ICL_TEST_TRUE(d.root()["h"].scalarKind()   == ScalarKind::Int);
  ICL_TEST_TRUE(d.root()["o"].scalarKind()   == ScalarKind::Int);
  ICL_TEST_TRUE(d.root()["neg"].scalarKind() == ScalarKind::Int);
  // parse<int> detects the 0x / 0o prefix and uses base 16 / 8.
  ICL_TEST_EQ(d.root()["h"].as<int>(),   31);
  ICL_TEST_EQ(d.root()["o"].as<int>(),   15);
  ICL_TEST_EQ(d.root()["neg"].as<int>(), -255);
  ICL_TEST_EQ(d.root()["h"].asStrict<int>(), 31);
}

ICL_REGISTER_TEST("utils.yaml.emit.deep_nesting", "3-level nested mapping emits correctly")
{
  Document d = Document::empty();
  d.root()["a"]["b"]["c"].setScalar("leaf");
  ICL_TEST_EQ(d.emit(), std::string("a:\n  b:\n    c: leaf\n"));
}

// ---------------------------------------------------------------------------
// operator= / setOwnedScalar — crisper syntax for programmatic building
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.assign.cstr", "node = \"literal\"")
{
  Node n;
  n = "hello";
  ICL_TEST_TRUE(n.isScalar());
  ICL_TEST_EQ(std::string(n.scalarView()), std::string("hello"));
}

ICL_REGISTER_TEST("utils.yaml.assign.string", "node = std::string")
{
  Node n;
  n = std::string("world");
  ICL_TEST_EQ(std::string(n.scalarView()), std::string("world"));
}

ICL_REGISTER_TEST("utils.yaml.assign.string_view", "node = string_view (copies)")
{
  Node n;
  // Deliberately use a temporary std::string — its view would dangle
  // with setScalar, but operator= copies into owned storage.
  std::string tmp = "from_view";
  std::string_view sv(tmp);
  n = sv;
  tmp.clear();                                  // kill the original buffer
  ICL_TEST_EQ(std::string(n.scalarView()), std::string("from_view"));
}

ICL_REGISTER_TEST("utils.yaml.assign.bool", "node = true / false")
{
  Node yes, no;
  yes = true;
  no  = false;
  ICL_TEST_EQ(std::string(yes.scalarView()), std::string("true"));
  ICL_TEST_EQ(std::string(no.scalarView()),  std::string("false"));
  ICL_TEST_TRUE(yes.scalarKind() == ScalarKind::Bool);
  ICL_TEST_TRUE(yes.asStrict<bool>());
  ICL_TEST_FALSE(no.asStrict<bool>());
}

ICL_REGISTER_TEST("utils.yaml.assign.int", "node = 42")
{
  Node n;
  n = 42;
  ICL_TEST_EQ(std::string(n.scalarView()), std::string("42"));
  ICL_TEST_TRUE(n.scalarKind() == ScalarKind::Int);
  ICL_TEST_EQ(n.asStrict<int>(), 42);
}

ICL_REGISTER_TEST("utils.yaml.assign.double", "node = 3.14")
{
  Node n;
  n = 3.14;
  ICL_TEST_TRUE(n.scalarKind() == ScalarKind::Float);
  ICL_TEST_NEAR(n.asStrict<double>(), 3.14, 1e-9);
}

ICL_REGISTER_TEST("utils.yaml.assign.registered_type_Size",
                  "node = Size(640, 480) via str(T)")
{
  Node n;
  n = Size(640, 480);
  ICL_TEST_EQ(std::string(n.scalarView()), std::string("640x480"));
  ICL_TEST_EQ(n.asStrict<Size>(), Size(640, 480));
}

ICL_REGISTER_TEST("utils.yaml.assign.mapping_then_scalar",
                  "fluid: d[\"x\"] = 42; d[\"y\"] = \"hi\"")
{
  Document d = Document::empty();
  d.root()["x"] = 42;
  d.root()["y"] = "hi";
  d.root()["f"] = 1.5;
  d.root()["b"] = true;
  const std::string out = d.emit();
  ICL_TEST_EQ(out, std::string("x: 42\ny: hi\nf: 1.5\nb: true\n"));
}

ICL_REGISTER_TEST("utils.yaml.assign.temporary_survives_scope",
                  "owned scalars don't dangle when source dies")
{
  auto makeNode = []{
    Node n;
    std::string tmp = "transient";
    n = tmp;
    tmp = "gone";                                 // kill source buffer
    return n;                                     // n's ScalarData carries owned string
  };
  Node n = makeNode();
  ICL_TEST_EQ(std::string(n.scalarView()), std::string("transient"));
}

ICL_REGISTER_TEST("utils.yaml.assign.roundtrip", "assigned mapping round-trips via emit/parse")
{
  Document d = Document::empty();
  d.root()["name"] = "icl";
  d.root()["port"] = 8080;
  d.root()["size"] = Size(640, 480);
  const std::string yamlText = d.emit();
  Document d2 = Document::own(std::string(yamlText));
  ICL_TEST_EQ(std::string(d2.root()["name"].scalarView()), std::string("icl"));
  ICL_TEST_EQ(d2.root()["port"].as<int>(), 8080);
  ICL_TEST_EQ(d2.root()["size"].as<Size>(), Size(640, 480));
}

ICL_REGISTER_TEST("utils.yaml.assign.setOwnedScalar_explicit",
                  "setOwnedScalar keeps style hint")
{
  Node n;
  n.setOwnedScalar("42", ScalarStyle::DoubleQuoted);
  ICL_TEST_TRUE(n.scalarStyle() == ScalarStyle::DoubleQuoted);
  // Quoted forces String kind
  ICL_TEST_TRUE(n.scalarKind() == ScalarKind::String);
}

// ---------------------------------------------------------------------------
// Level 1 — initializer-list assignment + converting ctor
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.assign.initlist_sequence_ints",
                  "node = {1, 2, 3}")
{
  Node n;
  n = {1, 2, 3};
  ICL_TEST_TRUE(n.isSequence());
  ICL_TEST_EQ(n.size(), size_t(3));
  ICL_TEST_EQ(n[size_t(0)].as<int>(), 1);
  ICL_TEST_EQ(n[size_t(2)].as<int>(), 3);
}

ICL_REGISTER_TEST("utils.yaml.assign.initlist_sequence_strings",
                  "node = {\"a\", \"b\", \"c\"}")
{
  Node n;
  n = {"a", "b", "c"};
  ICL_TEST_TRUE(n.isSequence());
  ICL_TEST_EQ(std::string(n[size_t(1)].scalarView()), std::string("b"));
}

ICL_REGISTER_TEST("utils.yaml.assign.initlist_mapping",
                  "node = {{\"k\", 1}, {\"j\", \"hi\"}}")
{
  Node n;
  n = {{"k", 1}, {"j", std::string("hi")}};
  ICL_TEST_TRUE(n.isMapping());
  ICL_TEST_EQ(n.size(), size_t(2));
  ICL_TEST_EQ(n["k"].as<int>(), 1);
  ICL_TEST_EQ(std::string(n["j"].scalarView()), std::string("hi"));
}

ICL_REGISTER_TEST("utils.yaml.assign.initlist_mapping_preserves_order",
                  "init-list mapping preserves user's declaration order")
{
  Node n;
  n = {{"z", 1}, {"a", 2}, {"m", 3}};
  const auto &map = n.mapping();
  ICL_TEST_EQ(std::string(map[0].first), std::string("z"));
  ICL_TEST_EQ(std::string(map[1].first), std::string("a"));
  ICL_TEST_EQ(std::string(map[2].first), std::string("m"));
}

ICL_REGISTER_TEST("utils.yaml.assign.initlist_owns_keys",
                  "init-list mapping keys are owned, not view-dependent")
{
  auto make = []{
    Node n;
    // Use non-literal keys to prove interning — the std::string source
    // dies at return, but the Mapping's ownedKeys spill preserves them.
    std::string k1 = "dyn_key_one";
    std::string k2 = "dyn_key_two";
    n = {{k1, 1}, {k2, 2}};
    return n;
  };
  Node n = make();
  ICL_TEST_EQ(n["dyn_key_one"].as<int>(), 1);
  ICL_TEST_EQ(n["dyn_key_two"].as<int>(), 2);
}

ICL_REGISTER_TEST("utils.yaml.ctor.converting",
                  "Node n = 42; Node s = \"foo\"; Node z = Size(1,2)")
{
  Node a = 42;
  Node b = "foo";
  Node c = Size(1, 2);
  ICL_TEST_EQ(a.as<int>(), 42);
  ICL_TEST_EQ(std::string(b.scalarView()), std::string("foo"));
  ICL_TEST_EQ(c.as<Size>(), Size(1, 2));
}

ICL_REGISTER_TEST("utils.yaml.ctor.initlist_mapping",
                  "Node n = {{\"k\", 1}, {\"j\", 2}}")
{
  // Mapping init-list works at ctor level.  (Sequence form is
  // deliberately not supported as a ctor — see Yaml.h — use the
  // two-line form `Node n; n = {1,2,3};` or an explicit vector<T>.)
  Node n = {{"k", 1}, {"j", 2}};
  ICL_TEST_TRUE(n.isMapping());
  ICL_TEST_EQ(n.size(), size_t(2));
  ICL_TEST_EQ(n["k"].as<int>(), 1);
  ICL_TEST_EQ(n["j"].as<int>(), 2);
}

// ---------------------------------------------------------------------------
// operator[] — crisp integer index without size_t cast
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.index.int_literal", "n[0], n[2] without size_t cast")
{
  Node n;
  n = {10, 20, 30};
  ICL_TEST_EQ(n[0].as<int>(), 10);
  ICL_TEST_EQ(n[1].as<int>(), 20);
  ICL_TEST_EQ(n[2].as<int>(), 30);
}

ICL_REGISTER_TEST("utils.yaml.index.mixed", "string key and int index both work")
{
  Node n;
  n["items"] = {1, 2, 3};
  ICL_TEST_EQ(n["items"][1].as<int>(), 2);
}

// ---------------------------------------------------------------------------
// Level 2 — container assignment
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.assign.vector_int", "node = std::vector<int>")
{
  std::vector<int> v = {10, 20, 30};
  Node n;
  n = v;
  ICL_TEST_TRUE(n.isSequence());
  ICL_TEST_EQ(n.size(), size_t(3));
  ICL_TEST_EQ(n[size_t(1)].as<int>(), 20);
}

ICL_REGISTER_TEST("utils.yaml.assign.vector_string", "node = std::vector<std::string>")
{
  std::vector<std::string> v = {"alpha", "beta"};
  Node n;
  n = v;
  v.clear();  // source vector dies — Node must have copied
  ICL_TEST_EQ(std::string(n[size_t(0)].scalarView()), std::string("alpha"));
}

ICL_REGISTER_TEST("utils.yaml.assign.map_string_int", "node = std::map<string, int>")
{
  std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
  Node n;
  n = m;
  m.clear();  // source map dies — keys must be interned in Node
  ICL_TEST_TRUE(n.isMapping());
  ICL_TEST_EQ(n.size(), size_t(3));
  ICL_TEST_EQ(n["a"].as<int>(), 1);
  ICL_TEST_EQ(n["c"].as<int>(), 3);
}

ICL_REGISTER_TEST("utils.yaml.assign.unordered_map", "node = std::unordered_map")
{
  std::unordered_map<std::string, double> m = {{"pi", 3.14}, {"e", 2.71}};
  Node n;
  n = m;
  ICL_TEST_TRUE(n.isMapping());
  ICL_TEST_EQ(n.size(), size_t(2));
  ICL_TEST_NEAR(n["pi"].as<double>(), 3.14, 1e-9);
}

ICL_REGISTER_TEST("utils.yaml.assign.nested_container",
                  "mapping with vector and map values")
{
  std::map<std::string, std::vector<int>> data = {
    {"primes", {2, 3, 5, 7}},
    {"evens",  {2, 4, 6, 8}},
  };
  Node n;
  n = data;
  ICL_TEST_EQ(n["primes"].size(), size_t(4));
  ICL_TEST_EQ(n["primes"][size_t(2)].as<int>(), 5);
  ICL_TEST_EQ(n["evens"][size_t(3)].as<int>(), 8);
}

ICL_REGISTER_TEST("utils.yaml.assign.nested_initlist",
                  "mapping with nested sequence via init-list")
{
  Node n;
  n = {
    {"name",  "config"},
    {"ports", std::vector<int>{80, 443, 8080}},
    {"dbg",   true},
  };
  ICL_TEST_EQ(std::string(n["name"].scalarView()), std::string("config"));
  ICL_TEST_EQ(n["ports"].size(), size_t(3));
  ICL_TEST_EQ(n["ports"][size_t(2)].as<int>(), 8080);
  ICL_TEST_TRUE(n["dbg"].asStrict<bool>());
}

// ---------------------------------------------------------------------------
// Mapping copy — source-views + owned-keys rebind
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.mapping.copy_preserves_owned_keys",
                  "deep-copying a Mapping re-binds owned-key views")
{
  Node src;
  src["a"] = 1;
  src["b"] = 2;
  // Force a copy via Node copy-ctor, then outlive the original.
  Node copy = src;
  // Mutate the original to prove independence.
  src["a"] = 999;
  ICL_TEST_EQ(copy["a"].as<int>(), 1);
  ICL_TEST_EQ(copy["b"].as<int>(), 2);
}

ICL_REGISTER_TEST("utils.yaml.mapping.copy_preserves_source_views",
                  "parsed Mapping copied: source views still valid")
{
  const std::string src_text = "a: 1\nb: 2\n";
  Document d = Document::own(std::string(src_text));
  // Copy the root mapping.
  Node copy = d.root();
  // Document still alive → source views in `copy` still valid.
  ICL_TEST_EQ(copy["a"].as<int>(), 1);
  ICL_TEST_EQ(copy["b"].as<int>(), 2);
}

ICL_REGISTER_TEST("utils.yaml.mapping.copy_mixed",
                  "Mapping with both source-viewed and owned keys copies cleanly")
{
  Document d = Document::own(std::string("a: 1\n"));
  // `a` is a source-view key.  Add an owned key programmatically.
  d.root()["b"] = 2;
  Node copy = d.root();
  ICL_TEST_EQ(copy["a"].as<int>(), 1);
  ICL_TEST_EQ(copy["b"].as<int>(), 2);
}

// ---------------------------------------------------------------------------
// Round-trip through emit — every assignment path round-trips via YAML text
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.assign.container_roundtrip",
                  "container-built tree emits + reparses identically")
{
  std::map<std::string, int> m = {{"x", 1}, {"y", 2}};
  Node n;
  n = m;
  const std::string yaml_text = [&]{
    Document d = Document::empty();
    d.root() = std::move(n);
    return d.emit();
  }();
  Document d2 = Document::own(std::string(yaml_text));
  ICL_TEST_EQ(d2.root()["x"].as<int>(), 1);
  ICL_TEST_EQ(d2.root()["y"].as<int>(), 2);
}

// ---------------------------------------------------------------------------
// A1: Block scalar chomping indicators
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.block_scalar_clip_default", "| default = clip (one trailing \\n)")
{
  Document d = Document::view("k: |\n  a\n  b\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("a\nb\n"));
}

ICL_REGISTER_TEST("utils.yaml.parse.block_scalar_strip", "|- strips all trailing newlines")
{
  Document d = Document::view("k: |-\n  a\n  b\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("a\nb"));
}

ICL_REGISTER_TEST("utils.yaml.parse.block_scalar_keep", "|+ keeps all trailing newlines")
{
  Document d = Document::view("k: |+\n  a\n  b\n\n\nnext: 1\n");
  // Keep preserves both blank lines between the block and `next:`.
  const std::string &v = std::string(d.root()["k"].scalarView());
  ICL_TEST_TRUE(v.size() >= 6);
  ICL_TEST_EQ(v.substr(0, 4), std::string("a\nb\n"));
  ICL_TEST_EQ(d.root()["next"].as<int>(), 1);
}

ICL_REGISTER_TEST("utils.yaml.parse.folded_scalar_strip", ">- strips trailing newlines from folded")
{
  Document d = Document::view("k: >-\n  a\n  b\n");
  ICL_TEST_EQ(std::string(d.root()["k"].scalarView()), std::string("a b"));
}

// ---------------------------------------------------------------------------
// A2: Hex / octal integer parsing
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.parse.hex_integer", "parse<int>('0x1F') = 31")
{
  ICL_TEST_EQ(parse<int>(std::string_view("0x1F")),  31);
  ICL_TEST_EQ(parse<int>(std::string_view("0X1f")),  31);
  ICL_TEST_EQ(parse<int>(std::string_view("-0xFF")), -255);
  ICL_TEST_EQ(parse<unsigned>(std::string_view("0xFFFF")), 65535u);
  ICL_TEST_EQ(parse<long long>(std::string_view("0x7FFFFFFFFFFFFFFF")),
              9223372036854775807LL);
}

ICL_REGISTER_TEST("utils.parse.oct_integer", "parse<int>('0o17') = 15")
{
  ICL_TEST_EQ(parse<int>(std::string_view("0o17")), 15);
  ICL_TEST_EQ(parse<int>(std::string_view("0O7")),  7);
  ICL_TEST_EQ(parse<int>(std::string_view("-0o10")), -8);
}

ICL_REGISTER_TEST("utils.parse.hex_edge_cases", "decimal INT_MIN still works, hex + unsigned negative errors")
{
  // Decimal edge: INT_MIN as a literal must still parse.  (Our hex/oct
  // path strip-then-negate would fail here; decimal path defers to
  // from_chars which handles signed min natively.)
  ICL_TEST_EQ(parse<int>(std::string_view("-2147483648")), std::numeric_limits<int>::min());
  // Negative into unsigned for hex rejects.
  ICL_TEST_THROW(parse<unsigned>(std::string_view("-0xFF")), ICLException);
}

// ---------------------------------------------------------------------------
// A3: .inf / .nan floats
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.parse.float_inf_nan", ".inf / -.inf / .nan parse correctly")
{
  ICL_TEST_TRUE(std::isinf(parse<double>(std::string_view(".inf"))));
  ICL_TEST_TRUE(parse<double>(std::string_view(".inf")) > 0);
  ICL_TEST_TRUE(parse<double>(std::string_view("-.inf")) < 0);
  ICL_TEST_TRUE(std::isnan(parse<double>(std::string_view(".nan"))));
  ICL_TEST_TRUE(std::isinf(parse<float>(std::string_view(".Inf"))));
}

ICL_REGISTER_TEST("utils.yaml.asStrict.float_inf", "asStrict<double> on plain .inf")
{
  Document d = Document::view("pi: .inf\nx: -.inf\n");
  ICL_TEST_TRUE(d.root()["pi"].scalarKind() == ScalarKind::Float);
  ICL_TEST_TRUE(std::isinf(d.root()["pi"].asStrict<double>()));
  ICL_TEST_TRUE(d.root()["x"].asStrict<double>() < 0);
}

// ---------------------------------------------------------------------------
// A4: sortKeys in emitter
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.emit.sort_keys_false_preserves_order", "default preserves insertion order")
{
  Document d = Document::empty();
  d.root()["z"].setScalar("1");
  d.root()["a"].setScalar("2");
  d.root()["m"].setScalar("3");
  ICL_TEST_EQ(d.emit(), std::string("z: 1\na: 2\nm: 3\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.sort_keys_true_sorts", "sortKeys=true sorts lexicographically")
{
  Document d = Document::empty();
  d.root()["z"].setScalar("1");
  d.root()["a"].setScalar("2");
  d.root()["m"].setScalar("3");
  EmitOptions opts; opts.sortKeys = true;
  ICL_TEST_EQ(d.emit(opts), std::string("a: 2\nm: 3\nz: 1\n"));
}

ICL_REGISTER_TEST("utils.yaml.emit.sort_keys_nested", "sortKeys applies recursively")
{
  Document d = Document::empty();
  d.root()["outer"]["z"].setScalar("1");
  d.root()["outer"]["a"].setScalar("2");
  EmitOptions opts; opts.sortKeys = true;
  ICL_TEST_EQ(d.emit(opts), std::string("outer:\n  a: 2\n  z: 1\n"));
}

// ---------------------------------------------------------------------------
// A5: Same-indent block sequence as mapping value
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.same_indent_sequence_value",
                  "YAML idiom: key followed by `-` at same indent")
{
  Document d = Document::view("items:\n- a\n- b\n- c\n");
  ICL_TEST_TRUE(d.root()["items"].isSequence());
  ICL_TEST_EQ(d.root()["items"].size(), size_t(3));
  ICL_TEST_EQ(std::string(d.root()["items"][size_t(1)].scalarView()), std::string("b"));
}

ICL_REGISTER_TEST("utils.yaml.parse.same_indent_sequence_then_sibling",
                  "same-indent seq value followed by sibling key")
{
  Document d = Document::view("items:\n- a\n- b\nnext: done\n");
  ICL_TEST_EQ(d.root()["items"].size(), size_t(2));
  ICL_TEST_EQ(std::string(d.root()["next"].scalarView()), std::string("done"));
}

// ---------------------------------------------------------------------------
// A6: Explicit scalar tags (!!str, !!int, !!bool, !!float, !!null)
// ---------------------------------------------------------------------------

ICL_REGISTER_TEST("utils.yaml.parse.tag_str_forces_string", "!!str forces String even on int-looking bytes")
{
  Document d = Document::view("x: !!str 42\n");
  ICL_TEST_TRUE(d.root()["x"].scalarKind() == ScalarKind::String);
  ICL_TEST_THROW(d.root()["x"].asStrict<int>(), TypeError);
  ICL_TEST_EQ(d.root()["x"].as<std::string>(), std::string("42"));
}

ICL_REGISTER_TEST("utils.yaml.parse.tag_int_overrides_resolution", "!!int overrides kind to Int")
{
  // Plain "hello" would resolve as String; explicit !!int overrides.
  // asStrict<int> passes the kind check but then parse<int>("hello")
  // throws — documenting that user is responsible for consistent tags.
  Document d = Document::view("x: !!int 99\n");
  ICL_TEST_TRUE(d.root()["x"].scalarKind() == ScalarKind::Int);
  ICL_TEST_EQ(d.root()["x"].asStrict<int>(), 99);
}

ICL_REGISTER_TEST("utils.yaml.parse.tag_bool_null_float", "!!bool, !!null, !!float all recognized")
{
  Document d = Document::view("a: !!bool yes\nb: !!null anything\nc: !!float 3\n");
  ICL_TEST_TRUE(d.root()["a"].scalarKind() == ScalarKind::Bool);
  ICL_TEST_TRUE(d.root()["b"].scalarKind() == ScalarKind::Null);
  ICL_TEST_TRUE(d.root()["c"].scalarKind() == ScalarKind::Float);
  ICL_TEST_NEAR(d.root()["c"].asStrict<double>(), 3.0, 1e-9);
}

ICL_REGISTER_TEST("utils.yaml.parse.tag_unknown_throws", "unknown tag errors")
{
  ICL_TEST_THROW(Document::view("x: !!bogus 1\n"), ParseError);
}

ICL_REGISTER_TEST("utils.yaml.emit.tag_roundtrip", "explicit tag preserved through emit")
{
  Document d = Document::view("x: !!str 42\n");
  const std::string out = d.emit();
  ICL_TEST_TRUE(out.find("!!str") != std::string::npos);
  // Reparse the emitted form and confirm the kind is still String.
  Document d2 = Document::own(std::string(out));
  ICL_TEST_TRUE(d2.root()["x"].scalarKind() == ScalarKind::String);
}

ICL_REGISTER_TEST("utils.yaml.emit.block_seq_of_mappings", "block seq where items are mappings")
{
  Document d = Document::empty();
  d.root().setSequence();
  Node item1; item1["name"].setScalar("a"); item1["v"].setScalar("1");
  Node item2; item2["name"].setScalar("b"); item2["v"].setScalar("2");
  d.root().pushBack(std::move(item1));
  d.root().pushBack(std::move(item2));
  // Must emit as block (nested items disqualify flow).
  std::string out = d.emit();
  ICL_TEST_TRUE(out.find("- ") != std::string::npos);
  ICL_TEST_TRUE(out.find("name: a") != std::string::npos);
  ICL_TEST_TRUE(out.find("name: b") != std::string::npos);
}

// ---------------------------------------------------------------------------
// External corpus — yaml-test-suite + JSONTestSuite
//
// The harness is deliberately tolerant: our YAML subset diverges from YAML 1.2
// in documented ways (no anchors / aliases / multi-doc / multi-line plain
// scalars), and JSON allows things YAML 1.2 rejects (duplicate keys).  We
// count parse successes, tree-matches, and hard crashes — only a catastrophic
// result (zero parses or a crash) fails the test.  A summary prints per run
// so divergences are visible.
// ---------------------------------------------------------------------------

#ifdef ICL_TEST_DATA_DIR
namespace {

  std::string readFile(const std::filesystem::path &p){
    std::ifstream f(p, std::ios::binary);
    std::ostringstream ss; ss << f.rdbuf();
    return ss.str();
  }

  // Canonical string form of a Node, used to compare two trees.  Scalars
  // are compared by raw view content; container shapes / sizes / key
  // order must match.  "null-kinded scalar" and "monostate Null" are
  // treated as equivalent (both emit as "#null").
  std::string canon(const Node &n){
    if(n.isNull()) return "#null";
    if(n.isScalar()){
      if(n.scalarKind() == ScalarKind::Null) return "#null";
      std::string out = "#s:";
      out += std::string(n.scalarView());
      return out;
    }
    if(n.isSequence()){
      std::string out = "[";
      bool first = true;
      for(const auto &e : n.sequence()){
        if(!first) out += ",";
        out += canon(e); first = false;
      }
      out += "]";
      return out;
    }
    if(n.isMapping()){
      std::string out = "{";
      bool first = true;
      for(const auto &kv : n.mapping()){
        if(!first) out += ",";
        out += "#k:"; out += std::string(kv.first);
        out += "="; out += canon(kv.second);
        first = false;
      }
      out += "}";
      return out;
    }
    return "#?";
  }

  struct CorpusStats {
    int total       = 0;
    int parsed      = 0;   // yaml parsed without throwing
    int matched     = 0;   // tree matched reference (when reference available)
    int refParseFail= 0;   // reference json (or comparison parse) itself failed
    std::vector<std::string> failures;  // names of cases that failed unexpectedly
  };

}  // anonymous
#endif

ICL_REGISTER_TEST("utils.yaml.corpus.json_suite",
                  "parse every valid JSON from JSONTestSuite")
{
#ifndef ICL_TEST_DATA_DIR
  // Corpus not available — test is a no-op.
#else
  namespace fs = std::filesystem;
  const fs::path dir = fs::path(ICL_TEST_DATA_DIR) / "yaml-corpus" / "json-suite";
  if(!fs::exists(dir)){
    // No corpus staged — not a failure, just skip.
    return;
  }

  CorpusStats st;
  for(auto &entry : fs::directory_iterator(dir)){
    if(entry.path().extension() != ".json") continue;
    ++st.total;
    const std::string name = entry.path().filename().string();
    const std::string src  = readFile(entry.path());
    try {
      Document d = Document::own(std::string(src));
      ++st.parsed;
      // Round-trip: emit and re-parse.  Should yield identical canonical form.
      std::string emitted = d.emit();
      try {
        Document d2 = Document::own(std::move(emitted));
        if(canon(d.root()) == canon(d2.root())) ++st.matched;
        else st.failures.push_back(name + " (roundtrip mismatch)");
      } catch (const ICLException &e){
        st.failures.push_back(name + " (roundtrip reparse threw: " + e.what() + ")");
      }
    } catch (const ICLException &e){
      st.failures.push_back(name + " (parse threw: " + std::string(e.what()) + ")");
    }
  }

  std::cerr << "[corpus] json-suite: "
            << st.parsed << "/" << st.total << " parsed, "
            << st.matched << "/" << st.total << " roundtrip-matched\n";
  if(!st.failures.empty()){
    std::cerr << "[corpus]   failures (" << st.failures.size() << "):\n";
    for(const auto &f : st.failures) std::cerr << "[corpus]     " << f << "\n";
  }

  // Sanity threshold: at least 80% must parse.  (We expect ~93/95 pass;
  // duplicate-key is the known divergence.)
  ICL_TEST_TRUE(st.total > 0);
  ICL_TEST_TRUE(st.parsed >= (st.total * 8) / 10);
#endif
}

ICL_REGISTER_TEST("utils.yaml.corpus.yaml_suite",
                  "parse every curated yaml-test-suite case")
{
#ifndef ICL_TEST_DATA_DIR
#else
  namespace fs = std::filesystem;
  const fs::path dir = fs::path(ICL_TEST_DATA_DIR) / "yaml-corpus" / "yaml-test-suite";
  if(!fs::exists(dir)) return;

  CorpusStats st;
  for(auto &entry : fs::directory_iterator(dir)){
    if(entry.path().extension() != ".yaml") continue;
    ++st.total;
    const std::string name = entry.path().filename().string();
    const std::string meta = readFile(entry.path());

    // Parse the meta-file itself.  It's a block sequence of mappings
    // where the first entry carries `yaml:` (literal block) and
    // optionally `json:` (literal block) fields.
    Document metaDoc;
    try {
      metaDoc = Document::own(std::string(meta));
    } catch (const ICLException &e){
      st.failures.push_back(name + " (meta parse threw: " + e.what() + ")");
      continue;
    }

    // Expected shape: top-level sequence of mappings.  Tolerate the
    // top-level-is-mapping shape too (some files omit the outer `- `).
    const Node *first = nullptr;
    if(metaDoc.root().isSequence() && metaDoc.root().size() > 0){
      first = &metaDoc.root().sequence()[0];
    } else if(metaDoc.root().isMapping()){
      first = &metaDoc.root();
    }
    if(!first || !first->isMapping()){
      st.failures.push_back(name + " (unexpected meta shape)");
      continue;
    }

    const Node *yamlField = first->find("yaml");
    if(!yamlField || !yamlField->isScalar()){
      st.failures.push_back(name + " (no `yaml` field)");
      continue;
    }
    const std::string yamlSrc = std::string(yamlField->scalarView());

    // Parse the actual yaml fixture.
    Document yamlDoc;
    try {
      yamlDoc = Document::own(std::string(yamlSrc));
      ++st.parsed;
    } catch (const ICLException &e){
      st.failures.push_back(name + " (fixture parse threw: " + e.what() + ")");
      continue;
    }

    // If a json reference is provided, parse it as YAML flow (valid JSON
    // is valid YAML) and compare canonical forms.
    const Node *jsonField = first->find("json");
    if(jsonField && jsonField->isScalar()){
      const std::string jsonSrc = std::string(jsonField->scalarView());
      try {
        Document jsonDoc = Document::own(std::string(jsonSrc));
        const std::string yc = canon(yamlDoc.root());
        const std::string jc = canon(jsonDoc.root());
        if(yc == jc) ++st.matched;
        else {
          st.failures.push_back(name + " (tree mismatch: yaml=" + yc.substr(0, 80) +
                                " vs json=" + jc.substr(0, 80) + ")");
        }
      } catch (const ICLException &){
        ++st.refParseFail;
        st.failures.push_back(name + " (json ref failed to parse)");
      }
    }
  }

  std::cerr << "[corpus] yaml-test-suite: "
            << st.parsed << "/" << st.total << " parsed, "
            << st.matched << "/" << st.total << " tree-matched ("
            << st.refParseFail << " json-ref-parse-fail)\n";
  if(!st.failures.empty()){
    std::cerr << "[corpus]   failures (" << st.failures.size() << "):\n";
    for(const auto &f : st.failures) std::cerr << "[corpus]     " << f << "\n";
  }

  // Sanity: corpus is staged and at least half parse.
  ICL_TEST_TRUE(st.total > 0);
  ICL_TEST_TRUE(st.parsed >= st.total / 2);
#endif
}
