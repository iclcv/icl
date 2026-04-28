// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// TabHandle is otherwise header-only; this TU exists solely to host
// the runtime enrollment of its `Assign<>` pairs (the compile-time
// dispatch needs no .cpp at all).

#include <icl/qt/TabHandle.h>

#include <icl/utils/dispatch/AssignRegistry.h>

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::TabHandle;
  __attribute__((constructor))
  static void icl_register_tab_handle_assignments() {
    AssignRegistry::enroll_provider<TabHandle, bool, int, float, double>();
    AssignRegistry::enroll_identity<TabHandle>();
  }
}
