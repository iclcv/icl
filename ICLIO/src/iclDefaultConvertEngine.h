#ifndef ICL_DEFAULT_CONVERT_ENGINE_H
#define ICL_DEFUALT_CONVERT_ENGINE_H
#include "iclUnicapConvertEngine.h"
#include <vector>

namespace icl{
  

  /// Basic implemenation for the UnicapCovertEngine \ingroup UNICAP_G
  /** providing abilities to convert the following formats:
      - Y444 YUV 4-4-4 interleaved (no sub sampling of U and V channel \n
        (C++ and IPP implementation)
      - UYVY YUV 4-2-2 interleaved (half size subsampling of U and V channel)\n
        (C++ and IPP implementation)
      - YUYV YUV 4-2-2 interleaved (half size subsampling of U and V channel)\n
        (C++ implementation only)
      - Y411 YUV 4-1-1 interleaved (quater size subsampling of U and V channel)\n
        (C++ implementation only) 
      - Y800 8Bit Grab image (no conversion ; just a deep copy)
      
      This convert engine is appropriate to the following camera models:
      - Sony DFW-VL500 cam
      - Apple Computers, Inc., iSight
  **/
  class DefaultConvertEngine : public UnicapConvertEngine{
    public:
    /// Default constructor just passing the UnicapDevice argument to the parent constructor
    DefaultConvertEngine(UnicapDevice *device):UnicapConvertEngine(device){}
    
    /// Empty destructor
    virtual ~DefaultConvertEngine(){}
    
    /// Convertsion function
    /** \copydoc icl::UnicapConvertEngine::cvt(const icl8u*, const ImgParams&, depth, ImgBase**) */
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst);

    /// returns whether this engine is able to provide images with given params
    virtual bool isAbleToProvideParams(const ImgParams &desiredParams, depth desiredDepth) const;

    private:
    /// internal used data buffer for conversion
    std::vector<icl8u> m_oCvtBuf;
  };
}

#endif
