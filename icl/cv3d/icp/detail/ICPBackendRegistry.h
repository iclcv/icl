// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Non-installed: the name->factory registry for ICP nearest-neighbour backends.
// Concrete backends (detail/OctreeNN.cpp, ColorNN.cpp, CLBackends.cpp) self-register
// here at library-load time; ICP.cpp looks them up for setBackend(name) /
// createBackend(). Kept out of the installed ICP.h so the concrete backend types
// stay implementation detail — callers see only ICP::Backend / ICP::ColorBackend.

#pragma once

#include <icl/cv3d/icp/ICP.h>
#include <icl/utils/plugin/PluginRegistry.h>

#include <functional>
#include <memory>
#include <string>

namespace icl::cv3d {

  /// factory producing a fresh backend instance (stateful backends are fine —
  /// each call yields a new one the caller then configures)
  using ICPBackendFactory = std::function<std::shared_ptr<ICP::Backend>()>;

  /// The process-wide ICP backend registry (single shared instance across TUs).
  /** first-wins on duplicate names; priority orders backendNames() (higher first). */
  inline utils::PluginRegistry<std::string, ICPBackendFactory> &icpBackendRegistry() {
    static utils::PluginRegistry<std::string, ICPBackendFactory>
        reg(utils::OnDuplicate::KeepFirst);
    return reg;
  }

} // namespace icl::cv3d