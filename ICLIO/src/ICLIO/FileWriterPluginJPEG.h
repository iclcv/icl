// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileWriterPlugin.h>
#include <mutex>

namespace icl::io {
  /// A Writer Plugin for writing ".jpeg" and ".jpg" images \ingroup FILEIO_G
  class ICLIO_API FileWriterPluginJPEG : public FileWriterPlugin{
    public:
    /// write implementation
    virtual void write(utils::File &file, const core::ImgBase *image);

    /// sets the currently used jped quality (0-100) (by default 90%)
    static void setQuality(int value);

    private:

    /// current quality (90%) by default
    static int s_iQuality;

    /// (static!) internal buffer for Any-to-icl8u conversion
    static core::Img8u s_oBufferImage;

    /// mutex to protect the static buffer
    static std::recursive_mutex s_oBufferImageMutex;
  };
  } // namespace icl::io