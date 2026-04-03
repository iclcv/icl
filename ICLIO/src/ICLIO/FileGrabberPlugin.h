// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/File.h>
#include <ICLCore/Img.h>

namespace icl::io {
  /// interface for ImageGrabber Plugins for reading different file types \ingroup FILEIO_G
  class ICLIO_API FileGrabberPlugin{
    public:
#ifdef ICL_HAVE_LIBJPEG
    friend class JPEGDecoder;
#endif

    virtual ~FileGrabberPlugin() {}
    /// pure virtual grab function
    virtual void grab(utils::File &file, core::ImgBase **dest)=0;

    protected:
    /// Internally used collection of image parameters
    struct HeaderInfo{
      core::format imageFormat; ///!< format
      core::depth imageDepth;   ///!< depth
      utils::Rect roi;           ///!< roi rect
      utils::Time time;          ///!< time stamp
      utils::Size size;          ///!< image size
      int channelCount;   ///!< image channel count
      int imageCount;     ///!< image count (e.g. when writing color images as .pgm gray-scale images)
    };
  };
  } // namespace icl::io