#ifndef ICL_IMAGE_STATISTICS_H
#define ICL_IMAGE_STATISTICS_H

#include <vector>
#include <iclTypes.h>
#include <iclImgParams.h>
#include <iclRange.h>

namespace icl{
  
  struct ImageStatistics{
    ImgParams params;
    depth d;
    std::vector<Range64f> ranges;
    std::vector<std::vector<int> > histos;
    bool isNull;
  };
}

#endif
