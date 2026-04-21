// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FileGrabberPluginBICL.h>
#include <icl/io/ImageCompressor.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  void FileGrabberPluginBICL::grab(File &file, ImgBase **dest){
    ICLASSERT_RETURN(dest);
    file.open(File::readBinary);

    const std::vector<icl8u> &data = file.readAll();

    ImageCompressor cmp;
    Image img = cmp.uncompress(data.data(), data.size());
    // The legacy FileGrabberPlugin API takes an `ImgBase **` outparam; the
    // new ImageCompressor returns an Image. Deep-copy across the boundary.
    img.ptr()->deepCopy(dest);
  }

  } // namespace icl::io

#include <icl/io/FileGrabber.h>  // REGISTER_FILE_GRABBER_PLUGIN
namespace {
  using icl::io::FileGrabberPlugin;
  using icl::io::FileGrabberPluginBICL;
  using P = std::unique_ptr<FileGrabberPlugin>;
}
REGISTER_FILE_GRABBER_PLUGIN(bicl, ".bicl", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle1, ".rle1", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle4, ".rle4", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle6, ".rle6", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle8, ".rle8", []{ return P(new FileGrabberPluginBICL); })
#ifdef ICL_HAVE_LIBJPEG
REGISTER_FILE_GRABBER_PLUGIN(jicl, ".jicl", []{ return P(new FileGrabberPluginBICL); })
#endif
#ifdef ICL_HAVE_LIBZ
REGISTER_FILE_GRABBER_PLUGIN(bicl_gz, ".bicl.gz", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle1_gz, ".rle1.gz", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle4_gz, ".rle4.gz", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle6_gz, ".rle6.gz", []{ return P(new FileGrabberPluginBICL); })
REGISTER_FILE_GRABBER_PLUGIN(rle8_gz, ".rle8.gz", []{ return P(new FileGrabberPluginBICL); })
#endif