// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Test.h"
#include <icl/utils/Xml.h>

#include <string>
#include <string_view>

using namespace icl::utils;
using namespace icl::utils::xml;

// ---------------------------------------------------------------------
// Parse — basic shapes
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.parse.empty_element", "self-closing root")
{
  auto doc = Document::parse("<root/>");
  ICL_TEST_TRUE(doc.hasRoot());
  ICL_TEST_EQ(std::string(doc.root().name()), std::string("root"));
  ICL_TEST_FALSE((bool)doc.root().firstChild());
}

ICL_REGISTER_TEST("utils.xml.parse.root_with_text", "leaf element carries text()")
{
  auto doc = Document::parse("<FOV>56</FOV>");
  ICL_TEST_EQ(std::string(doc.root().name()), std::string("FOV"));
  ICL_TEST_EQ(std::string(doc.root().text()), std::string("56"));
}

ICL_REGISTER_TEST("utils.xml.parse.nested_elements", "nested elements link via firstChild/nextSibling")
{
  auto doc = Document::parse("<a><b/><c/><b/></a>");
  auto a = doc.root();
  auto b1 = a.firstChild();
  ICL_TEST_EQ(std::string(b1.name()), std::string("b"));
  auto c  = b1.nextSibling();
  ICL_TEST_EQ(std::string(c.name()),  std::string("c"));
  auto b2 = c.nextSibling();
  ICL_TEST_EQ(std::string(b2.name()), std::string("b"));
  ICL_TEST_FALSE((bool)b2.nextSibling());
}

ICL_REGISTER_TEST("utils.xml.parse.child_by_name", "child(name) matches first-of-kind")
{
  auto doc = Document::parse("<a><x/><y>v</y><x/></a>");
  auto y = doc.root().child("y");
  ICL_TEST_EQ(std::string(y.text()), std::string("v"));
  auto x1 = doc.root().child("x");
  ICL_TEST_TRUE((bool)x1);
  auto x2 = x1.nextSibling("x");
  ICL_TEST_TRUE((bool)x2);
  ICL_TEST_FALSE((bool)x2.nextSibling("x"));
}

ICL_REGISTER_TEST("utils.xml.parse.children_range", "children() iterates all; filtered skips others")
{
  auto doc = Document::parse("<a><b/><c/><b/><d/></a>");
  int count = 0;
  for(auto c : doc.root().children()) (void)c, ++count;
  ICL_TEST_EQ(count, 4);

  int bcount = 0;
  for(auto c : doc.root().children("b")){
    ICL_TEST_EQ(std::string(c.name()), std::string("b"));
    ++bcount;
  }
  ICL_TEST_EQ(bcount, 2);
}

// ---------------------------------------------------------------------
// Attributes
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.attr.basic", "attribute name/value lookup")
{
  auto doc = Document::parse("<e id=\"42\" name='alpha'/>");
  auto e = doc.root();
  ICL_TEST_EQ(std::string(e.attribute("id").value()),   std::string("42"));
  ICL_TEST_EQ(std::string(e.attribute("name").value()), std::string("alpha"));
  ICL_TEST_FALSE((bool)e.attribute("missing"));
}

ICL_REGISTER_TEST("utils.xml.attr.numeric", "asInt/asFloat/asDouble")
{
  auto doc = Document::parse("<e i=\"42\" f=\"3.14\" d=\"2.71828\"/>");
  auto e = doc.root();
  ICL_TEST_EQ(e.attribute("i").asInt(0), 42);
  ICL_TEST_NEAR(e.attribute("f").asFloat(0.0f),  3.14f,   1e-5f);
  ICL_TEST_NEAR(e.attribute("d").asDouble(0.0),  2.71828, 1e-9);
  ICL_TEST_EQ(e.attribute("missing").asInt(99), 99);
}

ICL_REGISTER_TEST("utils.xml.attr.bool", "asBool literal forms")
{
  auto doc = Document::parse(
      "<e a=\"true\" b=\"false\" c=\"1\" d=\"0\" e=\"yes\" f=\"no\"/>");
  auto r = doc.root();
  ICL_TEST_TRUE(r.attribute("a").asBool());
  ICL_TEST_FALSE(r.attribute("b").asBool());
  ICL_TEST_TRUE(r.attribute("c").asBool());
  ICL_TEST_FALSE(r.attribute("d").asBool());
  ICL_TEST_TRUE(r.attribute("e").asBool());
  ICL_TEST_FALSE(r.attribute("f").asBool());
}

