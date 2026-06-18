// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Demonstrates qt::ProgressContext: an RAII modal progress bar. The "long task"
// runs in the ICLApp worker thread (run()), so the ProgressContext — created and
// updated off the GUI thread — marshals all its Qt work to the GUI thread (the
// same situation as reporting progress from a physics/sim/worker thread). The
// dialog appears when the object is constructed and vanishes when it leaves scope.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/qt/ProgressContext.h>
#include <thread>
#include <chrono>

GUI gui;

void init() {
  gui << (VBox()
          << ui::Label("Runs a fake long task behind a modal qt::ProgressContext.",
                       {.handle = "info"})
          << ui::Button("run long task", {.handle = "go"}))
      << ui::Show();
}

void run() {
  static ButtonHandle go = gui["go"];
  if (go.wasTriggered()) {
    ProgressContext progress("Rebuilding bending constraints");   // dialog pops up
    const int N = 100;
    for (int i = 0; i < N; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(25)); // simulate work
      progress = 100.f * (i + 1) / N;                            // update percent
    }
  }                                                               // dialog closes here
  std::this_thread::sleep_for(std::chrono::milliseconds(20));     // idle the worker loop
}

int main(int n, char **ppc) {
  return ICLApplication(n, ppc, "", init, run).exec();
}
