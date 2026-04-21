// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>

#include <string>

namespace icl::qt {
  /// Internally used and caught exception class for the GUI API \ingroup UNCOMMON
  class GUISyntaxErrorException : public utils::ICLException {
    public:
    GUISyntaxErrorException(const std::string &guidef, const std::string &problem) noexcept:
    utils::ICLException(std::string("Syntax Error while parsing:\n\"")+guidef+"\"\n("+problem+")\n") {}
    virtual ~GUISyntaxErrorException() noexcept {}
  };
  } // namespace icl::qt