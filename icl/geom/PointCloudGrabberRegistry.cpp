// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom/PointCloudGrabberRegistry.h>
#include <icl/utils/TextTable.h>

#include <mutex>

namespace icl::geom {
  PointCloudGrabberRegistry& pointCloudGrabberRegistry() {
    static PointCloudGrabberRegistry inst(utils::OnDuplicate::Replace);
    return inst;
  }

  std::map<std::string, std::string>& pointCloudGrabberSyntaxMap() {
    static std::map<std::string, std::string> map;
    return map;
  }

  std::string pointCloudGrabberInfoTable() {
    const auto entries = pointCloudGrabberRegistry().entries();
    utils::TextTable t(3, static_cast<int>(entries.size()) + 1, 40);
    t(0,0) = "ID";
    t(1,0) = "Description";
    t(2,0) = "Creation Syntax";
    int i = 1;
    const auto &syntax = pointCloudGrabberSyntaxMap();
    for (const auto &e : entries) {
      t(0,i) = e.key;
      t(1,i) = e.description;
      auto it = syntax.find(e.key);
      t(2,i) = (it != syntax.end()) ? it->second : std::string();
      ++i;
    }
    return t.toString();
  }
} // namespace icl::geom
