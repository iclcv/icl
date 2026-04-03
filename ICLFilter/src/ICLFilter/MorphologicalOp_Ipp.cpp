// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// IPP backend for MorphologicalOp — not available.
//
// IPP 2022.3 removed the general morphology border API
// (ippiMorphologyBorderGetSize/Init, ippiDilateBorder/ErodeBorder).
// Only ippiDilate3x3_64f_C1R / ippiErode3x3_64f_C1R remain.
//
// The C++ backend handles all morphological operations.
// On macOS, the Accelerate backend provides vImageDilate/Erode for 8u/32f.
