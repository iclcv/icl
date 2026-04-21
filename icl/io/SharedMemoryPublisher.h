// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Uncopyable.h>
#include <icl/core/Image.h>
#include <icl/io/SharedMemorySegment.h>
#include <icl/io/ImageOutput.h>

namespace icl::io {
  /// Publisher, that can be used to publish images via SharedMemorySegment
  class ICLIO_API SharedMemoryPublisher : public ImageOutput{
    struct Data;  //!< intenal data
    Data *m_data; //!< intenal data

    public:

    /// Creates a new publisher instance
    /** If memorySegmentName is "", no connection is performed */
    SharedMemoryPublisher(const std::string &memorySegmentName="");

    /// Destructor
    ~SharedMemoryPublisher();

    /// sets the publisher to use a new segment
    void createPublisher(const std::string &memorySegmentName="");

    /// publishs given image
    void publish(const core::ImgBase *image);

    /// wraps publish to implement ImageOutput interface
    virtual void send(const core::Image &image) { publish(image.ptr()); }

    /// returns current memory segment name
    std::string getMemorySegmentName() const;
  };
  } // namespace icl::io