// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// IPP backend for MorphologicalOp — currently disabled.
//
// Old API (removed in oneAPI 2022+):
//   ippiMorphologyInitAlloc_*, ippiMorphAdvInitAlloc_*
//   ippiDilate/Erode_*_C1R, ippiDilate/Erode3x3_*_C1R
//   ippiDilateBorderReplicate_*, ippiErodeBorderReplicate_*
//   ippiMorphOpenBorder_*, ippiMorphCloseBorder_*
//   ippiMorphTophatBorder_*, ippiMorphBlackhatBorder_*, ippiMorphGradientBorder_*
//   Stateful: IppiMorphState, IppiMorphAdvState (8u, 32f)
//
// Modern IPP replacement:
//   ippiMorphInit_* + spec-based API with ippiDilate_*_C1R_L etc.
//
// Accelerate alternative:
//   Partial — vImageErode/Dilate_Planar8/PlanarF (basic only)
//   No composite ops (open/close/gradient/tophat/blackhat).
//   (see claude.insights/accelerate-ipp-mapping.md)
