// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <memory>

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/ImageOutput.h>
#include <ICLUtils/Uncopyable.h>

#include <string>
#include <opencv2/videoio.hpp>

namespace icl::io {
  class ICLIO_API OpenCVVideoWriter :public ImageOutput{
    private:
    ///OpenCV VideoWriter struct
	  std::unique_ptr<cv::VideoWriter> writer;
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
          * FLV1 (for FLV1 code)
          * on linux: IYUV for IYUV codec ??
          * on windows: "" for open dialog

	    @param fps frames per second
	    @param frame_size size of the frames to be written out
	    @param frame_color currently only supported on windows 0 for greyscale else color
          **/
	OpenCVVideoWriter(const std::string &filename, const std::string &fourcc,
                        double fps, utils::Size frame_size, int frame_color=1);

	/// Destructor
	~OpenCVVideoWriter();

	/// writes the next image
	void write(const core::ImgBase *image);

      /// wraps write to implement ImageOutput interface
      virtual void send(const core::Image &image) { write(image.ptr()); }

	/// as write but in stream manner
	OpenCVVideoWriter &operator<<(const core::ImgBase *image);
  };
  } // namespace icl::io