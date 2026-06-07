// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/core/Image.h>
#include <string>

namespace icl::io {
  /// Process-wide registry of test-image factories keyed by name.
  /** Each factory returns a freshly-created `core::Image`.  The built-in
      images (lena, parrot, cameraman, …) self-register at static-init
      time via `REGISTER_TEST_IMAGE` — new images drop in the same way,
      and `TestImages::create` / `CreateGrabber` pick them up
      automatically with no edits to either. */
  using TestImageRegistry = utils::FunctionPluginRegistry<core::Image()>;
  ICLIO_API TestImageRegistry& testImageRegistry();

  /// Utility class for creating test images \ingroup UTILS_G
  /** Available names are the keys of testImageRegistry(); the built-in
      set is lena, cameraman, mandril, parrot, flowers, windows, women,
      tree and house. */
  class ICLIO_API TestImages{
    public:
    /// Creates a test image at the given size, format and depth.
    /** @param name name identifier of the image (see testImageRegistry().keys())
        @param size destination size of the image
        @param f core::format of the image
        @param d core::depth of the image
        @return new Image (null on unknown name) */
    static core::Image create(const std::string& name,
                              const utils::Size &size,
                              core::format f = core::formatRGB,
                              core::depth d = core::depth8u);

    /// Creates a test image at its native size.
    /** @param name name identifier of the image
        @param f core::format of the image
        @param d core::depth of the image
        @return new Image (null on unknown name) */
    static core::Image create(const std::string& name,
                              core::format f = core::formatRGB,
                              core::depth d = core::depth8u);

    private:
    /// internal factory lookup, returns a null Image on miss
    static core::Image internalCreate(const std::string &name);
  };

  } // namespace icl::io

/// Self-register a test-image factory at static-init time.
/** Use exactly once per name, at the bottom of a .cpp that defines the
    factory body:
    \code
      REGISTER_TEST_IMAGE(lena, createImage_lena);
    \endcode
    The factory must be invocable as `core::Image()`. */
#define REGISTER_TEST_IMAGE(NAME, FACTORY)                                     \
  ICL_REGISTER_PLUGIN(::icl::io::testImageRegistry(),                          \
                      NAME, #NAME, FACTORY)