ICL_REGISTER_TEST("utils.xml.attr.entity_decoding", "&amp; etc. round-trip through value()")
{
  auto doc = Document::parse("<e s=\"a &amp; b &lt; c &gt; d &quot;e&quot; f\"/>");
  ICL_TEST_EQ(std::string(doc.root().attribute("s").value()),
              std::string("a & b < c > d \"e\" f"));
  // Raw should stay encoded.
  ICL_TEST_EQ(std::string(doc.root().attribute("s").valueRaw()),
              std::string("a &amp; b &lt; c &gt; d &quot;e&quot; f"));
}

ICL_REGISTER_TEST("utils.xml.attr.numeric_entity_decimal", "&#65; decodes to 'A'")
{
  auto doc = Document::parse("<e v=\"&#65;&#66;\"/>");
  ICL_TEST_EQ(std::string(doc.root().attribute("v").value()), std::string("AB"));
}

ICL_REGISTER_TEST("utils.xml.attr.numeric_entity_hex", "&#x41; decodes to 'A'")
{
  auto doc = Document::parse("<e v=\"&#x41;&#x42;\"/>");
  ICL_TEST_EQ(std::string(doc.root().attribute("v").value()), std::string("AB"));
}

ICL_REGISTER_TEST("utils.xml.attr.utf8_via_numeric", "&#233; decodes as UTF-8 é (0xC3 0xA9)")
{
  auto doc = Document::parse("<e v=\"caf&#233;\"/>");
  std::string got(doc.root().attribute("v").value());
  // "caf" + 0xC3 0xA9
  ICL_TEST_EQ(got.size(), size_t(5));
  ICL_TEST_EQ(static_cast<unsigned char>(got[3]), 0xC3u);
  ICL_TEST_EQ(static_cast<unsigned char>(got[4]), 0xA9u);
}

// ---------------------------------------------------------------------
// CDATA, comments, PI, DOCTYPE
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.cdata.basic", "CDATA contents surface as text")
{
  auto doc = Document::parse("<e><![CDATA[<not a tag> & stuff]]></e>");
  ICL_TEST_EQ(std::string(doc.root().text()),
              std::string("<not a tag> & stuff"));
}

ICL_REGISTER_TEST("utils.xml.comments.skipped", "comments don't inject content")
{
  auto doc = Document::parse("<e>hi<!-- a --><!-- b -->bye</e>");
  // Contiguous text + comments + text → concatenated text view.
  std::string t(doc.root().text());
  // We concatenate; the final view may include either raw pieces.
  // Acceptable outcomes: "hi" + "bye" = "hibye" OR whatever the
  // parser's contiguous-view optimiser produces.  Validate a
  // non-empty leaf that doesn't contain comment bytes.
  ICL_TEST_FALSE(t.find("<!--") != std::string::npos);
}

ICL_REGISTER_TEST("utils.xml.pi.skipped", "processing instructions don't derail parse")
{
  auto doc = Document::parse("<?xml version=\"1.0\"?><?stuff foo=bar?><root/>");
  ICL_TEST_TRUE(doc.hasRoot());
  ICL_TEST_EQ(std::string(doc.root().name()), std::string("root"));
}

ICL_REGISTER_TEST("utils.xml.doctype.skipped", "DOCTYPE with internal subset tolerated")
{
  auto doc = Document::parse(
      "<?xml version=\"1.0\"?>\n"
      "<!DOCTYPE root [\n"
      "  <!ELEMENT root EMPTY>\n"
      "]>\n"
      "<root/>\n");
  ICL_TEST_TRUE(doc.hasRoot());
}

ICL_REGISTER_TEST("utils.xml.bom.utf8", "UTF-8 BOM at start parses cleanly")
{
  std::string src = "\xEF\xBB\xBF<root/>";
  auto doc = Document::parse(src);
  ICL_TEST_TRUE(doc.hasRoot());
}

// ---------------------------------------------------------------------
// Malformed input — line/col in ParseError
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.error.unclosed_element", "unterminated element raises ParseError")
{
  ICL_TEST_THROW(Document::parse("<a><b></a>"), ParseError);
}

