// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLCore/Img.h>
#include "RaytracerTypes.h"
#include <vector>

namespace icl::rt {

/// Persistent state for SVGF temporal denoiser.
/// Backends hold one instance; pass it to denoiseSVGF() each frame.
struct SVGFState {
  int width = 0, height = 0;

  // Previous frame filtered output (float [0,1])
  std::vector<float> prevR, prevG, prevB;

  // Previous frame G-buffers
  std::vector<float> prevDepth;
  std::vector<float> prevNX, prevNY, prevNZ;
  std::vector<int32_t> prevMeshId;

  // Temporal accumulation: luminance moments for variance estimation
  std::vector<float> moment1; // mean luminance
  std::vector<float> moment2; // mean of squared luminance
  std::vector<int> historyLen; // per-pixel history count

  RTMat4 prevViewProj{};
  RTFloat3 prevCamPos{};
  bool hasPrevFrame = false;

  void reset() { hasPrevFrame = false; width = height = 0; }
};

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

/// SVGF denoiser — spatiotemporal variance-guided filtering.
/// Requires G-buffer data (depth, normals) and camera parameters.
/// The SVGFState must persist across frames (held by the backend).
void denoiseSVGF(const core::Img8u &src, core::Img8u &dst,
                 const float *depth,
                 const float *normalX, const float *normalY, const float *normalZ,
                 const float *reflectivity,
                 const RTRayGenParams &camera,
                 SVGFState &state,
                 float strength = 0.5f);

} // namespace icl::rt
