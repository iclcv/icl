// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DataStore.h>

#include <icl/utils/AssignRegistry.h>

namespace icl::qt {

  DataStore::DataStore()
    : m_store(std::make_shared<utils::AnyMap>()),
      m_mutex(std::make_shared<std::recursive_mutex>()) {}

  void DataStore::Slot::assignAny(std::any &dst, std::any &src) {
    try {
      utils::AssignRegistry::dispatch(dst, src);
    } catch (const std::runtime_error &) {
      throw DataStore::UnassignableTypesException(src.type().name(),
                                                  dst.type().name());
    }
  }

  DataStore::Slot DataStore::operator[](const std::string &key) {
    if (auto *entry = m_store->findAny(key)) return Slot(entry);
    throw KeyNotFoundException(key);
  }

  // `Slot::render() / install() / link() / registerCallback() /
  // enable() / disable() / removeCallbacks()` implementations live in
  // `icl/qt/HandleVerbDispatch.cpp` — they need every handle type to
  // be complete, so the cascade is isolated to a dedicated TU.

  }  // namespace icl::qt
