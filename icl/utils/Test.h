// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <future>
#include <thread>

namespace icl::utils {
  /// Exception thrown by test assertion macros
  struct TestFailure {
    std::string file;
    int line;
    std::string message;
  };

  /// Result of a single test execution
  struct TestResult {
    std::string key;
    bool passed = false;
    bool skipped = false;
    std::string failMessage;  // empty if passed
    std::string failFile;
    int failLine = 0;
  };

  /// A registered test entry
  struct TestEntry {
    std::string key;
    std::string description;
    std::function<void()> func;
  };

  /// Singleton registry for tests — mirrors BenchmarkRegistry
  class ICLUtils_API TestRegistry {
  public:
    static TestRegistry &instance(){
      static TestRegistry reg;
      return reg;
    }

    void add(TestEntry entry){
      m_entries.push_back(std::move(entry));
    }

    std::vector<const TestEntry*> filter(const std::string &pattern) const {
      std::vector<const TestEntry*> result;
      for(auto &e : m_entries){
        if(matchGlob(pattern, e.key)) result.push_back(&e);
      }
      return result;
    }

    const std::vector<TestEntry> &entries() const { return m_entries; }

  private:
    TestRegistry() = default;
    std::vector<TestEntry> m_entries;

    static bool matchGlob(const std::string &pattern, const std::string &str){
      if(pattern == "*") return true;
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

  /// Runs tests and collects results
  class ICLUtils_API TestRunner {
  public:
    TestResult run(const TestEntry &entry) const {
      TestResult r;
      r.key = entry.key;
      try {
        entry.func();
        r.passed = true;
      } catch(const TestFailure &f){
        r.passed = false;
        r.failMessage = f.message;
        r.failFile = f.file;
        r.failLine = f.line;
      } catch(const std::exception &e){
        r.passed = false;
        r.failMessage = std::string("uncaught exception: ") + e.what();
      } catch(...){
        r.passed = false;
        r.failMessage = "uncaught unknown exception";
      }
      return r;
    }

    struct Summary {
      int total = 0;
      int passed = 0;
      int failed = 0;
      int skipped = 0;
      std::vector<TestResult> failures;
    };

    /// Run multiple tests in parallel, print results in order
    Summary runAll(const std::vector<const TestEntry*> &tests, bool verbose, int jobs = 0) const {
      if(jobs <= 0) jobs = 4;

      Summary summary;
      summary.total = static_cast<int>(tests.size());

      if(jobs == 1){
        // Sequential fallback
        for(auto *entry : tests){
          auto result = run(*entry);
          printResult(result, verbose);
          if(result.passed) summary.passed++;
          else { summary.failed++; summary.failures.push_back(result); }
        }
        return summary;
      }

      // Launch all tests as async tasks
      std::vector<std::future<TestResult>> futures;
      futures.reserve(tests.size());
      for(auto *entry : tests){
        futures.push_back(std::async(std::launch::async, [this, entry]{ return run(*entry); }));
      }

      // Collect and print in submission order
      for(auto &f : futures){
        auto result = f.get();
        printResult(result, verbose);
        if(result.passed) summary.passed++;
        else { summary.failed++; summary.failures.push_back(result); }
      }
      return summary;
    }

    static void printResult(const TestResult &r, bool verbose){
      if(r.passed){
        std::cout << "  \033[32mPASS\033[0m  " << r.key << "\n";
      } else {
        std::cout << "  \033[31mFAIL\033[0m  " << r.key << "\n";
        if(verbose || true){  // always show failure details
          std::cout << "         " << r.failFile << ":" << r.failLine
                    << ": " << r.failMessage << "\n";
        }
      }
    }

    static void printSummary(const Summary &s){
      std::cout << "\n" << std::string(60, '-') << "\n";
      std::cout << s.total << " test(s): "
                << "\033[32m" << s.passed << " passed\033[0m";
      if(s.failed > 0){
        std::cout << ", \033[31m" << s.failed << " failed\033[0m";
      }
      if(s.skipped > 0){
        std::cout << ", " << s.skipped << " skipped";
      }
      std::cout << "\n";

      if(!s.failures.empty()){
        std::cout << "\nFailed tests:\n";
        for(auto &f : s.failures){
          std::cout << "  " << f.key << " — " << f.failFile << ":"
                    << f.failLine << ": " << f.failMessage << "\n";
        }
      }
    }
  };

  /// Helper for static registration at file scope
  struct TestRegistrar {
    TestRegistrar(TestEntry entry){
      TestRegistry::instance().add(std::move(entry));
    }
  };

  // ------------------------------------------------------------------
  // Registration macro
  // ------------------------------------------------------------------

  /// Register a test at file scope
  /** Usage:
      ICL_REGISTER_TEST("core.img.copy", "deep copy preserves size")
      {
        Img8u a(Size(10,10), 1);
        auto b = a.deepCopy();
        ICL_TEST_EQ(a.getSize(), b->getSize());
      }
  */
  #define ICL_TEST_CONCAT2(a, b) a##b
  #define ICL_TEST_CONCAT(a, b) ICL_TEST_CONCAT2(a, b)
  #define ICL_REGISTER_TEST(key, desc)                                    \
    static void ICL_TEST_CONCAT(_icl_test_fn_, __LINE__)();              \
    static ::icl::utils::TestRegistrar                                    \
      ICL_TEST_CONCAT(_icl_test_reg_, __LINE__)(                          \
        ::icl::utils::TestEntry{key, desc,                                \
          ICL_TEST_CONCAT(_icl_test_fn_, __LINE__)});                     \
    static void ICL_TEST_CONCAT(_icl_test_fn_, __LINE__)()

  // ------------------------------------------------------------------
  // Assertion macros
  // ------------------------------------------------------------------

  #define ICL_TEST_FAIL(msg) do {                                         \
      throw ::icl::utils::TestFailure{__FILE__, __LINE__, msg};           \
    } while(0)

