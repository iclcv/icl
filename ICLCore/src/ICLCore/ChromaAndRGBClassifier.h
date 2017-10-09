/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ChromaAndRGBClassifier.h           **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/ChromaClassifier.h>

namespace icl{
  namespace core{
    /// Combination classifier using RG-chroma. as well as RGB-thresholded reference color classifiation \ingroup COMMON
    struct ChromaAndRGBClassifier{
      /// classifies a given r-g-b-Pixel
      /**The function is:
          \code
          bool is_pixel_skin_colored(int r, int g, int b, ChromaClassifier c, int refcol[3], int threshold[3]){
          return c(r,g,b)
          && abs(r-refcol[0])<threshold[0]
          && abs(g-refcol[1])<threshold[1]
          && abs(b-refcol[2])<threshold[2];
          }
          \endcode
          */
      inline bool operator()(icl8u r, icl8u g, icl8u b) const{
        return c(r,g,b) && ::abs(r-ref[0])<thresh[0] && ::abs(g-ref[1])<thresh[1] && ::abs(b-ref[2])<thresh[2];
      }
      /// wrapped ChromaClassifier
      ChromaClassifier c;

      /// r-g-b reference color
      icl8u ref[3];

      /// r-g-b threshold
      icl8u thresh[3];

      /// shows this classifier to std::out
      void show()const{
        printf("Combi-Classifier\n");
        c.show();
        printf("reference color:  %d %d %d \n",ref[0],ref[1],ref[2]);
        printf("color thresholds: %d %d %d \n",thresh[0],thresh[1],thresh[2]);
      }
    };
  } // namespace core
}
