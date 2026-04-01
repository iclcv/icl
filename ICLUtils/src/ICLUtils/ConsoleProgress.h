// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <string>

namespace icl{
  namespace utils{

    /// static utility function for displaying some progress information in console
    ICLUtils_API void progress_init(const std::string &text = "Creating LUT");

    /// static utility function for displaying some progress information in console
    ICLUtils_API void progress_finish();

    /// static utility function for displaying some progress information in console
    /** Extra text is show behind the progress bar */
    ICLUtils_API void progress(int curr, int max, const std::string &extraText = "");

  } // namespace utils
}
