// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FileWriterPluginBICL.h>
#include <icl/io/FileWriter.h>      // for REGISTER_FILE_WRITER_PLUGIN macro
#include <icl/io/ImageCompressor.h>
#include <icl/core/Image.h>
#include <icl/core/ImgBase.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  FileWriterPluginBICL::FileWriterPluginBICL(const std::string &compressionType,
                                             const std::string &quality):
    compressionType(compressionType), compressionQuality(quality) {}

  // Out-of-line so the unique_ptr's deleter sees the complete
  // ImageCompressor type (forward-declared in the header).
  FileWriterPluginBICL::~FileWriterPluginBICL() = default;

  void FileWriterPluginBICL::write(File &file, const ImgBase *image){
    std::scoped_lock<std::recursive_mutex> lock(mutex);
    if (!compressor) {
      // Lazy build — this runs after main(), so the
      // CompressionRegister is guaranteed populated.
      compressor.reset(new ImageCompressor(
        ImageCompressor::CompressionSpec(compressionType, compressionQuality)));
    }
    // Wrap the legacy `ImgBase *` in an Image (shallow — we don't take
    // ownership) for the new Image-based ImageCompressor API.
    const ImageCompressor::CompressedData data = compressor->compress(Image(*image));
    file.open(File::writeBinary);
    file.write(data.bytes,data.len);
  }

  } // namespace icl::io

// ----- registrations: BICL handles its own ext + the rle1/4/6/8/jicl
// pseudo-extensions that just instantiate BICL with different codec specs.
// Each gets its own factory (the plugin instance is stateful — it owns
// its compressor — and each codec spec needs a distinct instance).
namespace {
  using icl::io::FileWriterPlugin;
  using icl::io::FileWriterPluginBICL;
  using P = std::unique_ptr<FileWriterPlugin>;
}
REGISTER_FILE_WRITER_PLUGIN(bicl, ".bicl", []{ return P(new FileWriterPluginBICL); })
REGISTER_FILE_WRITER_PLUGIN(rle1, ".rle1", []{ return P(new FileWriterPluginBICL("rlen","1")); })
REGISTER_FILE_WRITER_PLUGIN(rle4, ".rle4", []{ return P(new FileWriterPluginBICL("rlen","4")); })
REGISTER_FILE_WRITER_PLUGIN(rle6, ".rle6", []{ return P(new FileWriterPluginBICL("rlen","6")); })
REGISTER_FILE_WRITER_PLUGIN(rle8, ".rle8", []{ return P(new FileWriterPluginBICL("rlen","8")); })
#ifdef ICL_HAVE_LIBJPEG
REGISTER_FILE_WRITER_PLUGIN(jicl, ".jicl", []{ return P(new FileWriterPluginBICL("jpeg","85")); })
#endif
#ifdef ICL_HAVE_LIBZ
REGISTER_FILE_WRITER_PLUGIN(bicl_gz, ".bicl.gz", []{ return P(new FileWriterPluginBICL); })
REGISTER_FILE_WRITER_PLUGIN(rle1_gz, ".rle1.gz", []{ return P(new FileWriterPluginBICL("rlen","1")); })
REGISTER_FILE_WRITER_PLUGIN(rle4_gz, ".rle4.gz", []{ return P(new FileWriterPluginBICL("rlen","4")); })
REGISTER_FILE_WRITER_PLUGIN(rle6_gz, ".rle6.gz", []{ return P(new FileWriterPluginBICL("rlen","6")); })
REGISTER_FILE_WRITER_PLUGIN(rle8_gz, ".rle8.gz", []{ return P(new FileWriterPluginBICL("rlen","8")); })
#endif