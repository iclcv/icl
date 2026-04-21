// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/file-plugins/FileWriterPluginBICL.h>
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
// Each lambda has its own per-type static impl with distinct ctor args.
namespace { using icl::io::FileWriterPluginBICL; }
#define ICL_BICL_REG(TAG, EXT, ...)                                           \
  REGISTER_FILE_WRITER_PLUGIN(TAG, EXT,                                       \
    [](icl::utils::File &f, const icl::core::ImgBase *img) {                  \
      static FileWriterPluginBICL impl{__VA_ARGS__}; impl.write(f, img);      \
    })
ICL_BICL_REG(bicl, ".bicl");
ICL_BICL_REG(rle1, ".rle1", "rlen", "1");
ICL_BICL_REG(rle4, ".rle4", "rlen", "4");
ICL_BICL_REG(rle6, ".rle6", "rlen", "6");
ICL_BICL_REG(rle8, ".rle8", "rlen", "8");
#ifdef ICL_HAVE_LIBJPEG
ICL_BICL_REG(jicl, ".jicl", "jpeg", "85");
#endif
#ifdef ICL_HAVE_LIBZ
ICL_BICL_REG(bicl_gz, ".bicl.gz");
ICL_BICL_REG(rle1_gz, ".rle1.gz", "rlen", "1");
ICL_BICL_REG(rle4_gz, ".rle4.gz", "rlen", "4");
ICL_BICL_REG(rle6_gz, ".rle6.gz", "rlen", "6");
ICL_BICL_REG(rle8_gz, ".rle8.gz", "rlen", "8");
#endif
#undef ICL_BICL_REG