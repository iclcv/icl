// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// IPP backend for LUTOp — currently disabled.
//
// Old API (signature changed in oneAPI 2022+):
//   ippiReduceBits_8u_C1R (added noise parameter)
//
// Modern IPP replacement:
//   ippiReduceBits_8u_C1R with updated signature (noise param added)
//
// Accelerate alternative:
//   Partial — vImageConvert_*_dithered (dithering only, not general LUT)
//   (see claude.insights/accelerate-ipp-mapping.md)