  #define ICL_TEST_TRUE(expr) do {                                        \
      if(!(expr)){                                                        \
        std::ostringstream _s;                                            \
        _s << "expected true: " #expr;                                    \
        throw ::icl::utils::TestFailure{__FILE__, __LINE__, _s.str()};    \
      }                                                                   \
    } while(0)

  #define ICL_TEST_FALSE(expr) do {                                       \
      if((expr)){                                                         \
        std::ostringstream _s;                                            \
        _s << "expected false: " #expr;                                   \
        throw ::icl::utils::TestFailure{__FILE__, __LINE__, _s.str()};    \
      }                                                                   \
    } while(0)

  // Note: assertion macros use inline helpers to avoid preprocessor comma
  // issues with template arguments like FixedMatrix<float,4,4>::id().det()
  namespace test_detail {
    template<class A, class B>
    void check_eq(const A &a, const B &b, const char *ea, const char *eb, const char *file, int line){
      if(!(a == b)){
        std::ostringstream s;
        s << ea << " == " << eb << " failed: " << a << " != " << b;
        throw ::icl::utils::TestFailure{file, line, s.str()};
      }
    }
    template<class A, class B>
    void check_ne(const A &a, const B &b, const char *ea, const char *eb, const char *file, int line){
      if(a == b){
        std::ostringstream s;
        s << ea << " != " << eb << " failed: both are " << a;
        throw ::icl::utils::TestFailure{file, line, s.str()};
      }
    }
    template<class A, class B>
    void check_lt(const A &a, const B &b, const char *ea, const char *eb, const char *file, int line){
      if(!(a < b)){
        std::ostringstream s;
        s << ea << " < " << eb << " failed: " << a << " >= " << b;
        throw ::icl::utils::TestFailure{file, line, s.str()};
      }
    }
    template<class A, class B>
    void check_le(const A &a, const B &b, const char *ea, const char *eb, const char *file, int line){
      if(!(a <= b)){
        std::ostringstream s;
        s << ea << " <= " << eb << " failed: " << a << " > " << b;
        throw ::icl::utils::TestFailure{file, line, s.str()};
      }
    }
    template<class A, class B, class E>
    void check_near(const A &a, const B &b, const E &eps, const char *ea, const char *eb, const char *file, int line){
      if(std::abs(a - b) > eps){
        std::ostringstream s;
        s << ea << " ~= " << eb << " failed: " << a << " vs " << b
          << " (diff=" << std::abs(a - b) << ", eps=" << eps << ")";
        throw ::icl::utils::TestFailure{file, line, s.str()};
      }
    }
  } // namespace test_detail

  #define ICL_TEST_EQ(a, b)                                               \
    ::icl::utils::test_detail::check_eq((a), (b), #a, #b, __FILE__, __LINE__)

  #define ICL_TEST_NE(a, b)                                               \
    ::icl::utils::test_detail::check_ne((a), (b), #a, #b, __FILE__, __LINE__)

  #define ICL_TEST_LT(a, b)                                               \
    ::icl::utils::test_detail::check_lt((a), (b), #a, #b, __FILE__, __LINE__)

  #define ICL_TEST_LE(a, b)                                               \
    ::icl::utils::test_detail::check_le((a), (b), #a, #b, __FILE__, __LINE__)


  #define ICL_TEST_NEAR(a, b, eps)                                        \
    ::icl::utils::test_detail::check_near((a), (b), (eps), #a, #b, __FILE__, __LINE__)

  #define ICL_TEST_THROW(expr, ExType) do {                               \
      bool _caught = false;                                               \
      try { (void)(expr); }                                               \
      catch(const ExType &){ _caught = true; }                            \
      catch(...){ }                                                       \
      if(!_caught){                                                       \
        std::ostringstream _s;                                            \
        _s << "expected " #ExType " from: " #expr;                        \
        throw ::icl::utils::TestFailure{__FILE__, __LINE__, _s.str()};    \
      }                                                                   \
    } while(0)

  #define ICL_TEST_NO_THROW(expr) do {                                    \
      try { (void)(expr); }                                               \
      catch(const std::exception &_e){                                    \
        std::ostringstream _s;                                            \
        _s << "unexpected exception from " #expr ": " << _e.what();       \
        throw ::icl::utils::TestFailure{__FILE__, __LINE__, _s.str()};    \
      }                                                                   \
      catch(...){                                                         \
        throw ::icl::utils::TestFailure{__FILE__, __LINE__,               \
          "unexpected unknown exception from " #expr};                    \
      }                                                                   \
    } while(0)

  } // namespace icl::utils