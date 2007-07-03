#ifndef ICL_DC_1394_CAMERA_H
#define ICL_DC_1394_CAMERA_H

#include <iclSmartPtr.h>

struct dc1394camera_t;

namespace icl{
  
  class DC1394Camera {
    public:
    
    private:
    dc1394camera_t *m_poHandle;
  };
  
}

#endif