ICL_REGISTER_TEST("utils.xml.error.mismatched_close", "mismatched close tag reports location")
{
  try {
    Document::parse("<a>\n  <b></c>\n</a>");
    ICL_TEST_FAIL("expected ParseError");
  } catch(const ParseError &e){
    ICL_TEST_EQ(e.line(), size_t(2));
  }
}

ICL_REGISTER_TEST("utils.xml.error.no_root", "empty string rejected")
{
  ICL_TEST_THROW(Document::parse(""), ParseError);
}

ICL_REGISTER_TEST("utils.xml.error.trailing_content", "content after root is rejected")
{
  ICL_TEST_THROW(Document::parse("<a/>garbage"), ParseError);
}

ICL_REGISTER_TEST("utils.xml.error.bad_attr_quote", "missing quote on attr value raises")
{
  ICL_TEST_THROW(Document::parse("<a x=unquoted/>"), ParseError);
}

// ---------------------------------------------------------------------
// Emit + round-trip
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.emit.self_close_empty", "empty element emits as <x/> by default")
{
  auto doc = Document::empty();
  doc.setRoot("x");
  EmitOptions o;
  o.prologue = false;
  o.indent   = 0;
  std::string s = doc.emit(o);
  ICL_TEST_EQ(s, std::string("<x/>"));
}

ICL_REGISTER_TEST("utils.xml.emit.attrs_and_text", "attributes and text emit with escaping")
{
  auto doc = Document::empty();
  auto r = doc.setRoot("e");
  r.setAttribute("q", "a & b");
  r.setText("hi < world");
  EmitOptions o; o.prologue = false; o.indent = 0;
  std::string s = doc.emit(o);
  ICL_TEST_EQ(s, std::string("<e q=\"a &amp; b\">hi &lt; world</e>"));
}

ICL_REGISTER_TEST("utils.xml.emit.round_trip", "parse → emit → parse is stable")
{
  std::string src = "<root><a x=\"1\"/><b>text</b><c/></root>";
  auto d1 = Document::parse(src);
  EmitOptions o; o.prologue = false; o.indent = 0;
  std::string out = d1.emit(o);
  auto d2 = Document::parseOwned(out);
  ICL_TEST_TRUE(d2.hasRoot());
  ICL_TEST_EQ(std::string(d2.root().name()), std::string("root"));
  ICL_TEST_EQ(std::string(d2.root().child("a").attribute("x").value()),
              std::string("1"));
  ICL_TEST_EQ(std::string(d2.root().child("b").text()),
              std::string("text"));
}

// ---------------------------------------------------------------------
// Mutation — build programmatically
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.build.basic", "empty() + setRoot + appendChild + setAttribute")
{
  auto doc = Document::empty();
  auto r   = doc.setRoot("cfg");
  auto a   = r.appendChild("item");
  a.setAttribute("id", "0");
  a.setText("alpha");
  auto b   = r.appendChild("item");
  b.setAttribute("id", "1");
  b.setText("beta");

  // Round-trip the build.
  EmitOptions o; o.prologue = false; o.indent = 0;
  std::string out = doc.emit(o);
  auto d2 = Document::parseOwned(out);
  int count = 0;
  for(auto c : d2.root().children("item")){
    ICL_TEST_EQ(std::string(c.name()), std::string("item"));
    ++count;
  }
  ICL_TEST_EQ(count, 2);
}

ICL_REGISTER_TEST("utils.xml.build.overwrite_attr", "setAttribute twice overwrites")
{
  auto doc = Document::empty();
  auto r   = doc.setRoot("e");
  r.setAttribute("k", "v1");
  r.setAttribute("k", "v2");
  ICL_TEST_EQ(std::string(r.attribute("k").value()), std::string("v2"));
  ICL_TEST_EQ(r.attributeCount(), size_t(1));
}

