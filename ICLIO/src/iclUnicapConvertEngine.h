#ifndef ICL_Unicap_CONVERT_ENGINE_H
#define ICL_Unicap_CONVERT_ENGINE_H
#include <iclTypes.h>
#include <iclImgParams.h>
namespace icl{
  /** \cond */
  class UnicapDevice;
  /** \endcond */
  
  /// Interface class for unicap convert engines \ingroup UNICAP_G
  struct UnicapConvertEngine{
    
    /// Default constructor (just saving the device)
    UnicapConvertEngine(UnicapDevice *device):m_poDevice(device){}
    
    /// Empty destructor
    virtual ~UnicapConvertEngine(){}
    
    /// Conversion function
    /** This function must be implemented for each specific convert engine class. The raw data must be converted
        into the destination image. If possible, the desired params and depth should be regarded. If not, the 
        parent UnicapGrabber will perform the conversion using a Converter 
        @param rawData incomming raw data of current image
        @param desiredParams params (size, format,...) for the result image
        @param desiredDepth depth for the result image
        @param ppoDst destination image pointer-pointer. This must be adapted to the image params that are actually
                      produce by this converter 
    **/
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst)=0;

    /// returns whether this engine is able to provide given paramters
    virtual bool isAbleToProvideParams(const ImgParams &desiredParams, depth desiredDepth) const = 0;
    
    /// converts given frame to native sized rgb depth8u image
    virtual void cvtNative(const icl8u *rawData, ImgBase **ppoDst);
    protected:
    
    /// internal storage of the associated UnicapDevice
    UnicapDevice *m_poDevice;
  };
}

#endif
