// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/File.h>
#include <ICLCore/Img.h>


namespace icl::io {
  /// Interface class for writer plugins writing images in different file formats \ingroup FILEIO_G
  class ICLIO_API FileWriterPlugin{
    public:
    virtual ~FileWriterPlugin() {}
    /// pure virtual writing function
    virtual void write(utils::File &file, const core::ImgBase *image)=0;
  };
  } // namespace icl::io