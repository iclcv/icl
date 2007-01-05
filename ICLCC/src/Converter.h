#ifndef ICLCONVERTER_H
#define ICLCONVERTER_H

#include <Img.h>

namespace icl{
  /// General Image Converter
  /**
  The Converter wraps and summarizes all image conversion routines,
  including depth change, scaling and color conversion. It provides all
  neccessary buffers to do several of these changes in series. Simply
  provide the desired output format to dst of the apply function,
  and this method will select the appropriate conversion steps.
  */

  class Converter {
    public:
    
    /// constructor
    /** @param applyToROIOnly if true, the source images ROI only is
                              used instead of the whole src image.
        @param useShallowCopy if true and if no operation at all is needed
                              to convert the source data into the destination
                              data - in other words, if a shallow copy
                              is possible, the converter will do this.
                              <b>Note:</b> Only in this case, the destination 
                              image shares its data pointers with the source
                              image, which may cause errors.
    */
    Converter(bool applyToROIOnly=false, bool useShallowCopy=false);
    

    
    /// creates a converter, and converts the srcImage to dstImage imediately
    Converter(ImgBase *srcImage, ImgBase *dstImage, bool applyToROIOnly=false, bool useShallowCopy=false);

    /// destructor
    ~Converter();
    
    /// image conversion function
    /**
    Although this function looks like the iclcc function located in the 
    iclcc.h,  it brings some additional functionalities (see class description).
    @param src source image
    @param dst destination image
    */
    //    void convert(ImgBase *poDst, ImgBase *poSrc);
    void apply(ImgBase *src, ImgBase *dst);

    /// sets up the converter to apply operations on the source images roi only
    /** @see Converter(bool,bool) */
    void setApplyToROIOnly(bool applyToROIOnly){ m_bROIOnly = applyToROIOnly; }

    /// sets up the converter to perform a shallow copy from src to dst if possible
    /** @see Converter(bool,bool) */
    void setUseShallowCopy(bool useShallowCopy) { m_bUseShallowCopy = useShallowCopy; }
    private:

    /// internally used conversion function
    /** This function wraps the icl::cc function and optimizes its 
        performance by using the Converter objects internally hold
        color conversion buffer for "emulated" color conversions.
        (E.g. HLStoYUV, is emulated by HLSToRGB followed by RGBToYUV.
        icl::cc stores the result of HLSToRGB in a temporarily 
        allocated and released image buffer. The Converter uses an
        persisten buffer of the Converter object.) This will speed
        up cross format conversions in looped applications.
    */
    void cc(ImgBase *src, ImgBase *dst);
    
    ImgBase *m_poBuffer,*m_poCCBuffer;
    bool  m_bROIOnly, m_bUseShallowCopy;
  };
}

#endif
