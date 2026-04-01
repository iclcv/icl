// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Point.h>
#include <functional>
#include <string>

namespace icl{
  namespace physics{

    class PhysicsPaper3ContextMenu {
      struct Data;
      Data *m_data;

      public:
      PhysicsPaper3ContextMenu(const PhysicsPaper3ContextMenu&) = delete;
      PhysicsPaper3ContextMenu& operator=(const PhysicsPaper3ContextMenu&) = delete;

      using callback = std::function<void(const std::string&)>;

      PhysicsPaper3ContextMenu();

      PhysicsPaper3ContextMenu(const std::string &commaSepEntries);

      ~PhysicsPaper3ContextMenu();

      void addEntry(const std::string &entry);

      void addEntries(const std::string &commaSepEntryList);

      void setCallback(callback cb);

      void show(const utils::Point &screenPos);
    };
  }
}
