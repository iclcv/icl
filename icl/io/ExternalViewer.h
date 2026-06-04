// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

// Free functions for spawning an external image viewer.  Writes the
// image to a temp file on disk and launches the configured shell
// command (default: ICLQt's `icl-xv`).  Independent of any Qt build —
// uses only FileWriter + system() + sleep.

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>
#include <string>

namespace icl::io {

  /// Write the image to disk, then launch xv (or a chosen viewer) on it.
  /** @param image image to display (null images are no-ops)
      @param tmpName temp file path; `.pgm` is auto-appended for non-3-channel
                     images so xv picks the right loader.
      @param msec_to_rm_call this many milliseconds are slept after xv launch
                             before the temp file is removed (gives xv time
                             to come up and read it). */
  ICLIO_API void xv(const core::Image &image,
                    const std::string &tmpName = "./tmp_image.ppm",
                    long msec_to_rm_call = 1000);

  /// Write the image to disk, then launch an external viewer command.
  /** @param image image to display (null images are no-ops)
      @param showCommand viewer command; the `%s` token is replaced by the
                         temp file path.  Default uses ICLQt's `icl-xv`
                         viewer — make sure it is in $PATH.
      @param msec_to_rm_call viewer-doesn't-read-and-go delay before the
                             temp file is removed.
      @param rmCommand shell command to delete the temp file (e.g. `"rm
                       -rf %s"`); empty disables the cleanup step. */
  ICLIO_API void show(const core::Image &image,
                      const std::string &showCommand = "icl-xv -input %s -delete",
                      long msec_to_rm_call = 0,
                      const std::string &rmCommand = "");

} // namespace icl::io
