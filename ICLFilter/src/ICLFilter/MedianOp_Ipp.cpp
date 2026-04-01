// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// IPP backend for MedianOp — currently disabled.
//
// Old API (removed in oneAPI 2022+):
//   ippiFilterMedianCross_8u_C1R, ippiFilterMedianCross_16s_C1R
//   ippiFilterMedian_8u_C1R, ippiFilterMedian_16s_C1R
//
// Modern IPP replacement:
//   ippiFilterMedianBorder_*_C1R (requires border type + buffer management)
//
// Accelerate alternative:
//   None — no median filter in vImage. Keep C++ implementation.
//   (see claude.insights/accelerate-ipp-mapping.md)
