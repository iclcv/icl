#include "ICLCore.h"

using namespace ICL;

int iclGetChannelsOfFormat(iclformat eFormat)
{
  switch (eFormat)
    {
      case formatRGB:
      case formatHLS:
      case formatLAB:
      case formatYUV:
        return 3;
        break;
        
      case formatGray:
        return 1;
        break;
        
      case formatMatrix:
        return 1;
        break;
        
      default:
        return 1;
    }
}

