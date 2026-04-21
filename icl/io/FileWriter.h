// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/utils/PluginRegistry.h>
#include <icl/utils/Uncopyable.h>
#include <icl/core/Image.h>
#include <icl/core/Img.h>
#include <icl/io/FilenameGenerator.h>

#include <functional>
#include <string>
#include <vector>

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
        icl::core::Img8u a = cvt8u(scale(create("parrot"),640,480));
        icl::io::FileWriter writer("image_####.jpg");
        writer.write(&a);
      \endcode
  **/
  /// Process-wide registry of file-extension → write-callable.
  /** Thin façade over `utils::FunctionPluginRegistry<Signature>` with
      `OnDuplicate::KeepHighestPriority`: whoever registers with the
      highest priority wins a contested extension; ties fall back to
      first-wins. Used so libpng (prio 0) beats ImageMagick (prio -10)
      for `.png` deterministically across dyld static-init orderings.

      The stored callable carries its own state (per-lambda function-local
      statics), so no external per-extension cache is needed — a no-op
      compared to the pre-4b `m_cache` map. */
  class ICLIO_API FileWriterPluginRegister : public utils::Uncopyable {
    public:

    /// Write callable: (file, image) → void. Thread-safety is the
    /// callable's own responsibility (plugin backends use static
    /// per-lambda state with their own mutexes).
    using Factory  = std::function<void(utils::File&, const core::ImgBase*)>;
    using Registry = utils::FunctionPluginRegistry<void(utils::File&, const core::ImgBase*)>;

    /// Register `factory` as the writer for `extension`.
    /// Strictly-higher `priority` wins against any competing registration;
    /// ties resolve first-wins. Pass a negative priority (e.g. -10) to
    /// register as a fallback that only activates when no higher-priority
    /// plugin is available (ImageMagick's pattern for .png / .jpg /
    /// other formats also handled by libpng / libjpeg).
    static void registerExtension(const std::string &extension,
                                  Factory factory,
                                  int priority = 0);

    /// Look up the write callable for `extension`. Returns nullptr if no
    /// plugin is registered for the extension.
    static const Factory *find(const std::string &extension);

    /// All registered extensions, lex-sorted.
    static std::vector<std::string> extensions();

    /// Singleton accessor (also used by `ICL_REGISTER_PLUGIN` / the
    /// `REGISTER_FILE_WRITER_PLUGIN` macro).
    static FileWriterPluginRegister &instance();
    Registry       &registry()       { return m_registry; }
    const Registry &registry() const { return m_registry; }

    private:
    FileWriterPluginRegister() : m_registry(utils::OnDuplicate::KeepHighestPriority) {}
    Registry m_registry;
  };

  class ICLIO_API FileWriter {
    public:
    /// creates an empty file writer
    FileWriter();

    /// Creates a new filewriter with given filepattern
    FileWriter(const std::string &filepattern);

    /// Creates a new FileWriter with given FilenameGenerator
    FileWriter(const FilenameGenerator &gen);

    /// Destructor
    ~FileWriter();

    /// returns the wrapped filename generator reference
    const FilenameGenerator &getFilenameGenerator() const;

    /// writes the next image
    void write(const core::ImgBase *image);

    /// convenience: accept a value-type Image (matches the former
    /// ImageOutput contract). Kept for direct callers; the GenericImageOutput
    /// registry uses `write(image.ptr())` internally.
    void send(const core::Image &image) { write(image.ptr()); }

    /// as write but in stream manner
    FileWriter &operator<<(const core::ImgBase *image);

    /// sets a core::format specific option
    /** currently allowed options are:
        - "jpg:quality"  values of type int in range [0,100]
        - "csv:extend-file-name" value of type bool ("true" or "false")
    **/
    void setOption(const std::string &option, const std::string &value);

    private:
    /// internal generator for new filenames
    FilenameGenerator m_oGen;
  };

  } // namespace icl::io

/// Self-register a write callable for a given file extension.
/** The FACTORY expression must be a callable of signature
    `void(utils::File&, const core::ImgBase*)`. Typically a lambda with
    a function-local static implementation object:
    \code
      REGISTER_FILE_WRITER_PLUGIN(ppm, ".ppm",
        [](utils::File& f, const core::ImgBase* img) {
          static FileWriterPluginPNM impl;
          impl.write(f, img);
        })
    \endcode
    Each lambda definition gets its own per-type static storage, so
    multiple extensions handled by the same backend (e.g. BICL's
    .rle1/.rle4/.jicl variants) each get their own instance with
    distinct ctor args. */
#define REGISTER_FILE_WRITER_PLUGIN(TAG, EXTENSION, ...)                       \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterFileWriterPlugin_##TAG() {                                        \
    ::icl::io::FileWriterPluginRegister::registerExtension(EXTENSION, __VA_ARGS__); \
  }
