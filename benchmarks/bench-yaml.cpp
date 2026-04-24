// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "harness/Benchmark.h"
#include <icl/utils/Yaml.h>
#include <icl/utils/Size.h>

#include <string>

// Reference numbers (vs yaml-cpp 0.9.0 and rapidyaml 0.11.1, release
// -O3 on Apple-Silicon arm64) are recorded in icl/utils/Yaml.h.  The
// competitor probes were removed from meson.build to keep the build
// dependency-free; re-add them temporarily when re-measuring.

using namespace icl::utils;

namespace {

  // ~500-byte representative config: nested mappings, block sequences,
  // flow sequences, mixed scalar types (string / int / float / bool),
  // comments, 3-level deep.  Kept realistic — the parse cost for a
  // file this size is what a ConfigFile user actually pays per load.
  static const std::string kSmallYaml = R"YAML(server:
  host: "localhost"
  port: 8080
  timeout: 30.5
  debug: true
  workers:
    - name: "w0"
      cpu: 0
      memory_mb: 512
    - name: "w1"
      cpu: 1
      memory_mb: 512
    - name: "w2"
      cpu: 2
      memory_mb: 1024
cameras:
  - id: cam0
    resolution: [640, 480]
    fps: 30
    format: rgb8
  - id: cam1
    resolution: [1920, 1080]
    fps: 60
    format: rgb8
paths:
  data:   /var/data/logs
  config: /etc/app.yaml
  cache:  /tmp/app.cache
logging:
  level: info
  targets:
    - stdout
    - /var/log/app.log
)YAML";

  // Build a ~large input by repeating the small fixture under synthetic
  // section keys — exercises throughput rather than per-call overhead.
  static std::string makeLargeYaml(int repeats){
    std::string out;
    out.reserve(repeats * (kSmallYaml.size() + 32));
    for(int i = 0; i < repeats; ++i){
      out += "section_";
      out += std::to_string(i);
      out += ":\n";
      // Indent every line of kSmallYaml by 2 spaces.
      std::size_t start = 0;
      while(start < kSmallYaml.size()){
        std::size_t nl = kSmallYaml.find('\n', start);
        if(nl == std::string::npos) nl = kSmallYaml.size();
        out += "  ";
        out.append(kSmallYaml, start, nl - start);
        out.push_back('\n');
        start = nl + 1;
      }
    }
    return out;
  }

  // ---------------------------------------------------------------------
  // icl::utils::yaml
  // ---------------------------------------------------------------------

  static BenchmarkRegistrar bench_icl_parse_small({"utils.yaml.parse_small",
    "parse ~500 B config",
    {},
    [](const BenchParams &){
      auto d = yaml::Document::view(std::string_view(kSmallYaml));
      (void)d.root();
    }
  });

  static BenchmarkRegistrar bench_icl_parse_large({"utils.yaml.parse_large",
    "parse ~50 KB config (100x repetition)",
    {BenchParamDef::Int("repeats", 100, 10, 1000)},
    [](const BenchParams &p){
      static std::string src = makeLargeYaml(p.getInt("repeats"));
      auto d = yaml::Document::view(std::string_view(src));
      (void)d.root();
    }
  });

  static BenchmarkRegistrar bench_icl_emit({"utils.yaml.emit",
    "emit a built tree back to YAML text",
    {},
    [](const BenchParams &){
      // Pre-parse once; benchmark only the emit path.
      static auto d = yaml::Document::own(kSmallYaml);
      volatile auto s = d.emit();
      (void)s;
    }
  });

  static BenchmarkRegistrar bench_icl_build({"utils.yaml.build",
    "programmatic build of a ~20-key config tree",
    {},
    [](const BenchParams &){
      auto d = yaml::Document::empty();
      d.root() = yaml::map{
        {"host",    "localhost"},
        {"port",    8080},
        {"timeout", 30.5},
        {"debug",   true},
        {"paths",   yaml::map{
          {"data",   "/var/data/logs"},
          {"config", "/etc/app.yaml"},
          {"cache",  "/tmp/app.cache"},
        }},
        {"ports",   yaml::seq{80, 443, 8080, 8443}},
        {"res",     Size(1920, 1080)},
      };
      volatile auto s = d.emit();
      (void)s;
    }
  });

  static BenchmarkRegistrar bench_icl_traverse({"utils.yaml.traverse",
    "parse + deep lookup + typed conversions",
    {},
    [](const BenchParams &){
      auto d = yaml::Document::view(std::string_view(kSmallYaml));
      const auto &r = d.root();
      volatile int port    = r["server"]["port"].as<int>();
      volatile double to   = r["server"]["timeout"].as<double>();
      volatile int cpu     = r["server"]["workers"][1]["cpu"].as<int>();
      volatile int fps     = r["cameras"][0]["fps"].as<int>();
      (void)port; (void)to; (void)cpu; (void)fps;
    }
  });

}  // anonymous namespace
