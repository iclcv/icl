// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/file-plugins/FileGrabberPluginBICL.h>
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
namespace { using icl::io::FileGrabberPluginBICL; }
#define ICL_BICL_REG(TAG, EXT)                                                \
  REGISTER_FILE_GRABBER_PLUGIN(TAG, EXT,                                      \
    [](icl::utils::File &f, icl::core::ImgBase **dst) {                       \
      static FileGrabberPluginBICL impl; impl.grab(f, dst);                   \
    })
ICL_BICL_REG(bicl, ".bicl");
ICL_BICL_REG(rle1, ".rle1");
ICL_BICL_REG(rle4, ".rle4");
ICL_BICL_REG(rle6, ".rle6");
ICL_BICL_REG(rle8, ".rle8");
#ifdef ICL_HAVE_LIBJPEG
ICL_BICL_REG(jicl, ".jicl");
#endif
#ifdef ICL_HAVE_LIBZ
ICL_BICL_REG(bicl_gz, ".bicl.gz");
ICL_BICL_REG(rle1_gz, ".rle1.gz");
ICL_BICL_REG(rle4_gz, ".rle4.gz");
ICL_BICL_REG(rle6_gz, ".rle6.gz");
ICL_BICL_REG(rle8_gz, ".rle8.gz");
#endif
#undef ICL_BICL_REG