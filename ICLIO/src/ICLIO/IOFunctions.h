// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert Haschke

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <string>

/// icl namespace
namespace icl {
  namespace io{

    /// draws a label into the upper left image corner \ingroup UTILS_G
    /** This utility function can be used e.g. to identify images in longer
        computation queues. Internally is uses a static map of hard-coded
        ascii-art letters ('a'-'z)'=('A'-'Z'), ('0'-'9') and ' '-'/' are defined yet.
        which associates letters to letter images and corresponding offsets.
        Some tests showed, that is runs very fast (about 100ns per call).
        Note, that no line-break mechanism is implemented, so the labeling
        is restricted to a single line, which is cropped, if the label would
        overlap with the right or bottom  image border.
    */
    ICLIO_API void labelImage(core::ImgBase *image, const std::string &label);

  } // namespace io
} //namespace icl
