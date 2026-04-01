// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// IPP backend for AffineOp — currently disabled.
//
// Old API (removed in oneAPI 2022+):
//   ippiWarpAffine_8u_C1R, ippiWarpAffine_32f_C1R
//
// Modern IPP replacement:
//   ippiWarpAffineNearest / ippiWarpAffineLinear + spec init
//
// Accelerate alternative:
//   vImageAffineWarp_Planar8, vImageAffineWarp_PlanarF
//   (see claude.insights/accelerate-ipp-mapping.md)
