// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/file-plugins/FileGrabberPluginJPEG.h>

#include <icl/utils/StrTok.h>
#include <icl/io/detail/file-plugins/JPEGDecoder.h>
using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
#ifdef ICL_HAVE_LIBJPEG
  void FileGrabberPluginJPEG::grab(File &file, ImgBase **dest){
    JPEGDecoder::decode(file,dest);
  }
#else
  void FileGrabberPluginJPEG::grab(File &file, ImgBase **dest){
    ERROR_LOG("JPEG support currently not available! \n" <<
              "To enabled JPEG support: you have to compile the ICLIO package\n" <<
              "with -DICL_HAVE_LIBJPEG compiler flag AND with a valid\n" <<
              "LIBJPEG_ROOT set.");
    ERROR_LOG("Destination image is set to NULL, which may cause further errors!");
    (void) file;
    ICL_DELETE( *dest );
  }
#endif

  } // namespace icl::io

#ifdef ICL_HAVE_LIBJPEG
#include <icl/io/FileGrabber.h>  // REGISTER_FILE_GRABBER_PLUGIN
namespace { using icl::io::FileGrabberPluginJPEG; }
#define ICL_JPEG_REG(TAG, EXT)                                                \
  REGISTER_FILE_GRABBER_PLUGIN(TAG, EXT,                                      \
    [](icl::utils::File &f, icl::core::ImgBase **dst) {                       \
      static FileGrabberPluginJPEG impl; impl.grab(f, dst);                   \
    })
ICL_JPEG_REG(jpeg, ".jpeg");
ICL_JPEG_REG(jpg,  ".jpg");
#undef ICL_JPEG_REG
#endif