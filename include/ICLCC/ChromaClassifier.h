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
