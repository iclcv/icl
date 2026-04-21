// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>
#include <icl/core/Types.h>
#include <icl/utils/Size.h>

#include <string>

namespace icl::qt {

  /** @{ @name Image creator functions (Quick2) */

  /// Creates a zero-filled Image with given depth (default depth32f)
  ICLQt_API core::Image zeros(int width, int height, int channels = 1,
                               core::depth d = core::depth32f);

  /// Creates an Image filled with ones (default depth32f)
  ICLQt_API core::Image ones(int width, int height, int channels = 1,
                              core::depth d = core::depth32f);

  /// Loads an image file, returning native depth from the file
  ICLQt_API core::Image load(const std::string &filename);

  /// Loads an image file and converts to the given format
  ICLQt_API core::Image load(const std::string &filename, core::format fmt);

  /// Creates a test image (parrot, lena, cameraman, mandril, etc.)
  ICLQt_API core::Image create(const std::string &name,
                                core::format fmt = core::formatRGB);

  /// Grabs a new image from a device, returning native depth
  /** @param dev device driver type (see GenericGrabber)
      @param devSpec device specifier
      @param size output image size (grabber's size if Size::null)
      @param fmt output format
      @param releaseGrabber if true, the grabber is deleted after use */
  ICLQt_API core::Image grab(const std::string &dev, const std::string &devSpec,
                              const utils::Size &size = utils::Size::null,
                              core::format fmt = core::formatRGB,
                              bool releaseGrabber = false);

  /** @} */

} // namespace icl::qt
