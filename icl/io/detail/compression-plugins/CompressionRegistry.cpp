// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/compression-plugins/CompressionRegistry.h>

namespace icl::io {
  CompressionPluginRegistry& compressionRegistry() {
    static CompressionPluginRegistry inst(utils::OnDuplicate::Throw);
    return inst;
  }
} // namespace icl::io
