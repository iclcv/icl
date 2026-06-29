// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/NativeCheckerboardDetector.h>
#include <icl/cv/CheckerboardGrid.h>
#include <utility>

namespace icl{
  namespace cv{

    CheckerboardDetector::Result
    NativeCheckerboardDetector::detect(const core::Img8u &image, const Hints &){
      const std::vector<CornerSeed> seeds =
        CheckerboardSaddleDetector(m_saddle).detect(image);
      Result res;
      CheckerboardGrid g = recoverCheckerboardGrid(seeds, &image);   // guided
      if(m_cleanup && !g.empty())
        g = refineCheckerboardGrid(g, seeds, &image);   // homography + Hungarian
      if(!g.empty()) res.boards.push_back(std::move(g));
      return res;
    }

  } // namespace cv
} // namespace icl
