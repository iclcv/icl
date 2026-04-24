// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Xml.h>

#include <string_view>

namespace icl::utils::xml::detail {

  /// Parse `src` into `doc`'s tree.  Attribute and element-name views
  /// point directly into `src`; entity-decoded attribute values and
  /// text content are interned into `doc`'s string arena on demand
  /// (and only when the raw bytes contain an entity reference).
  ///
  /// Throws `ParseError` on malformed input.
  ICLUtils_API void parseInto(std::string_view src, Document &doc);

  /// Decode XML entity references in `raw` into `out`.  Handles the
  /// five built-ins (&amp; &lt; &gt; &quot; &apos;) plus numeric
  /// refs (&#NNN; &#xNNN;).  Returns true on success; false (with
  /// partial output) on a malformed reference.  `line`/`col` are
  /// used for diagnostics in the caller.
  ICLUtils_API bool decodeEntities(std::string_view raw, std::string &out);

}  // namespace icl::utils::xml::detail
