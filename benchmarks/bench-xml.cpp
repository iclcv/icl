// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Benchmark.h"
#include <icl/utils/Xml.h>
#include <icl/utils/detail/pugi/PugiXML.h>

#include <string>

// Reference numbers (vs vendored pugixml, release -O3 on Apple-Silicon
// arm64) are pinned in icl/utils/Xml.h.  Pugi stays vendored for
// this benchmark only — the follow-up commit retires the vendored
// parser and strips the `utils.xml.pugi.*` entries below.

using namespace icl::utils;

namespace {

  // ~500-byte representative config shaped like Primitive3DFilter's
  // pointcloud-filter XML: nested elements, attribute-heavy, a bit
  // of whitespace / indentation.
  static const std::string kSmallXml = R"XML(<?xml version="1.0" encoding="UTF-8"?>
<pointcloudfilter>
  <primitivegroup id="table"     regex=".*table.*"   padding="0.02"/>
  <primitivegroup id="cup"       regex=".*cup.*"     padding="0.01"/>
  <primitivegroup id="hand"      regex=".*hand.*"    padding="0.05"/>
  <remove>
    <group part="inner" id="table"/>
  </remove>
  <setpos x="0.0" y="0.0" z="0.0">
    <group part="outer" id="hand"/>
  </setpos>
  <color r="0.8" g="0.2" b="0.1" a="1.0">
    <intersection>
      <group part="inner" id="cup"/>
      <group part="outer" id="hand"/>
    </intersection>
  </color>
  <label value="42">
    <group part="inner" id="cup"/>
  </label>
  <intensity value="0.75">
    <group part="inner" id="cup"/>
  </intensity>
</pointcloudfilter>
)XML";

  // Scale up by repetition under a synthetic wrapper root, similar to
  // bench-yaml.  Each repetition is a full copy of kSmallXml's body
  // wrapped as a nested element, so the result is ~n*kSmallXml bytes.
  static std::string makeLargeXml(int repeats){
    std::string out;
    out.reserve(repeats * (kSmallXml.size() + 32));
    out += "<?xml version=\"1.0\"?>\n<root>\n";
    for(int i = 0; i < repeats; ++i){
      out += "  <section idx=\"";
      out += std::to_string(i);
      out += "\">\n";
      // Strip the prologue line from kSmallXml and indent the rest.
      std::size_t bodyStart = kSmallXml.find('\n') + 1;  // skip <?xml ...?>\n
      std::size_t start = bodyStart;
      while(start < kSmallXml.size()){
        std::size_t nl = kSmallXml.find('\n', start);
        if(nl == std::string::npos) nl = kSmallXml.size();
        out += "    ";
        out.append(kSmallXml, start, nl - start);
        out.push_back('\n');
        start = nl + 1;
      }
      out += "  </section>\n";
    }
    out += "</root>\n";
    return out;
  }

  // ---------------------------------------------------------------------
  // icl::utils::xml
  // ---------------------------------------------------------------------

  static BenchmarkRegistrar bench_icl_parse_small({"utils.xml.parse_small",
    "parse ~500 B config (icl::utils::xml)",
    {},
    [](const BenchParams &){
      auto d = xml::Document::parse(std::string_view(kSmallXml));
      (void)d.root();
    }
  });

  static BenchmarkRegistrar bench_icl_parse_large({"utils.xml.parse_large",
    "parse ~50 KB config (100x repetition) — icl::utils::xml",
    {BenchParamDef::Int("repeats", 100, 10, 1000)},
    [](const BenchParams &p){
      static std::string src = makeLargeXml(p.getInt("repeats"));
      auto d = xml::Document::parse(std::string_view(src));
      (void)d.root();
    }
  });

  static BenchmarkRegistrar bench_icl_traverse({"utils.xml.traverse",
    "parse + named child walk + attribute-typed reads — icl::utils::xml",
    {},
    [](const BenchParams &){
      auto d = xml::Document::parse(std::string_view(kSmallXml));
      volatile int count = 0;
      for(auto pg : d.root().children("primitivegroup")){
        volatile float pad = pg.attribute("padding").asFloat();
        (void)pad;
        ++count;
      }
      volatile int lbl = d.root().child("label").attribute("value").asInt();
      (void)lbl; (void)count;
    }
  });

  static BenchmarkRegistrar bench_icl_xpath({"utils.xml.xpath",
    "XPath predicate-union dispatch (Primitive3DFilter-shaped)",
    {},
    [](const BenchParams &){
      static auto d = xml::Document::parseOwned(kSmallXml);
      auto ns = d.root().selectAll(
          "/pointcloudfilter/*[self::remove or self::setpos or self::color "
          "or self::label or self::intensity or self::filterdepthimg]");
      volatile std::size_t n = ns.size();
      (void)n;
    }
  });

  static BenchmarkRegistrar bench_icl_emit({"utils.xml.emit",
    "parse + re-emit small config — icl::utils::xml",
    {},
    [](const BenchParams &){
      static auto d = xml::Document::parseOwned(kSmallXml);
      volatile auto s = d.emit();
      (void)s;
    }
  });

  // ---------------------------------------------------------------------
  // pugi — vendored reference, same inputs, same measurement shape.
  // These probes are deleted together with the vendored parser in the
  // follow-up commit.
  // ---------------------------------------------------------------------

  static BenchmarkRegistrar bench_pugi_parse_small({"utils.xml.pugi.parse_small",
    "parse ~500 B config (pugixml, reference)",
    {},
    [](const BenchParams &){
      pugi::xml_document doc;
      doc.load_buffer(kSmallXml.data(), kSmallXml.size());
      (void)doc.first_child();
    }
  });

  static BenchmarkRegistrar bench_pugi_parse_large({"utils.xml.pugi.parse_large",
    "parse ~50 KB config (100x repetition) — pugixml",
    {BenchParamDef::Int("repeats", 100, 10, 1000)},
    [](const BenchParams &p){
      static std::string src = makeLargeXml(p.getInt("repeats"));
      pugi::xml_document doc;
      doc.load_buffer(src.data(), src.size());
      (void)doc.first_child();
    }
  });

  static BenchmarkRegistrar bench_pugi_traverse({"utils.xml.pugi.traverse",
    "parse + named child walk + attribute-typed reads — pugixml",
    {},
    [](const BenchParams &){
      pugi::xml_document doc;
      doc.load_buffer(kSmallXml.data(), kSmallXml.size());
      auto root = doc.child("pointcloudfilter");
      volatile int count = 0;
      for(auto pg = root.child("primitivegroup"); pg; pg = pg.next_sibling("primitivegroup")){
        volatile float pad = pg.attribute("padding").as_float();
        (void)pad;
        ++count;
      }
      volatile int lbl = root.child("label").attribute("value").as_int();
      (void)lbl; (void)count;
    }
  });

  static BenchmarkRegistrar bench_pugi_xpath({"utils.xml.pugi.xpath",
    "XPath predicate-union dispatch — pugixml",
    {},
    [](const BenchParams &){
      static pugi::xml_document doc = [](){
        pugi::xml_document d;
        d.load_buffer(kSmallXml.data(), kSmallXml.size());
        return d;
      }();
      auto ns = doc.select_nodes(
          "/pointcloudfilter/*[self::remove or self::setpos or self::color "
          "or self::label or self::intensity or self::filterdepthimg]");
      volatile std::size_t n = ns.size();
      (void)n;
    }
  });

}  // anonymous namespace
