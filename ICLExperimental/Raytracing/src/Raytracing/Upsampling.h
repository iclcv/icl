// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLCore/Img.h>
#include <cstdint>
#include <vector>

namespace icl::rt {

/// Bilinear upsample an RGB Img8u from its current size to dstW × dstH.
void upsampleBilinear(const core::Img8u &src, core::Img8u &dst,
                      int dstW, int dstH);

/// Edge-aware (joint bilateral) upsample. Preserves silhouette edges using
/// luminance-guided weights. sigma_s = spatial sigma in source pixels,
/// sigma_r = range sigma in luminance units (0–255).
void upsampleEdgeAware(const core::Img8u &src, core::Img8u &dst,
                       int dstW, int dstH,
                       float sigma_s = 1.5f, float sigma_r = 30.0f);

/// Nearest-neighbor upsample for integer buffers (object IDs, etc.).
void upsampleNearestInt(const std::vector<int32_t> &src, int srcW, int srcH,
                        std::vector<int32_t> &dst, int dstW, int dstH);

} // namespace icl::rt
