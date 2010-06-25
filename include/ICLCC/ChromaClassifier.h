/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCC/ChromaClassifier.h                       **
** Module : ICLCC                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_CHROMA_CLASSIFIER_H
#define ICL_CHROMA_CLASSIFIER_H

#include <ICLCore/Types.h>
#include <ICLCC/Parable.h>

namespace icl{
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
}

#endif
