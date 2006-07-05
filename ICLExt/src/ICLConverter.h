#ifndef TDIICL_IMAGECONVERTER_H
#define TDIICL_IMAGECONVERTER_H

#include <ICL.h>

namespace icl{
  /// Color Converter Object
  /**
  The IclConverter wrapps color conversion function from iclcc.h
  into an Object, which contains all necessary Buffers for converting images.
  In addition to that it brings the functionality for scaling images.
  Scaling and converting do not have to debar each other - the IclConverter
  is able to scale images internally, or to convert them internally into
  another depth before converting.
  */
  class IclConverter{
    public:
    
    /// Base constructor
    IclConverter();

    /// Destructors
    ~IclConverter();

    /// conversion function
    /**
    Although this function looks like the iclcc function located in the 
    iclcc.h,  it brings some additional functionalities (see class description).
    @param poDst destination image
    @param poSrc source image
    */
    void convert(ICLBase *poDst, ICLBase *poSrc);

    private:
    ICLBase *m_poDepthBuffer,*m_poSizeBuffer;
  };
}

#endif
