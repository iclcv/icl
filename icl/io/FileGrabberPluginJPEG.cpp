// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FileGrabberPluginJPEG.h>

#include <icl/utils/StrTok.h>
#include <icl/io/JPEGDecoder.h>
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
namespace {
  using icl::io::FileGrabberPlugin;
  using icl::io::FileGrabberPluginJPEG;
  using P = std::unique_ptr<FileGrabberPlugin>;
}
REGISTER_FILE_GRABBER_PLUGIN(jpeg, ".jpeg", []{ return P(new FileGrabberPluginJPEG); })
REGISTER_FILE_GRABBER_PLUGIN(jpg,  ".jpg",  []{ return P(new FileGrabberPluginJPEG); })
#endif