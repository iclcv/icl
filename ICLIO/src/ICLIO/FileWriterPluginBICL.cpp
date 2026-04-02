// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/FileWriterPluginBICL.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  FileWriterPluginBICL::FileWriterPluginBICL(const std::string &compressionType,
                                             const std::string &quality):
    compressor(ImageCompressor::CompressionSpec(compressionType,quality)){}

  void FileWriterPluginBICL::write(File &file, const ImgBase *image){
    std::scoped_lock<std::recursive_mutex> lock(mutex);
    const ImageCompressor::CompressedData data = compressor.compress(image);
    file.open(File::writeBinary);
    file.write(data.bytes,data.len);
  }

  } // namespace icl::io