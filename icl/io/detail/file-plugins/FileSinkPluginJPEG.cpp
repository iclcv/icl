// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/file-plugins/FileSinkPluginJPEG.h>
#include <icl/utils/prop/Constraints.h>

#ifdef ICL_HAVE_LIBJPEG
#include <icl/io/detail/file-plugins/JPEGEncoder.h>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {

  FileSinkPluginJPEG::FileSinkPluginJPEG() {
    addProperty("quality", utils::prop::Range<int>{.min=0, .max=100}, m_quality,
                "JPEG compression quality (0=worst, 100=best, default 90).");
    registerCallback([this](const utils::Configurable::Property &p) {
      if (p.name == "quality") m_quality = p.as<int>();
    });
  }

  FileSinkPluginJPEG &FileSinkPluginJPEG::instance() {
    static FileSinkPluginJPEG inst;
    return inst;
  }

#ifdef ICL_HAVE_LIBJPEG
  void FileSinkPluginJPEG::write(File &file, const ImgBase *image){
    ICLASSERT_RETURN(image);
    // Each call gets its own encoder, so concurrent writes don't share
    // state.  JPEGEncoder owns the depth8u conversion + color-space
    // mapping that this plugin used to duplicate inline.
    JPEGEncoder enc(m_quality);
    enc.writeToFile(image, file.getName());
  }

#else // no JPEG_SUPPORT
  /// empty implementation with warning message!
  void FileSinkPluginJPEG::write(File &file, const ImgBase *poSrc){
    ERROR_LOG("JPEG support currently not available! \n" <<
              "To enabled JPEG support: you have to compile the ICLIO package\n" <<
              "with -DICL_HAVE_LIBJPEG compiler flag AND with a valid\n" <<
              "LIBJPEG_ROOT std::set.");
    (void) file;
    (void) poSrc;
  }
#endif

  } // namespace icl::io

#ifdef ICL_HAVE_LIBJPEG
#include <icl/io/detail/FileWriter.h>  // REGISTER_FILE_SINK_PLUGIN / REGISTER_FILE_SINK_CONFIG
namespace { using icl::io::FileSinkPluginJPEG; }
#define ICL_JPEG_REG(TAG, EXT)                                                 \
  REGISTER_FILE_SINK_PLUGIN(TAG, EXT,                                       \
    [](icl::utils::File &f, const icl::core::ImgBase *img) {                  \
      FileSinkPluginJPEG::instance().write(f, img);                         \
    })
ICL_JPEG_REG(jpeg, ".jpeg");
ICL_JPEG_REG(jpg,  ".jpg");
#undef ICL_JPEG_REG

REGISTER_FILE_SINK_CONFIG(jpeg, "jpeg",
  []() -> icl::utils::Configurable* { return &FileSinkPluginJPEG::instance(); });
#endif
