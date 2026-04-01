// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/File.h>
#include <ICLUtils/Exception.h>
#include <ICLCore/Types.h>

namespace icl{
  namespace io{
    /// Utility class for decoding JPEG-Data streams (with ICL_HAVE_LIBJPEG only)
    class ICLIO_API JPEGDecoder{
      public:
      /// Decode JPEG-File (E.g. used for FileGrabberPluginJPEG)
      /** @param file must be opened in mode readBinary or not opend
          @param dst image, which is adapted to the found image parameters
      */
      static void decode(utils::File &file, core::ImgBase **dst);

      /// Decode a data stream (E.g. used for Decoding Motion-JPEG streams in unicap's DefaultConvertEngine)
      /** @param data jpeg data stream (must be valid, otherwise unpredictable behaviour occurs
          @param maxDataLen length of the given data pointer
                            <b>Note:</b>This is just an upper limit to avoid segmentation faults on
                            corrupted jpeg data (e.g. end-of-image-marker is missing). The given data
                            pointer can be much longer then the actual jpeg data. If that is the case,
                            libjpeg obviously reads only necessary bytes.
          @param dst destination image, which is adapted to the found images parameters */
      static void decode(const unsigned char *data,unsigned int maxDataLen,core::ImgBase **dst);

      private:
      /// internal utility function, which does all the work
      static void decode_internal(utils::File *file,const unsigned char *data,
                                  unsigned int maxDataLen, core::ImgBase **dst);
    };
  } // namespace io
}
