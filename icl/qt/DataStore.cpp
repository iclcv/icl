// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DataStore.h>

#include <icl/qt/MouseHandler.h>
#include <icl/utils/AssignRegistry.h>

namespace icl::qt {

  DataStore::DataStore()
    : m_store(std::make_shared<utils::AnyMap>()),
      m_mutex(std::make_shared<std::recursive_mutex>()) {}

  void DataStore::Data::assignAny(std::any &dst, std::any &src) {
    try {
      utils::AssignRegistry::dispatch(dst, src);
    } catch (const std::runtime_error &) {
      throw DataStore::UnassignableTypesException(src.type().name(),
                                                  dst.type().name());
    }
  }

  DataStore::Data DataStore::operator[](const std::string &key) {
    if (auto *entry = m_store->findAny(key)) return Data(entry);
    throw KeyNotFoundException(key);
  }

  void DataStore::Data::install(std::function<void(const MouseEvent &)> f) {
    /// Locally-defined adapter — keeps the function alive inside a
    /// MouseHandler subclass so the handle's usual install(MouseHandler*)
    /// path works.
    struct FunctionMouseHandler : public MouseHandler {
      std::function<void(const MouseEvent &)> f;
      FunctionMouseHandler(std::function<void(const MouseEvent &)> f) : f(f) {}
      void process(const MouseEvent &e) { f(e); }
    };

    install(new FunctionMouseHandler(f));
  }

  }  // namespace icl::qt
