/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ChromaClassifier.h                 **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLCore/Types.h>
#include <ICLCore/Parable.h>

namespace icl{
  namespace core{
    /// Classifier interface using RG-chromaticity space and two parables \ingroup COMMON
    struct ICL_CORE_API ChromaClassifier{
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
