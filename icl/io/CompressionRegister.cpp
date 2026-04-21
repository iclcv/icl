// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/CompressionRegister.h>
#include <icl/utils/Exception.h>

namespace icl::io {
  using namespace icl::utils;

  CompressionRegister &CompressionRegister::instance() {
    static CompressionRegister inst;
    return inst;
  }

  void CompressionRegister::registerPlugin(const std::string &name,
                                           Factory factory) {
    std::scoped_lock lk(m_mutex);
    if (m_factories.find(name) != m_factories.end()) {
      throw ICLException("CompressionRegister::registerPlugin: codec '"
                         + name + "' is already registered");
    }
    m_factories[name] = std::move(factory);
  }

  std::unique_ptr<CompressionPlugin>
  CompressionRegister::create(const std::string &name) {
    auto &inst = instance();
    std::scoped_lock lk(inst.m_mutex);
    auto it = inst.m_factories.find(name);
    if (it == inst.m_factories.end()) {
      throw ICLException("CompressionRegister::create: codec '"
                         + name + "' is not registered "
                         "(check that ICL was built with the required "
                         "optional dependency, e.g. ICL_HAVE_ZSTD)");
    }
    return (it->second)();
  }

  std::vector<std::string> CompressionRegister::names() {
    auto &inst = instance();
    std::scoped_lock lk(inst.m_mutex);
    std::vector<std::string> out;
    out.reserve(inst.m_factories.size());
    for (const auto &[n, _] : inst.m_factories) out.push_back(n);
    return out;  // already sorted (std::map iteration order)
  }

  bool CompressionRegister::has(const std::string &name) {
    auto &inst = instance();
    std::scoped_lock lk(inst.m_mutex);
    return inst.m_factories.find(name) != inst.m_factories.end();
  }
} // namespace icl::io
