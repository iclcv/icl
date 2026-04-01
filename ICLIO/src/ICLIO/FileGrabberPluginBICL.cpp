// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/FileGrabberPluginBICL.h>
#include <ICLIO/ImageCompressor.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    void FileGrabberPluginBICL::grab(File &file, ImgBase **dest){
      ICLASSERT_RETURN(dest);
      file.open(File::readBinary);

      const std::vector<icl8u> &data = file.readAll();

      ImageCompressor cmp;
      cmp.uncompress(data.data(), data.size(), dest);
    }

  } // namespace io
}
