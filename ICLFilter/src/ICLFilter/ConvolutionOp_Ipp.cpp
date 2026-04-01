// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// IPP backend for ConvolutionOp — currently disabled.
//
// Old API (removed in oneAPI 2022+):
//   34 specializations: ippiFilterSobelHoriz/Vert_*, ippiFilterLaplace_*,
//   ippiFilterGauss_*, ippiFilter_*, ippiFilter32f_* (8u, 16s, 32f)
//
// Modern IPP replacement:
//   ippiFilterSobelBorder_*, ippiFilterGaussBorder_*, ippiFilterBorder_*
//   (border-based APIs with spec/buffer init)
//
// Accelerate alternative:
//   vImageConvolve_PlanarF, vImageConvolve_Planar8, separable variants
//   (see claude.insights/accelerate-ipp-mapping.md)
