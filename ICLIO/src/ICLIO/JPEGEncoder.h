// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
namespace icl { namespace core { class ImgBase; template<class T> class Img; } }
// forward declaration (was #include <ICLCore/ImgBase.h>)

#ifndef ICL_HAVE_LIBJPEG
  #if WIN32
    #pragma WARNING("libjpeg is not available, therefore, this header should not be included")
  #else
    #warning "libjpeg is not available, therefore, this header should not be included"
  #endif
#endif

namespace icl{
  namespace io{
    /// encoding class for data-to-data jpeg compression
    class ICLIO_API JPEGEncoder {
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer

      public:
      JPEGEncoder(const JPEGEncoder&) = delete;
      JPEGEncoder& operator=(const JPEGEncoder&) = delete;

      /// constructor with given jpeg quality
      /** The quality value is always given in percet (1-100)*/
      JPEGEncoder(int quality=90);

      /// Destructor
      ~JPEGEncoder();

      /// sets the compression quality level
      void setQuality(int quality);

      /// encoded data type
      struct EncodedData{
        icl8u *bytes; //!< byte pointer
        int len;      //!< number of bytes used
      };

      /// encodes a given core::ImgBase * (only depth8u is supported natively)
      /** non-depth8u images are automatically converted before compression.
          This might lead to loss of data*/
      const EncodedData &encode(const core::ImgBase *image);

      /// first encodes the jpeg in memory and then write the whole memory chunk to disc
      void writeToFile(const core::ImgBase *image, const std::string &filename);
    };

  } // namespace io
}
