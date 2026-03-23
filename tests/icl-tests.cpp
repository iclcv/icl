/********************************************************************
**                Image Component Library (ICL)                    **
**  Test runner — discovers and runs registered tests              **
********************************************************************/

#include <ICLUtils/Test.h>
#include <ICLUtils/ProgArg.h>
#include <iostream>
#include <string>
#include <thread>
#include <algorithm>

using namespace icl::utils;

int main(int argc, char **argv){
  pa_init(argc, argv,
    "-list|-l "
    "-filter|-f(pattern=*) "
    "-jobs|-j(int=0) "
    "-verbose|-v ");

  auto &registry = TestRegistry::instance();

  // List mode
  if(pa("-l")){
    std::cout << "Registered tests:\n\n";
    for(auto &e : registry.entries()){
      std::cout << "  " << e.key;
      if(!e.description.empty()) std::cout << "  -- " << e.description;
      std::cout << "\n";
    }
    if(registry.entries().empty()){
      std::cout << "  (none registered -- link against modules that contain tests)\n";
    }
    return 0;
  }

  // Filter
  std::string pattern = pa("-f");
  auto matches = registry.filter(pattern);

  if(matches.empty()){
    std::cerr << "No tests matching '" << pattern << "'\n";
    std::cerr << "Use --list to see available tests.\n";
    return 1;
  }

  bool verbose = pa("-v");
  int jobs = pa("-j");
  if(jobs <= 0) jobs = std::max(1u, std::thread::hardware_concurrency());

  std::cout << "Running " << matches.size() << " test(s)"
            << " [" << jobs << " threads]...\n\n";

  TestRunner runner;
  auto summary = runner.runAll(matches, verbose, jobs);

  TestRunner::printSummary(summary);

  return summary.failed > 0 ? 1 : 0;
}