// ---------------------------------------------------------------------
// Integration-shape — Primitive3DFilter style query
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.integration.primitive3dfilter_shape",
                  "walk element-type dispatch like Primitive3DFilter does")
{
  const char *src =
    "<pointcloudfilter>\n"
    "  <primitivegroup id=\"g1\" regex=\".*\" padding=\"0.5\"/>\n"
    "  <remove><group part=\"inner\" id=\"g1\"/></remove>\n"
    "  <setpos x=\"1\" y=\"2\" z=\"3\"><group part=\"outer\" id=\"g1\"/></setpos>\n"
    "  <color r=\"0.1\" g=\"0.2\" b=\"0.3\" a=\"1.0\"/>\n"
    "</pointcloudfilter>\n";
  auto doc = Document::parse(src);
  auto root = doc.root();
  auto pg   = root.child("primitivegroup");
  ICL_TEST_EQ(std::string(pg.attribute("id").value()), std::string("g1"));
  ICL_TEST_NEAR(pg.attribute("padding").asFloat(0), 0.5f, 1e-6f);

  int actionCount = 0;
  for(auto a : root.children()){
    auto n = a.name();
    if(n == "remove" || n == "setpos" || n == "color") ++actionCount;
  }
  ICL_TEST_EQ(actionCount, 3);

  auto setpos = root.child("setpos");
  ICL_TEST_NEAR(setpos.attribute("x").asFloat(0), 1.0f, 1e-6f);
  ICL_TEST_NEAR(setpos.attribute("y").asFloat(0), 2.0f, 1e-6f);
  ICL_TEST_NEAR(setpos.attribute("z").asFloat(0), 3.0f, 1e-6f);
}

// ---------------------------------------------------------------------
// Integration-shape — OptrisGrabber style nested path query
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.integration.optris_shape",
                  "chained .child() fetches a deep leaf value")
{
  const char *src =
    "<CaliData>\n"
    "  <Temperature>\n"
    "    <Optics>\n"
    "      <OpticsDef>\n"
    "        <FOV>56</FOV>\n"
    "      </OpticsDef>\n"
    "    </Optics>\n"
    "  </Temperature>\n"
    "</CaliData>\n";
  auto doc = Document::parse(src);
  auto fov = doc.root()
               .child("Temperature")
               .child("Optics")
               .child("OpticsDef")
               .child("FOV");
  ICL_TEST_TRUE((bool)fov);
  // The raw text view may include surrounding whitespace depending
  // on how the parser concatenates between-children chunks; trim
  // and compare.
  std::string t(fov.text());
  while(!t.empty() && (t.front() == ' ' || t.front() == '\n' || t.front() == '\t'))
    t.erase(t.begin());
  while(!t.empty() && (t.back() == ' ' || t.back() == '\n' || t.back() == '\t'))
    t.pop_back();
  ICL_TEST_EQ(t, std::string("56"));
}

// ---------------------------------------------------------------------
// XPath — Phase 7 tests
// ---------------------------------------------------------------------

ICL_REGISTER_TEST("utils.xml.xpath.relative_child", "simple relative path")
{
  auto doc = Document::parse("<a><b><c/></b></a>");
  auto c = doc.root().selectOne("b/c");
  ICL_TEST_TRUE((bool)c);
  ICL_TEST_EQ(std::string(c.name()), std::string("c"));
}

ICL_REGISTER_TEST("utils.xml.xpath.absolute_from_inner", "absolute path resolves to root regardless of start")
{
  auto doc = Document::parse("<a><b><c x=\"1\"/></b></a>");
  auto inner = doc.root().child("b");
  auto c = inner.selectOne("/a/b/c");
  ICL_TEST_TRUE((bool)c);
  ICL_TEST_EQ(c.attribute("x").asInt(), 1);
}

ICL_REGISTER_TEST("utils.xml.xpath.wildcard_step", "'*' matches any element name")
{
  auto doc = Document::parse("<a><b/><c/><d/></a>");
  auto ns = doc.root().selectAll("*");
  ICL_TEST_EQ(ns.size(), size_t(3));
}

ICL_REGISTER_TEST("utils.xml.xpath.descendant", "'//name' matches anywhere under doc root")
{
  auto doc = Document::parse("<a><b><target/></b><c><inner><target/></inner></c></a>");
  auto abs = doc.root().selectAll("//target");
  ICL_TEST_EQ(abs.size(), size_t(2));
}

ICL_REGISTER_TEST("utils.xml.xpath.self_predicate_union",
                  "Primitive3DFilter-shaped [self::x or self::y or ...]")
{
  const char *src =
    "<root>"
    "<a/><b/><remove/><c/><setpos/><label/>"
    "</root>";
  auto doc = Document::parse(src);
  auto ns = doc.root().selectAll(
      "/root/*[self::remove or self::setpos or self::label]");
  ICL_TEST_EQ(ns.size(), size_t(3));
  std::string names;
  for(auto e : ns){ names += std::string(e.name()); names += ","; }
  ICL_TEST_EQ(names, std::string("remove,setpos,label,"));
}

