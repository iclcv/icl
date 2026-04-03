// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Benchmark.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <iostream>
#include <string>

using namespace icl::utils;

int main(int argc, char **argv){
  pa_init(argc, argv,
    "-list|-l "
    "-filter|-f(pattern=*) "
    "-param|-p(...params) "
    "-iterations|-n(int=50) "
    "-warmup|-w(double=2.0) "
    "-csv "
    "-verbose|-v ");

  auto &registry = BenchmarkRegistry::instance();

  // List mode
  if(pa("-l")){
    std::cout << "Registered benchmarks:\n\n";
    for(auto &e : registry.entries()){
      std::cout << "  " << e.key << "\n    " << e.description << "\n";
      if(!e.params.empty()){
        std::cout << "    Parameters:\n";
        for(auto &p : e.params){
          std::cout << "      " << p.name << " = ";
          std::visit([](auto &&v){ std::cout << v; }, p.defaultVal);
          if(auto *iv = std::get_if<int>(&p.minVal)){
            if(auto *ix = std::get_if<int>(&p.maxVal)){
              if(*iv != 0 || *ix != 0) std::cout << "  [" << *iv << " .. " << *ix << "]";
            }
          }
          if(auto *dv = std::get_if<double>(&p.minVal)){
            if(auto *dx = std::get_if<double>(&p.maxVal)){
              if(*dv != 0 || *dx != 0) std::cout << "  [" << *dv << " .. " << *dx << "]";
            }
          }
          std::cout << "\n";
        }
      }
      std::cout << "\n";
    }
    if(registry.entries().empty()){
      std::cout << "  (none registered — link against modules that contain benchmarks)\n";
    }
    return 0;
  }

  // Filter
  std::string pattern = pa("-f");
  auto matches = registry.filter(pattern);

  if(matches.empty()){
    std::cerr << "No benchmarks matching '" << pattern << "'\n";
    std::cerr << "Use --list to see available benchmarks.\n";
    return 1;
  }

  // Parse parameter overrides
  std::map<std::string, std::string> overrides;
  if(pa("-p")){
    for(unsigned i = 0; i < pa("-p").n(); ++i){
      std::string arg = pa("-p", i);
      auto eq = arg.find('=');
      if(eq != std::string::npos){
        overrides[arg.substr(0, eq)] = arg.substr(eq+1);
      }
    }
  }

  int iterations = pa("-n");
  double warmup = pa("-w");
  bool csv = pa("-csv");
  bool verbose = pa("-v");

  if(verbose && !csv){
    std::cout << "Running " << matches.size() << " benchmark(s)"
              << " (" << iterations << " iterations, " << warmup << "s warmup)\n\n";
  }

  BenchmarkRunner runner(iterations, warmup);

  if(csv){
    BenchmarkRunner::printHeader(true);
  }else{
    BenchmarkRunner::printHeader(false);
  }

  for(auto *entry : matches){
    // Build params: start with defaults, apply overrides
    BenchParams params;
    for(auto &pd : entry->params){
      params.set(pd.name, pd.defaultVal);
      // Check for override
      auto it = overrides.find(pd.name);
      if(it != overrides.end()){
        if(std::holds_alternative<int>(pd.defaultVal)){
          params.set(pd.name, std::stoi(it->second));
        }else if(std::holds_alternative<double>(pd.defaultVal)){
          params.set(pd.name, std::stod(it->second));
        }else{
          params.set(pd.name, it->second);
        }
      }
    }

    auto result = runner.run(*entry, params);
    BenchmarkRunner::printResult(result, csv);
  }

  if(!csv){
    std::cout << std::string(120, '-') << "\n";
  }

  return 0;
}
