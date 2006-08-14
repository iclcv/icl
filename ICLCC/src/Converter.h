#ifndef ICLCONVERTER_H
#define ICLCONVERTER_H

#include <Img.h>

namespace icl{
  /// Color Converter Object
  /**
  The ImgConverter wrapps color conversion function from iclcc.h
  into an Object, which contains all necessary Buffers for converting images.
  In addition to that it brings the functionality for scaling images.
  Scaling and converting do not have to debar each other - the Converter
  is able to scale images internally, or to convert them internally into
  another depth before converting.
  */
  class Converter{
    public:
    
    /// Base constructor
    Converter();

    /// Destructors
    ~Converter();

    /// conversion function
    /**
    Although this function looks like the iclcc function located in the 
    iclcc.h,  it brings some additional functionalities (see class description).
    @param poDst destination image
    @param poSrc source image
    */
    void convert(ImgI *poDst, ImgI *poSrc);

    private:
    ImgI *m_poDepthBuffer,*m_poSizeBuffer;
  };
}

#endif
