// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLCore/Img.h>

namespace icl::rt {

/// Edge-preserving bilateral filter for denoising path-traced output.
/// strength: 0.0–1.0, controls the range sigma (how aggressively similar
/// colors are blended). Higher = more denoising but more detail loss.
void denoiseBilateral(const core::Img8u &src, core::Img8u &dst,
                      float strength = 0.5f);

/// À-Trous wavelet filter — hierarchical edge-preserving denoiser.
/// Applies a 5×5 B3-spline wavelet kernel at 5 increasing step sizes
/// (1, 2, 4, 8, 16), each pass guided by an edge-stopping function.
/// strength: 0.0–1.0, controls the edge-stopping threshold (sigma_c).
/// Higher = stronger denoising.
void denoiseATrous(const core::Img8u &src, core::Img8u &dst,
                   float strength = 0.5f);

} // namespace icl::rt
