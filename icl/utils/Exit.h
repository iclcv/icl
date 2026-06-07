// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <functional>

namespace icl::utils {
  /// Process exit that an application front-end can intercept.
  /** Library code that needs to terminate the process after a one-shot
      diagnostic (e.g. `icl-viewer -i list`) should call this instead of
      `std::exit`.  By default it *is* `std::exit(code)`.

      The catch: such diagnostics run from deep inside a library call
      (`ImageSource::init`), which for a GUI tool happens *after*
      `ICLApplication` has spun up `QApplication` and its worker threads.
      `std::exit` then runs static destructors mid-flight and Qt prints
      "QThreadStorage: entry N destroyed before end of thread" noise.
      `ICLApplication` installs a handler (see setExitHandler) that
      terminates without that partial-teardown, keeping the output clean.

      Non-GUI programs install no handler and get plain `std::exit`. */
  [[noreturn]] ICLUtils_API void exit(int code = 0);

  /// Install the handler invoked by utils::exit (should not return).
  /** Pass an empty std::function to restore the std::exit default.  If the
      handler returns, utils::exit falls back to std::exit(code). */
  ICLUtils_API void setExitHandler(std::function<void(int)> handler);
} // namespace icl::utils
