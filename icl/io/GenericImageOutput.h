// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/ProgArg.h>
#include <memory>
#include <icl/io/ImageOutput.h>

namespace icl::io {
  /// Generic Sink for images
  /** Like the GenericGrabber, the GenericImageOutput provides a string-configurable
      interface for arbitrary image sinks.

      \section BACK Supported Backends

      Supported Backends are:
        - "file" (description=filepattern)
        - "video" (description=output-video-filename,CODEC-FOURCCC=DIV3,VideoSize=VGA,FPS=24)
        - "sm" (SharedMemory output, description=memory-segment-ID)
        - "xcfp" (XCF Publisher output, description=stream-name)
        - "rsb" (Robotics Service Bus Output), description=[comma-sep. transport-list=spread]:scope)
        - "udp" QUdpSocket-based udp transfer, description=host:port

      \section META Image Meta Data

      Only a few backends do actually support sending also image meta data. So far,
      this is only supported by the RSB and by the shared memory backend, however,
      we plan to add this feature at least for the .icl-file core::format. The corresponding
      GenericGrabber backends for these types are also able to deserialize the images meta data
  */

  class ICLIO_API GenericImageOutput : public ImageOutput{
    std::string type;
    std::string description;
    std::shared_ptr<ImageOutput> impl;

    public:

    /// Null constructor
    GenericImageOutput(){}

    /// Create and initialize
    /** @see init */
    GenericImageOutput(const std::string &type, const std::string &description);

    /// Create from given program argument
    GenericImageOutput(const utils::ProgArg &pa);

    /// initialize this instance
    void init(const std::string &type, const std::string &description);

    /// initialization method (from given progarg)
    void init(const utils::ProgArg &pa);

    /// releases the internal plugin (after this, isNull() returns true again!)
    void release();

    /// sends a new image
    virtual void send(const core::Image &image){
      if (impl) {
        impl->send(image);
      }
      else{
        ERROR_LOG("unable to send image with a NULL output");
      }
    }

    /// returns whether this instance was already initialized
    inline bool isNull() const { return !impl; };

    /// retusn current type string
    inline const std::string &getType() const { return type; }

    /// retusn current description string
    inline const std::string &getDescription() const { return description; }

    /// sets the implementations compression options
    virtual void setCompression(const ImageCompressor::CompressionSpec &spec){
      ICLASSERT_THROW(impl,utils::ICLException("GenericImageOutput:setCompression: impl was null"));
      impl->setCompression(spec);
    }

    /// returns the implementation's current compression type (result.first) and quality (result.second)
    virtual CompressionSpec getCompression() const{
      ICLASSERT_THROW(impl,utils::ICLException("GenericImageOutput:getCompression: impl was null"));
      return impl->getCompression();
    }
  };
  } // namespace icl::io