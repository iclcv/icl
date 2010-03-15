/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLCC/ChromaAndRGBClassifier.h                 **
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
*********************************************************************/

#ifndef ICL_CHROMA_AND_RGB_CLASSIFIER_H
#define ICL_CHROMA_AND_RGB_CLASSIFIER_H

#include <ICLCC/ChromaClassifier.h>

namespace icl{
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
}

#endif
