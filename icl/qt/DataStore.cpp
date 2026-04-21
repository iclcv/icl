// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DataStore.h>

#include <icl/qt/MouseHandler.h>
#include <icl/utils/AssignRegistry.h>

namespace icl::qt {

  void DataStore::Data::assign(void *src, const std::string &srcType,
                               void *dst, const std::string &dstType) {
    // All assignment rules live in the runtime AssignRegistry (enrolled
    // at static-init time via __attribute__((constructor)) blocks in
    // each handle's TU).  DataStore owns the raw `void* + RTTI-name`
    // storage, so the name-keyed dispatch overload is the one we want;
    // it translates to `type_index` internally via AssignRegistry's
    // shadow name table.
    try {
      utils::AssignRegistry::dispatch(dst, dstType, src, srcType);
    } catch (const std::runtime_error &) {
      throw DataStore::UnassignableTypesException(srcType, dstType);
    }
  }

  DataStore::Data DataStore::operator[](const std::string &key) {
    if (auto it = m_oDataMapPtr->find(key); it == m_oDataMapPtr->end()) {
      throw KeyNotFoundException(key);
    } else {
      return Data(&it->second);
    }
  }

  void DataStore::Data::install(std::function<void(const MouseEvent &)> f) {
    /// crazy local class here!
    struct FunctionMouseHandler : public MouseHandler {
      std::function<void(const MouseEvent &)> f;
      FunctionMouseHandler(std::function<void(const MouseEvent &)> f) : f(f) {}
      void process(const MouseEvent &e) { f(e); }
    };

    install(new FunctionMouseHandler(f));
  }

  }  // namespace icl::qt
