// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <memory>
#include <string>

namespace icl::qt {

  /// RAII modal progress dialog: construct one to pop up a titled, modal progress
  /// bar; assign a percentage to update it; it disappears when it leaves scope.
  /** Typical use around a long operation:
      \code
        {
          qt::ProgressContext progress("Rebuilding bending constraints");
          for (int i = 0; i < n; ++i) {
            ... heavy step ...
            progress = 100.f * (i + 1) / n;     // update (percent, 0..100)
          }
        }   // dialog closes here automatically
      \endcode

      Thread-safe: all Qt work is marshalled to the GUI thread via
      ICLApplication::executeInGUIThread (construction/destruction block so the
      dialog's lifetime matches the object's; updates are posted asynchronously).
      With no GUI application present (headless / console / unit tests) it is a
      harmless no-op, so the same code path works with and without a UI. */
  class ICLQt_API ProgressContext {
    struct Data;
    std::unique_ptr<Data> m_data;

  public:
    /// Show a modal progress dialog titled \a title at \a percent (0..100).
    /** \a minDurationMs > 0 defers showing the dialog until the operation has run
        that long (Qt's QProgressDialog auto-show) — so a quick operation that
        finishes first never flashes a dialog. 0 (default) shows it immediately.

        \a modal (default true) blocks input to the rest of the app while shown.
        Pass false for an operation driven by a *live* widget (e.g. a slider): the
        dialog is then non-modal and shown without activating, so it gives feedback
        without stealing focus / interrupting the drag. */
    explicit ProgressContext(const std::string &title, float percent = 0.f,
                             int minDurationMs = 0, bool modal = true);
    /// Closes the dialog.
    ~ProgressContext();

    ProgressContext(const ProgressContext &) = delete;
    ProgressContext &operator=(const ProgressContext &) = delete;

    /// Update the progress percentage (0..100, clamped).
    ProgressContext &operator=(float percent);

    /// Update the label text shown under the title (defaults to the title).
    void setText(const std::string &text);
    /// Update the window title.
    void setTitle(const std::string &title);
  };

} // namespace icl::qt
