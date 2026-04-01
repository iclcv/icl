// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Time.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <variant>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <iomanip>

namespace icl{
  namespace utils{

    /// Single benchmark parameter value (int, double, or string)
    using BenchParamValue = std::variant<int, double, std::string>;

    /// Description of a benchmark parameter with default and valid range
    struct BenchParamDef {
      std::string name;
      BenchParamValue defaultVal;
      BenchParamValue minVal;   // ignored for string type
      BenchParamValue maxVal;   // ignored for string type

      /// Convenience constructors
      static BenchParamDef Int(const std::string &name, int def, int lo = 0, int hi = 0){
        return {name, def, lo, hi};
      }
      static BenchParamDef Dbl(const std::string &name, double def, double lo = 0, double hi = 0){
        return {name, def, lo, hi};
      }
      static BenchParamDef Str(const std::string &name, const std::string &def){
        return {name, def, std::string{}, std::string{}};
      }
    };

    /// Resolved parameter set passed to benchmark functions
    class BenchParams {
      std::map<std::string, BenchParamValue> m_vals;
    public:
      void set(const std::string &key, BenchParamValue v){ m_vals[key] = std::move(v); }

      int getInt(const std::string &key) const {
        auto it = m_vals.find(key);
        return it != m_vals.end() ? std::get<int>(it->second) : 0;
      }
      double getDbl(const std::string &key) const {
        auto it = m_vals.find(key);
        return it != m_vals.end() ? std::get<double>(it->second) : 0.0;
      }
      std::string getStr(const std::string &key) const {
        auto it = m_vals.find(key);
        return it != m_vals.end() ? std::get<std::string>(it->second) : "";
      }
    };

    /// Timing result for a single benchmark run
    struct BenchResult {
      std::string key;
      int iterations;
      std::vector<double> timings_us; // microseconds per iteration

      double mean() const {
        if(timings_us.empty()) return 0;
        return std::accumulate(timings_us.begin(), timings_us.end(), 0.0) / timings_us.size();
      }
      double median() const {
        if(timings_us.empty()) return 0;
        auto sorted = timings_us;
        std::sort(sorted.begin(), sorted.end());
        int n = sorted.size();
        return (n % 2) ? sorted[n/2] : (sorted[n/2-1] + sorted[n/2]) / 2.0;
      }
      double minVal() const {
        return timings_us.empty() ? 0 : *std::min_element(timings_us.begin(), timings_us.end());
      }
      double maxVal() const {
        return timings_us.empty() ? 0 : *std::max_element(timings_us.begin(), timings_us.end());
      }
      double stddev() const {
        if(timings_us.size() < 2) return 0;
        double m = mean();
        double sq = 0;
        for(auto t : timings_us) sq += (t-m)*(t-m);
        return std::sqrt(sq / (timings_us.size()-1));
      }
    };

    /// A registered benchmark entry
    struct BenchmarkEntry {
      std::string key;
      std::string description;
      std::vector<BenchParamDef> params;
      std::function<void(const BenchParams&)> func;
    };

    /// Singleton registry for benchmarks
    class ICLUtils_API BenchmarkRegistry {
    public:
      static BenchmarkRegistry &instance(){
        static BenchmarkRegistry reg;
        return reg;
      }

      /// Register a benchmark
      void add(BenchmarkEntry entry){
        m_entries.push_back(std::move(entry));
      }

      /// Get all entries matching a glob-like filter (supports * wildcard)
      std::vector<const BenchmarkEntry*> filter(const std::string &pattern) const {
        std::vector<const BenchmarkEntry*> result;
        for(auto &e : m_entries){
          if(matchGlob(pattern, e.key)) result.push_back(&e);
        }
        return result;
      }

      /// Get all entries
      const std::vector<BenchmarkEntry> &entries() const { return m_entries; }

    private:
      BenchmarkRegistry() = default;
      std::vector<BenchmarkEntry> m_entries;

