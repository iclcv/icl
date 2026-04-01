// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/Parable.h>
#include <ICLCore/ChromaClassifier.h>
#include <ICLCore/ChromaAndRGBClassifier.h>

namespace icl{
  namespace qt{

    class ICLQt_API ChromaClassifierIO{
      public:
      static void save(const core::ChromaClassifier &cc,
                       const std::string &filename,
                       const std::string &name="chroma-classifier");

      static void save(const core::ChromaAndRGBClassifier &carc,
                       const std::string &filename);

      static core::ChromaClassifier load(const std::string &filename,
                                   const std::string &name="chroma-classifier");

      static core::ChromaAndRGBClassifier loadRGB(const std::string &filename);
    };
  } // namespace qt
}
