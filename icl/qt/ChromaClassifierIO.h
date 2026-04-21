// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/ConfigFile.h>
#include <icl/utils/StringUtils.h>
#include <icl/core/Parable.h>
#include <icl/core/ChromaClassifier.h>
#include <icl/core/ChromaAndRGBClassifier.h>

namespace icl::qt {
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
  } // namespace icl::qt