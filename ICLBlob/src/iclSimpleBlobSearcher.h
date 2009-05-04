#ifndef ICL_SIMPLE_BLOB_SEARCHER_H
#define ICL_SIMPLE_BLOB_SEARCHER_H

#include <iclRegion.h>
#include <iclColor.h>
#include <iclRange.h>
namespace icl{

  class SimpleBlobSearcher{
    public:
    struct Blob{
      Blob():region(0),refColorIndex(-1){}
      Blob(const Region *region, const Color &refColor, int refColorIndex);
      const Region *region;
      Color refColor;
      int refColorIndex;
    };

    SimpleBlobSearcher();
    ~SimpleBlobSearcher();

    void add(const Color &color, 
             float thresh, 
             const Range32s &sizeRange);

    void remove(int index);

    
    /// ...
    /** no ROI support yet!*/
    const std::vector<Blob> &detect(const Img8u &image);
    
    private:
    class Data;
    Data *m_data;
  };

  
}

#endif
