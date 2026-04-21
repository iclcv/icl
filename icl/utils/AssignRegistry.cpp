// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/AssignRegistry.h>

#include <algorithm>

namespace icl::utils {

  AssignRegistry &AssignRegistry::instance() {
    static AssignRegistry r;
    return r;
  }

  const AssignRegistry::Fn &
  AssignRegistry::lookup(std::type_index dstT, std::type_index srcT,
                         const char *dstName, const char *srcName) {
    const auto &m = instance().m_map;
    auto dstIt = m.find(dstT);
    if (dstIt == m.end()) {
      throw std::runtime_error(
        std::string("AssignRegistry::dispatch: no rule with destination type ")
        + dstName);
    }
    auto srcIt = dstIt->second.find(srcT);
    if (srcIt == dstIt->second.end()) {
      throw std::runtime_error(
        std::string("AssignRegistry::dispatch: no rule for ")
        + dstName + " = " + srcName);
    }
    return srcIt->second;
  }

  void AssignRegistry::dispatch(std::any &dst, std::any &src) {
    const auto &fn = lookup(std::type_index(dst.type()),
                            std::type_index(src.type()),
                            dst.type().name(), src.type().name());
    fn.any(dst, src);
  }

  void AssignRegistry::dispatch(void *dst, std::type_index dstT,
                                void *src, std::type_index srcT) {
    const auto &fn = lookup(dstT, srcT, dstT.name(), srcT.name());
    fn.ptr(dst, src);
  }

  void AssignRegistry::dispatch(void *dst, const std::string &dstName,
                                void *src, const std::string &srcName) {
    const auto &r = instance();
    auto dstNt = r.m_nameToType.find(dstName);
    auto srcNt = r.m_nameToType.find(srcName);
    if (dstNt == r.m_nameToType.end() || srcNt == r.m_nameToType.end()) {
      throw std::runtime_error(
        std::string("AssignRegistry::dispatch: type not enrolled for ")
        + srcName + " -> " + dstName);
    }
    const auto &fn = lookup(dstNt->second, srcNt->second,
                            dstName.c_str(), srcName.c_str());
    fn.ptr(dst, src);
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

  std::vector<std::pair<std::string, std::string>>
  AssignRegistry::listRules(const std::string &srcFilter,
                            const std::string &dstFilter) {
    // Reverse the name table so we can print the RTTI names alongside
    // the type_indices.  Multiple names can map to the same
    // type_index (alias via the same typeid), but typically one each.
    const auto &r = instance();
    std::unordered_map<std::type_index, std::string> typeToName;
    for (const auto &[name, idx] : r.m_nameToType) {
      typeToName.insert({idx, name});
    }

    std::vector<std::pair<std::string, std::string>> out;
    for (const auto &[dstT, innerMap] : r.m_map) {
      auto dstIt = typeToName.find(dstT);
      const std::string &dstName = (dstIt != typeToName.end()) ? dstIt->second
                                                               : std::string(dstT.name());
      if (!dstFilter.empty() && dstName.find(dstFilter) == std::string::npos) continue;
      for (const auto &[srcT, fn] : innerMap) {
        (void)fn;
        auto srcIt = typeToName.find(srcT);
        const std::string &srcName = (srcIt != typeToName.end()) ? srcIt->second
                                                                 : std::string(srcT.name());
        if (!srcFilter.empty() && srcName.find(srcFilter) == std::string::npos) continue;
        out.emplace_back(srcName, dstName);
      }
    }
    std::sort(out.begin(), out.end());
    return out;
  }

}  // namespace icl::utils
