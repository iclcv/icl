// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Identity-only `Assign<H, H>` enrollments for qt handles that don't
// have their own per-handle enrollment TU (typically because they
// expose no cross-type assignment surface — Event-driven handles,
// container handles, or legacy ones).  These are the rules that let
// `HandleType h = gui["name"]` dispatch through the runtime registry
// via DataStore's `as<T>()` path.

#include <icl/utils/dispatch/AssignRegistry.h>

#include <icl/qt/BorderHandle.h>
#include <icl/qt/BoxHandle.h>
#include <icl/qt/DispHandle.h>
#include <icl/qt/FPSHandle.h>
#include <icl/qt/PlotHandle.h>
#include <icl/qt/SplitterHandle.h>
#include <icl/qt/StateHandle.h>

// MultiDrawHandle is intentionally NOT enrolled — its QObject base has
// a deleted copy-assignment, so `Assign<H, H>` is not well-formed.
// The legacy DataStore didn't ship an ADD_T_TO_T(MultiDrawHandle) rule
// either, and no consumer uses the `MultiDrawHandle h = gui["name"]`
// pattern in tree.

namespace {
  using icl::utils::AssignRegistry;
  using namespace icl::qt;
  __attribute__((constructor))
  static void icl_register_handle_identity_enrollments() {
    AssignRegistry::enroll_identity<
        BorderHandle,
        BoxHandle,
        DispHandle,
        FPSHandle,
        PlotHandle,
        SplitterHandle,
        StateHandle>();
  }
}
