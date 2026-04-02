// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Image.h>
#include <ICLIO/ImageCompressor.h>

namespace icl::io {
    /// Minimal interface for image output classes
    /** The image output is used as generic interface for image sinks.
        Usually, it is recommended to use the GenericImageOutput class
        tha provides a string-based interface to set up the output
        backend.

        \section CMP Compression
        A few outputs do also support generic image compression, while
        other implementation provide output dependend compression parameters.
        E.g. shared memory or RSB-based network output streams use
        the inherited ImageCompressor to compress sent data. The file- our
        video image output of course use the used video/file formats compression
        mechanism.
    */
    struct ICLIO_API ImageOutput : protected ImageCompressor{
      /// virtual destructor
      virtual ~ImageOutput() {}

      /// ImageOutput instances must implement this method
      virtual void send(const core::Image &image) = 0;

      /// provide the protectedly inherited image compressor options here
      using ImageCompressor::getCompression;

      /// provide the protectedly inherited image compressor options here
      using ImageCompressor::setCompression;
    };
  } // namespace icl::io