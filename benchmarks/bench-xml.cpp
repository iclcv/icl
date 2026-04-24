// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Benchmark.h"
#include <icl/utils/Xml.h>

#include <string>

// Reference numbers are pinned in icl/utils/Xml.h (vs the formerly
// vendored pugixml, release -O3 on Apple-Silicon arm64).  The pugi
// probes that produced those numbers were removed together with the
// vendored parser.  To re-measure against a modern pugixml,
// temporarily vendor it under a local 3rdparty path and reinstate
// the `utils.xml.pugi.*` entries that lived here in the prior commit.

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

}  // anonymous namespace
