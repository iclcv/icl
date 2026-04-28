// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/dispatch/AssignRegistry.h>
#include <icl/utils/ProgArg.h>

#include <iostream>

int main(int n, char **args){
  icl::utils::pa_init(n, args, "-src-type|-s(name) -dst-type|-d(name)");

  const std::string src = icl::utils::pa("-s") ? *icl::utils::pa("-s") : std::string();
  const std::string dst = icl::utils::pa("-d") ? *icl::utils::pa("-d") : std::string();

  const auto rules = icl::utils::AssignRegistry::listRules(src, dst);
  for (const auto &[s, d] : rules) {
    std::cout << s << "  ->  " << d << "\n";
  }
  std::cout << rules.size() << " rule(s)\n";
}
