// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Exception.h>

#include <string>

namespace icl{
  namespace qt{
    /// Internally used and caught exception class for the GUI API \ingroup UNCOMMON
    class GUISyntaxErrorException : public utils::ICLException {
      public:
      GUISyntaxErrorException(const std::string &guidef, const std::string &problem) noexcept:
      utils::ICLException(std::string("Syntax Error while parsing:\n\"")+guidef+"\"\n("+problem+")\n") {}
      virtual ~GUISyntaxErrorException() noexcept {}
    };
  } // namespace qt
}
