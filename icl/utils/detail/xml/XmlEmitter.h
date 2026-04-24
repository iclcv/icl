// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Xml.h>

#include <string>

namespace icl::utils::xml::detail {

  /// Serialize the given element subtree (rooted at `root`, may be
  /// null) to an XML 1.0 string per `opts`.  Text and attribute
  /// values are escape-encoded; text already in entity form (as it
  /// came from the parser's raw view) is re-escaped to its canonical
  /// entity form on output.  CDATA is NOT round-tripped — its
  /// contents surface as regular text and are escaped accordingly.
  ICLUtils_API std::string emit(const ElementNode *root,
                                const EmitOptions &opts);

}  // namespace icl::utils::xml::detail
