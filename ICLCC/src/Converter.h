#ifndef ICLCONVERTER_H
#define ICLCONVERTER_H

#include <Img.h>

namespace icl{
  /// General Image Converter
  /**
  The Converter wraps and summarizes all image conversion routines,
  including depth change, scaling and color conversion. It provides all
  neccessary buffers to do several of these changes in series. Simply
  provide the desired output format to poDst of the convert function,
  and this method will select the appropriate conversion steps.
  */
  class Converter {
    public:
    
    /// constructor
    Converter(bool bROIOnly=false);

    /// destructor
    ~Converter();

    /// image conversion function
    /**
    Although this function looks like the iclcc function located in the 
    iclcc.h,  it brings some additional functionalities (see class description).
    @param poDst destination image
    @param poSrc source image
    */
    void convert(ImgBase *poDst, ImgBase *poSrc);

    private:
    ImgBase *m_poDepthBuffer,*m_poSizeBuffer;
    bool  m_bROIOnly;
  };
}

#endif
