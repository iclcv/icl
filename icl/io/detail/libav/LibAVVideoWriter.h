// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Size.h>
#include <icl/core/Image.h>

#include <string>

namespace icl::io {
  class ICLIO_API LibAVVideoWriter {
    struct Data;
    Data *m_data;

    public:

	/// Creates a new videowriter with given filename
	/** @param filename the filename to write to
	    @param fourcc this is translated into an instance of FOURCC
          possible is:
          * PIM1 (for mpeg 1)
          * MJPG (for motion jepg)
          * MP42 (for mpeg 4.2)
          * DIV3 (for mpeg 4.3)
          * DIVX (for mpeg 4)
          * U263 (for H263 codec)
          * I263 (for H263I codec)
          * X264 (for H264 codec)
          * FLV1 (for FLV1 code)
          * on linux: IYUV for IYUV codec ??
          * on windows: "" for open dialog

	    @param fps frames per second
      @param frame_size size of the frames to be written out
          **/
  LibAVVideoWriter(const std::string &filename, const std::string &fourcc,
                        double fps, utils::Size frame_size);


	/// Destructor
  ~LibAVVideoWriter();

  /// write an image (value semantics; was virtual ImageOutput::send pre-4a)
  void send(const core::Image &image);

	/// as write but in stream manner
  LibAVVideoWriter &operator<<(const core::ImgBase *image);
  };
  } // namespace icl::io