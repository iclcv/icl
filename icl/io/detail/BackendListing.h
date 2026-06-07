// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/TextTable.h>
#include <icl/utils/StringUtils.h>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace icl::io::detail {

  /// Shared printer for the `-i list` / `-o list` affordances.
  /** `entries` is a list of `(id, "paramHint~explanation")` pairs — the
      same shape on both the source and sink sides now that descriptions
      live in the PluginRegistry Entry. Prints a 4-column table
      (nr / id / parameter / explanation). */
  inline void printBackendTable(const std::string &title,
                                const std::vector<std::pair<std::string,std::string>> &entries){
    utils::TextTable t(4, static_cast<int>(entries.size()) + 1, 28);
    t[0] = utils::tok("nr,id,parameter,explanation", ",");
    for(size_t i=0;i<entries.size();++i){
      const auto parts = utils::tok(entries[i].second, "~");
      t(0, i+1) = utils::str(i);
      t(1, i+1) = entries[i].first;
      t(2, i+1) = parts.size() > 0 ? parts[0] : std::string();
      t(3, i+1) = parts.size() > 1 ? parts[1] : std::string();
    }
    std::cout << title << "\n\n" << t << std::endl;
  }

} // namespace icl::io::detail
