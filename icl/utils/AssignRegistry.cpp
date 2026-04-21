// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/AssignRegistry.h>

namespace icl::utils {

  AssignRegistry &AssignRegistry::instance() {
    static AssignRegistry r;
    return r;
  }

  void AssignRegistry::dispatch(std::any &dst, std::any &src) {
    const auto &m = instance().m_map;
    auto dstIt = m.find(std::type_index(dst.type()));
    if (dstIt == m.end()) {
      throw std::runtime_error(
        std::string("AssignRegistry::dispatch: no rule with destination type ")
        + dst.type().name());
    }
    auto srcIt = dstIt->second.find(std::type_index(src.type()));
    if (srcIt == dstIt->second.end()) {
      throw std::runtime_error(
        std::string("AssignRegistry::dispatch: no rule for ")
        + dst.type().name() + " = " + src.type().name());
    }
    srcIt->second(dst, src);
  }

  bool AssignRegistry::has(std::type_index dstType,
                           std::type_index srcType) noexcept {
    const auto &m = instance().m_map;
    auto dstIt = m.find(dstType);
    if (dstIt == m.end()) return false;
    return dstIt->second.find(srcType) != dstIt->second.end();
  }

  std::size_t AssignRegistry::size() noexcept {
    std::size_t total = 0;
    for (const auto &[k, inner] : instance().m_map) {
      (void)k;
      total += inner.size();
    }
    return total;
  }

}  // namespace icl::utils
