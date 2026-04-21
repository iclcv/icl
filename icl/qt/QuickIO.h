// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>

#include <string>
#include <vector>

namespace icl::qt {

  /** @{ @name Image I/O (Quick2) */

  /// Write an image to disk
  ICLQt_API void save(const core::Image &image, const std::string &filename);

  /// Show an image using an external viewer
  ICLQt_API void show(const core::Image &image);

  /// Print image parameters to stdout
  ICLQt_API void print(const core::Image &image);

  /** @} */

  /** @{ @name Timing (Quick2) */

  /// Starts a timer
  ICLQt_API void tic(const std::string &label = "");

  /// Stops a timer started with tic()
  ICLQt_API void toc();

  /** @} */

#ifdef ICL_HAVE_QT
  /** @{ @name Qt dialogs and utilities (Quick2) */

  /// Spawns an open-file dialog (thread-safe)
  ICLQt_API std::string openFileDialog(const std::string &filter = "",
                             const std::string &caption = "open file",
                             const std::string &initialDirectory = "_____last",
                             void *parentWidget = 0);

  /// Spawns a save-file dialog (thread-safe)
  ICLQt_API std::string saveFileDialog(const std::string &filter = "",
                             const std::string &caption = "save file",
                             const std::string &initialDirectory = "_____last",
                             void *parentWidget = 0);

  /// Spawns a text input dialog (thread-safe)
  ICLQt_API std::string textInputDialog(const std::string &caption = "text ...",
                              const std::string &message = "please write your text here",
                              const std::string &initialText = "",
                              void *parentWidget = 0,
                              core::ImgBase *visImage = 0,
                              std::vector<std::string> completionOptions = std::vector<std::string>());

  /// Executes a command as a child process and returns its output
  ICLQt_API std::string execute_process(const std::string &command);

  /** @} */
#endif

} // namespace icl::qt
