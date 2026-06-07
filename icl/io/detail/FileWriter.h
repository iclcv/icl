// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/utils/config/Configurable.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/core/Image.h>
#include <icl/io/detail/FilenameGenerator.h>

#include <functional>
#include <string>

namespace icl::io {
  ///  File Writer implementation writing images to the hard disc \ingroup FILEIO_G
  /** \section OVERVIEW Overview
      The implementation has been re-designed to provide a structured more flexible
      plugin based interface for writing images using most different file formats.
      Currently the following formats are supported:
      - <b>ppm</b> (8 Bit 3-channel interleaved color image core::format - 24Bit per pixel )
        multi channel images must have a channel count which is a multiple of 3.
      - <b>pgm</b> ( 8 Bit mono core::format ) Multi channel images are written by a vertical
      - <b>pnm</b> on of ppm or pgm
      - <b>icl</b> icl file core::format.
      - <b>csv</b> Comma Separated Values core::format.
      - <b>jpg</b> JPEG format (libjpeg required).
      - <b>bicl</b> ICL's built-in binary core::format.
      - <b>rle1 / rle4 / rle6 / rle8</b> run-length encoded binary.
      - <b>jicl</b> JPEG-compressed bicl (requires libjpeg).

      \section ZLIB Z-Lib support
      All supported file formats (except jpg) can be written/read as gzipped file
      by appending ".gz" to the file name.

      \section EX Example
      \code
        icl::core::Image a = ...;
        icl::io::FileWriter writer("image_####.jpg");
        writer.write(a);

        // Per-plugin tunables hang off the FileWriter as named children:
        writer.setPropertyValue("jpeg.quality", 85);
      \endcode
  **/
  /// Callable type stored in the file-writer registry: `(file, image)`.
  /// Thread-safety is the callable's own responsibility (plugin backends
  /// use static per-lambda state with their own mutexes).
  using FileSinkFn = std::function<void(utils::File&, const core::ImgBase*)>;

  /// Process-wide registry of file-extension → write-callable.
  /** Uses `OnDuplicate::KeepHighestPriority`: whoever registers with the
      highest priority wins a contested extension; ties fall back to
      first-wins. Used so libpng (prio 0) beats ImageMagick (prio -10)
      for `.png` deterministically across dyld static-init orderings.

      The stored callable carries its own state (per-lambda function-local
      statics), so no external per-extension cache is needed. */
  using FileSinkRegistry =
      utils::FunctionPluginRegistry<void(utils::File&, const core::ImgBase*)>;

  /// Singleton accessor for the process-wide file-writer registry.
  ICLIO_API FileSinkRegistry& fileSinkRegistry();

  /// Returns a pointer to a singleton plugin Configurable, or nullptr if
  /// the plugin doesn't expose tunable properties.  Each FileWriter
  /// instance walks this registry at ctor time and adds the non-null
  /// entries as named child Configurables.
  using FileWriterConfigFn = std::function<utils::Configurable*()>;
  using FileWriterConfigRegistry =
      utils::FunctionPluginRegistry<utils::Configurable*()>;
  ICLIO_API FileWriterConfigRegistry& fileWriterConfigRegistry();

  class ICLIO_API FileWriter : public utils::Configurable {
    public:
    /// creates an empty file writer
    FileWriter();

    /// Creates a new filewriter with given filepattern
    FileWriter(const std::string &filepattern);

    /// Creates a new FileWriter with given FilenameGenerator
    FileWriter(const FilenameGenerator &gen);

    /// returns the wrapped filename generator reference
    const FilenameGenerator &getFilenameGenerator() const { return m_oGen; }

    /// Writes the image to the next filename in the generator's sequence.
    /** Extension of the generated filename dispatches into
        fileSinkRegistry() to select the matching plugin. */
    void write(const core::Image &image);

    private:
    /// internal generator for new filenames
    FilenameGenerator m_oGen;

    void attachPluginConfigurables();
  };

  } // namespace icl::io

/// Self-register a write callable for a given file extension.
/** The FACTORY expression must be a callable of signature
    `void(utils::File&, const core::ImgBase*)`. Typically a lambda with
    a function-local static implementation object:
    \code
      REGISTER_FILE_SINK_PLUGIN(ppm, ".ppm",
        [](utils::File& f, const core::ImgBase* img) {
          static FileSinkPluginPNM impl;
          impl.write(f, img);
        })
    \endcode
    Each lambda definition gets its own per-type static storage, so
    multiple extensions handled by the same backend (e.g. BICL's
    .rle1/.rle4/.jicl variants) each get their own instance with
    distinct ctor args. */
#define REGISTER_FILE_SINK_PLUGIN(TAG, EXTENSION, ...)                       \
  ICL_REGISTER_PLUGIN(::icl::io::fileSinkRegistry(), TAG, EXTENSION, __VA_ARGS__)

/// Self-register the singleton Configurable of a file-writer plugin.
/** Each registered factory is invoked once per FileWriter ctor and the
    returned Configurable* is added as a named child under PREFIX.  Used
    to surface per-plugin tunables (jpeg quality, csv extend-file-name)
    on every FileWriter instance — `writer.setPropertyValue("jpeg.quality", 85)`. */
#define REGISTER_FILE_SINK_CONFIG(TAG, PREFIX, FACTORY)                      \
  ICL_REGISTER_PLUGIN(::icl::io::fileWriterConfigRegistry(), TAG, PREFIX, FACTORY)
