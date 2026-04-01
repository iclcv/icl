// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/FileGrabberPluginJPEG.h>

#include <ICLUtils/StrTok.h>
#include <ICLIO/JPEGDecoder.h>
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
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

  } // namespace io
}// end of the namespace icl
