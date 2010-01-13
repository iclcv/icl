#ifndef ICL_CHROMA_CLASSIFIER_IO_H
#define ICL_CHROMA_CLASSIFIER_IO_H

#include <ICLCC/Parable.h>
#include <ICLCC/ChromaClassifier.h>
#include <ICLCC/ChromaAndRGBClassifier.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  class ChromaClassifierIO{
    public:
    static void save(const ChromaClassifier &cc, 
                     const std::string &filename, 
                     const std::string &name="chroma-classifier");

    static void save(const ChromaAndRGBClassifier &carc, 
                     const std::string &filename);
    
    static ChromaClassifier load(const std::string &filename, 
                                 const std::string &name="chroma-classifier");
    
    static ChromaAndRGBClassifier loadRGB(const std::string &filename);
  };
}

#endif