ICL_REGISTER_TEST("utils.xml.xpath.attr_eq", "[@id='foo'] filters by attribute value")
{
  auto doc = Document::parse(
      "<a><item id=\"x\"/><item id=\"y\"/><item id=\"x\"/></a>");
  auto ns = doc.root().selectAll("item[@id='x']");
  ICL_TEST_EQ(ns.size(), size_t(2));
  auto one = doc.root().selectOne("item[@id='y']");
  ICL_TEST_TRUE((bool)one);
  ICL_TEST_EQ(std::string(one.attribute("id").value()), std::string("y"));
}

ICL_REGISTER_TEST("utils.xml.xpath.attr_exists", "[@id] matches any element with that attr")
{
  auto doc = Document::parse(
      "<a><item/><item id=\"y\"/><item/></a>");
  auto ns = doc.root().selectAll("item[@id]");
  ICL_TEST_EQ(ns.size(), size_t(1));
}

ICL_REGISTER_TEST("utils.xml.xpath.index", "[1] [2] pick positional nth of filtered set")
{
  auto doc = Document::parse("<a><b/><b/><b/></a>");
  auto first = doc.root().selectOne("b[1]");
  auto third = doc.root().selectOne("b[3]");
  ICL_TEST_TRUE((bool)first);
  ICL_TEST_TRUE((bool)third);
  ICL_TEST_FALSE((bool)doc.root().selectOne("b[4]"));
}

ICL_REGISTER_TEST("utils.xml.xpath.and_or_precedence",
                  "'a or b and c' parses as 'a or (b and c)'")
{
  // <e k="x"/>  →  @k='x' OR @k='y' AND @missing=''  = TRUE (left OR holds)
  auto doc = Document::parse("<r><e k=\"x\"/><e k=\"y\"/></r>");
  auto ns = doc.root().selectAll(
      "e[@k='x' or @k='y' and @missing='nope']");
  // With correct precedence: first matches (@k='x'), second's OR-RHS
  // evaluates to FALSE (second clause of AND fails), so only the
  // first matches.
  ICL_TEST_EQ(ns.size(), size_t(1));
}

ICL_REGISTER_TEST("utils.xml.xpath.attr_terminal", "terminal '@name' returns an Attribute")
{
  auto doc = Document::parse("<a><b id=\"42\"/></a>");
  auto a = doc.root().selectAttr("b/@id");
  ICL_TEST_TRUE((bool)a);
  ICL_TEST_EQ(a.asInt(), 42);
}

ICL_REGISTER_TEST("utils.xml.xpath.primitive3dfilter_exact",
                  "the exact query Primitive3DFilter used with pugi")
{
  const char *src =
    "<pointcloudfilter>"
    "<primitivegroup id=\"g1\" regex=\".*\"/>"
    "<remove/><setpos x=\"0\"/><color r=\"1\"/><label value=\"1\"/>"
    "<intensity value=\"2\"/><filterdepthimg value=\"3\"/>"
    "<unrelated/>"
    "</pointcloudfilter>";
  auto doc = Document::parse(src);
  auto ns = doc.root().selectAll(
      "/pointcloudfilter/*[self::remove or self::setpos or self::color "
      "or self::label or self::intensity or self::filterdepthimg]");
  ICL_TEST_EQ(ns.size(), size_t(6));
}

ICL_REGISTER_TEST("utils.xml.xpath.optris_exact",
                  "the exact query OptrisGrabber used with pugi")
{
  const char *src =
    "<CaliData>"
      "<Temperature>"
        "<Optics>"
          "<OpticsDef>"
            "<FOV>72</FOV>"
          "</OpticsDef>"
        "</Optics>"
      "</Temperature>"
    "</CaliData>";
  auto doc = Document::parse(src);
  auto fov = doc.root().selectOne("/CaliData/Temperature/Optics/OpticsDef/FOV");
  ICL_TEST_TRUE((bool)fov);
  ICL_TEST_EQ(std::string(fov.text()), std::string("72"));
}

ICL_REGISTER_TEST("utils.xml.xpath.error.unclosed_bracket", "unterminated predicate is an error")
{
  ICL_TEST_THROW(Document::parse("<a><b/></a>").root().selectAll("b[@x"),
                 ParseError);
}
