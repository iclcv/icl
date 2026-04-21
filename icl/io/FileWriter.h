// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Uncopyable.h>
#include <icl/core/Types.h>
#include <icl/io/ImageOutput.h>
#include <icl/io/FilenameGenerator.h>
#include <icl/io/FileWriterPlugin.h>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace icl::io {
  ///  File Writer implementation writing images to the hard disc \ingroup FILEIO_G
  /** \section OVERVIEW Overview
      The implementation has been re-designed to provide a structured more flexible
      plugin based interface for writing images using most different file formats.
      Currently the following formats are supported:
      - <b>ppm</b> (8 Bit 3-channel interleaved color image core::format - 24Bit per pixel )
        multi channel images must have a channel count which is a multiple of 3. The
        original channel information is encoded into the header. Other image viewers
        will show e.g. a 6 channel image as a default 3 channel image where the channels
        3,4 and 5 are put <b>under</b> the channels 0, 1 and 2 (<b>vertical extended</b>)
      - <b>pgm</b> ( 8 Bit mono core::format ) Multi channel images are written by a vertical
        of the image
      - <b>pnm</b> on of ppm or pgm (the viewer or grabber will determine the actual format
        by parsing the first line of the image file
      - <b>icl</b> icl file core::format . For best compatibility the icl core::format includes a header
        which is denoted by trailing '#' characters and which determines the images
        parameters. In the data section, the channel-data is put planar into the file
        channel by channel. Apart from the ".csv"-core::format, the ICL core::format is the only
        core::format, which provides different data types (in particular floats and doubles).
      - <b>csv</b> Comma Separated Values core::format. To provide a convenient interface to
        math-programs as gnuplot, matlab or maple this core::format can be use. In contrast
        to all the other formats, the csv-core::format is <em>human readable</em>, which means,
        that each value is written as ASCII text.
      - <b>jpg</b> Format of the <b>J</b>oint <b>P</b>hotographic <b>E</b>xperts <b>G</b>roup.
        Loss-full compressed image core::format (libjpeg required and -DWITH_JPEG_SUPPORT must be
        defined, which is performed automatically by the makefile system)
      - <b>bicl</b> ICL's built-in binary core::format. This core::format does also support loading
        and saving of image meta data
      - <b>rle1</b> for run-length encoded binary images (best core::format for low-noise
        binary images, does also support writing and reading of image meta data)
      - <b>rle4,rle6,rle8</b> for non-binary images with run-length encoding (other than this,
        as rle1
      - <b>jicl</b> only supported with jpeg support, like bicl, but with jpeg compressed
        image data (jpeg compression is set to 70%, does also support saving meta data).


      \section ZLIB Z-Lib support
        All supported file formats (except jpg) can be written/read as gzipped file. This
        feature is available if the libz is found by the makefile system, which automatically
        defines the -DWITH_ZLIB_SUPPORT then. To write a file with zip compression, you
        just have to add an additional ".gz"-suffix to the file name.

      \section SPEED IO-Speed
        Dependent on the particular core::format, the IO process needs a larger or smaller
        amount of time. The following table shows a summary of the I/O Times. The times
        are taken on a 1.6GHz pentium-m notebook, so the actual times may fluctuate:

      <table>
      <tr> <td>          </td>  <td><b>Writing (gz)</b></td>  <td><b>Reading (gz)</b></td>  <td><b>File-utils::Size (640x480-Parrot) (gz)</b></td></tr>
      <tr> <td><b>ppm</b></td>  <td>10ms (100ms)</td>         <td>6ms (25ms)</td>           <td>901K (545K)</td> </tr>
      <tr> <td><b>pnm</b></td>  <td>10ms (100ms)</td>         <td>6ms (25ms)</td>           <td>901K (545K)</td> </tr>
      <tr> <td><b>pgm</b></td>  <td>7ms (120ms)</td>          <td>7ms (25ms)</td>           <td>901K (562K)</td> </tr>

      <tr> <td><b>icl</b></td>  <td>4ms (122ms)</td>          <td>7ms (26ms)</td>           <td>901K (562K)</td> </tr>

      <tr> <td><b>csv</b></td>  <td>800ms (1800ms)</td>       <td>780ms (820ms)</td>        <td>2901K (690K)</td> </tr>

      <tr> <td><b>jpg 10%</b></td>  <td>5ms (not supported)</td> <td>5ms (not supported)</td> <td>15K (not supported)</td> </tr>
      <tr> <td><b>jpg 50%</b></td>  <td>6ms (not supported)</td> <td>5ms (not supported)</td> <td>41K (not supported)</td> </tr>
      <tr> <td><b>jpg 90%</b></td>  <td>5ms (not supported)</td> <td>5ms (not supported)</td> <td>101K (not supported)</td> </tr>
      <tr> <td><b>jpg 100%</b></td>  <td>7ms (not supported)</td> <td>3ms (not supported)</td> <td>269K (not supported)</td> </tr>
      </table>

      \section EX Example
      The following example illustrates using the file writer:
      \code

      #include <icl/io/FileWriter.h>
      #include <icl/qt/Quick.h>

      int main(){
         // create an image
         icl::core::Img8u a = cvt8u(scale(create("parrot"),640,480));

         // create the file writer
         icl::io::FileWriter writer("image_####.jpg");

         // write the file
         writer.write(&a);
      }
      \endcode
  **/
  /// Process-wide registry of file-extension → FileWriterPlugin factories.
  /** Mirrors `GrabberRegister` and `CompressionRegister`. Plugins
      self-register at static-init time via `REGISTER_FILE_WRITER_PLUGIN`
      (see below). The factory is invoked the first time a given extension
      is requested; the resulting plugin is cached for the lifetime of the
      process. */
  class ICLIO_API FileWriterPluginRegister {
    public:

    using Factory = std::function<std::unique_ptr<FileWriterPlugin>()>;

    /// Register `factory` as the constructor for `extension`. Throws on
    /// duplicate registration unless `override` is true (last-writer-wins —
    /// useful for build configurations where ImageMagick claims an
    /// extension that libpng/libjpeg also claim, see FileWriter.cpp).
    static void registerExtension(const std::string &extension,
                                  Factory factory,
                                  bool overrideExisting = false);

    /// Lazily-construct (or return cached) plugin for the given extension.
    /// Returns nullptr if no plugin is registered for `extension`.
    static FileWriterPlugin *getOrCreate(const std::string &extension);

    /// All registered extensions, lex-sorted.
    static std::vector<std::string> extensions();

    private:
    FileWriterPluginRegister() = default;
    static FileWriterPluginRegister &instance();

    std::mutex m_mutex;
    std::map<std::string, Factory, std::less<>>                          m_factories;
    std::map<std::string, std::unique_ptr<FileWriterPlugin>, std::less<>> m_cache;
  };

  class ICLIO_API FileWriter : public ImageOutput{
    public:
    /// creates an empty file writer
    FileWriter();

    /// Creates a new filewriter with given filepattern
    /** @param filepattern this string is passed to the member FilenameGenerator
               @see FilenameGenerator
    **/
    FileWriter(const std::string &filepattern);

    /// Creates a new FileWriter with given FilenameGenerator
    FileWriter(const FilenameGenerator &gen);

    /// Destructor
    ~FileWriter();

    /// returns the wrapped filename generator reference
    const FilenameGenerator &getFilenameGenerator() const;

    /// writes the next image
    void write(const core::ImgBase *image);

    /// wraps write to implement ImageOutput interface
    virtual void send(const core::Image &image) { write(image.ptr()); }

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

/// Self-register a `FileWriterPlugin` factory for a given file extension.
/** Use exactly once per (plugin, extension) pair, at the bottom of the
    plugin's `.cpp`. The TAG must be a unique C++ identifier within the
    TU; the EXTENSION is the matched suffix including the leading dot.
    \code
      REGISTER_FILE_WRITER_PLUGIN(ppm, ".ppm", []{
        return std::make_unique<icl::io::FileWriterPluginPNM>();
      })
      REGISTER_FILE_WRITER_PLUGIN(pgm, ".pgm", []{
        return std::make_unique<icl::io::FileWriterPluginPNM>();
      })
    \endcode
    `__attribute__((constructor, used))` puts the registration call into
    the dylib's `__init_offsets` section — guaranteed to run on dlopen
    even if nothing else in the .o is referenced. */
#define REGISTER_FILE_WRITER_PLUGIN(TAG, EXTENSION, FACTORY)                   \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterFileWriterPlugin_##TAG() {                                        \
    ::icl::io::FileWriterPluginRegister::registerExtension(EXTENSION, FACTORY);\
  }