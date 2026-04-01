// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <ICLCore/Parable.h>

namespace icl{
  namespace core{
    /// Classifier interface using RG-chromaticity space and two parables \ingroup COMMON
    struct ChromaClassifier{
    public:
      /// classifies a given R-G-Pixel
      inline bool operator()(icl8u chromaR, icl8u chromaG) const{
        return parables[0](chromaR) > chromaG && parables[1](chromaR) < chromaG;
      }
      /// classifies a given r-g-b-Pixel
      inline bool operator()(icl8u r, icl8u g, icl8u b) const{
        int sum = r+g+b+1;
        return (*this)((255*r)/sum,(255*g)/sum);
      }
      /// Shows this classifier to std::out
      void show()const{
        std::cout << "chroma classifier:" << std::endl;
        parables[0].show();
        parables[1].show();
      }
      /// Used two parables
      //    ParableSet parables;
      Parable parables[2];
    };
  } // namespace core
}
