#ifndef ICL_CHROMA_AND_RGB_CLASSIFIER_H
#define ICL_CHROMA_AND_RGB_CLASSIFIER_H

#include "iclChromaClassifier.h"

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