      static bool matchGlob(const std::string &pattern, const std::string &str){
        if(pattern == "*") return true;
        // simple glob: split on * and check substrings appear in order
        size_t si = 0;
        std::vector<std::string> parts;
        std::string current;
        for(char c : pattern){
          if(c == '*'){ if(!current.empty()) parts.push_back(current); current.clear(); }
          else current += c;
        }
        if(!current.empty()) parts.push_back(current);
        if(parts.empty()) return true;

        bool startsWithStar = (!pattern.empty() && pattern[0] == '*');
        bool endsWithStar = (!pattern.empty() && pattern.back() == '*');

        si = 0;
        for(size_t i = 0; i < parts.size(); ++i){
          size_t pos = str.find(parts[i], si);
          if(pos == std::string::npos) return false;
          if(i == 0 && !startsWithStar && pos != 0) return false;
          si = pos + parts[i].size();
        }
        if(!endsWithStar && si != str.size()) return false;
        return true;
      }
    };

    /// Run a benchmark with warmup and timing
    class ICLUtils_API BenchmarkRunner {
    public:
      BenchmarkRunner(int iterations = 50, double warmupSeconds = 2.0)
        : m_iterations(iterations), m_warmupSeconds(warmupSeconds) {}

      BenchResult run(const BenchmarkEntry &entry, const BenchParams &params) const {
        BenchResult result;
        result.key = entry.key;
        result.iterations = m_iterations;

        // Cold run: trigger static initialization (allocations, image loading, etc.)
        // This call is never timed.
        entry.func(params);

        // Warmup: run repeatedly to settle CPU frequency / thermal state
        if(m_warmupSeconds > 0){
          Time start = Time::now();
          while((Time::now() - start).toSecondsDouble() < m_warmupSeconds){
            entry.func(params);
          }
        }

        // Timed runs
        result.timings_us.resize(m_iterations);
        for(int i = 0; i < m_iterations; ++i){
          Time start = Time::now();
          entry.func(params);
          result.timings_us[i] = (Time::now() - start).toMicroSeconds();
        }
        return result;
      }

      static void printResult(const BenchResult &r, bool csv = false){
        if(csv){
          std::cout << r.key << ","
                    << r.iterations << ","
                    << r.mean() << ","
                    << r.median() << ","
                    << r.minVal() << ","
                    << r.maxVal() << ","
                    << r.stddev() << "\n";
        }else{
          std::cout << std::left << std::setw(45) << r.key
                    << std::right
                    << std::setw(10) << std::fixed << std::setprecision(1) << r.median() << " us"
                    << "  (mean=" << std::setw(10) << r.mean()
                    << "  min=" << std::setw(10) << r.minVal()
                    << "  max=" << std::setw(10) << r.maxVal()
                    << "  stddev=" << std::setw(8) << r.stddev()
                    << "  n=" << r.iterations << ")\n";
        }
      }

      static void printHeader(bool csv = false){
        if(csv){
          std::cout << "key,iterations,mean_us,median_us,min_us,max_us,stddev_us\n";
        }else{
          std::cout << std::string(120, '-') << "\n"
                    << std::left << std::setw(45) << "Benchmark"
                    << std::right << std::setw(13) << "Median"
                    << "  Details\n"
                    << std::string(120, '-') << "\n";
        }
      }

    private:
      int m_iterations;
      double m_warmupSeconds;
    };

    /// Helper for static registration at file scope
    struct BenchmarkRegistrar {
      BenchmarkRegistrar(BenchmarkEntry entry){
        BenchmarkRegistry::instance().add(std::move(entry));
      }
    };

    /// Macro for registering a benchmark at file scope
    /** Usage:
        ICL_REGISTER_BENCHMARK("filter.threshold.lt_32f", "LT threshold on icl32f",
          (std::vector<BenchParamDef>{
            BenchParamDef::Int("width", 1920, 64, 7680),
            BenchParamDef::Int("height", 1080, 64, 4320),
          }),
          [](const BenchParams &p){ ... });
    */
    #define ICL_REGISTER_BENCHMARK(key, desc, params, func)                \
      static ::icl::utils::BenchmarkRegistrar                              \
        ICL_BENCHMARK_REG_##__LINE__(::icl::utils::BenchmarkEntry{         \
          key, desc, params, func})

  } // namespace utils
}
