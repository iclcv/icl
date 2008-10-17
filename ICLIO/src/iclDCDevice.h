#ifndef ICL_DC_DEVICE_H
#define ICL_DC_DEVICE_H

#include "iclDC.h"
#include <string>
#include <vector>
#include <iclTypes.h>
#include <iclSize.h>

namespace icl{
  /** \cond */
  class DCGrabber;
  namespace dc{
    class DCGrabberThread;
  }
  /** \endcond */
  
  /// Device struct, used by the DCGrabber class to identify devices \ingroup DC_G
  class DCDevice{
    public:

    /// Enumeration of supported cameras
    enum CameraTypeID{
      pointGreyFire_FlyMVMono,
      pointGreyFire_FlyMVColor,
      sony_DFW_VL500_2_30,
      apple_ISight,
      fireI_1_2,
      imagingSource_DFx_21BF04,
      unknownCameraType
    };
    
    /// translates a camera id type into a string
    static std::string translate(CameraTypeID id);    

    /// translates a string into a camera id type
    static CameraTypeID translate(const std::string &name);

    /// estimates the camera id type of a given camera 
    static CameraTypeID estimateCameraType(dc1394camera_t *m_poCam);

    /// static null device (m_poCam is null)
    static const DCDevice null;
    
    /// DCDevices may only be created by the DCGrabbers private function
    friend class icl::DCGrabber;

    /// DCDevices may only be created by the DCGrabbers private function
    friend class icl::dc::DCGrabberThread;
    
    /// save version to call dc1394_reset_bus (after call all other cams become useless)
    static void dc1394_reset_bus(bool verbose=false);
    
    /// Internally used Mode struct (combination of videomode and framerate)
    struct Mode{
      /// creates a new Mode with given videomode and framerate
      Mode(dc1394video_mode_t vm, dc1394framerate_t fr):
        videomode(vm),framerate(fr){}
      
      /// creates a new Mode by a given string representation
      /** syntax: videomode\@framerate */
      Mode(const std::string &stringRepr);

      /// create a new Mode by given cam
      Mode(dc1394camera_t *cam);
      
      /// returns a string representation of the mode
      /** syntax: videomode\@framerate */
      std::string toString() const;

      /// returns whether the given camera supports this mode or not
      bool supportedBy(dc1394camera_t *cam) const;

      /// corresponding videomode
      dc1394video_mode_t videomode;
      
      /// corresponding framerate;
      dc1394framerate_t framerate;
      
      /// compares to modes
      bool operator==(const Mode &m) const{ return videomode == m.videomode && framerate == m.framerate; }
      
      /// compares to modes [!=  complies !(==)]
      bool operator!=(const Mode &m) const{ return !((*this)==m);}
    };
    
    /// returns the camera, which is associated with this device (fixed)
    dc1394camera_t *getCam(){ return m_poCam; }

    /// returns the current mode
    Mode getMode() const{ return Mode(m_poCam); }
    
    /// returns a list of supported modes for this device
    std::vector<Mode> getModes() const ;

    /// returns the vendor id string from the wrapped camera or "null" if the device is null
    std::string getVendorID() const;
    
    /// returns the model id string from the wrapped camera or "null" if the device is null
    std::string getModelID() const;
    
    /// returns the devices Global Unique ID
    uint64_t getGUID() const;

    /// returns the IEEE1394 Unit of the device
    icl32s getUnit() const;

    /// returns the IEEE1394 UnitSpecID of the device
    icl32s getUnitSpecID() const;
    
    /// returns wheather the device is associated to a dc-camera
    bool isNull() const { return m_poCam == 0; }
    
    /// shows some device information 
    void show(const std::string &title="DCDevice") const;
       
    /// returns whether the Device supports a given icl-format
    bool supports(format fmt) const;

    /// returns whether the Device supports a given icl-format
    bool supports(const Size &size) const;    

    /// returns whether the Device supports a given mode
    bool supports(const Mode &mode) const;
    
    /// returns whether images need by decoding
    bool needsBayerDecoding() const;
       
    /// returns the bayer-filter layout (for the current set format)
    dc1394color_filter_t getBayerFilterLayout() const;
    
    /// returns wheter a given feature is available on this camera
    bool isFeatureAvailable(const std::string &feature) const;

    /// returns a list of all supported features
    std::vector<std::string> getFeatures() const;
    
    /// returns the type of the given feature ("" for unsupported features)
    std::string getFeatureType(const std::string &feature) const;
    
    /// returns the feature information depending on the feature type
    std::string getFeatureInfo(const std::string &feature) const;

    /// returns the current value of the given feature
    std::string getFeatureValue(const std::string &feature) const;
    
    /// sets the current value of the given feature
    void setFeatureValue(const std::string &feature, const std::string &value);

    /// sets the cameras iso speed
    /** @see icl::dc::set_iso_speed(int) */
    void setISOSpeed(int mbits);
    
    private:    
    /// Creates a new device (pivate; called by DCGrabber::getDeviceList())
    DCDevice(dc1394camera_t *cam):
    m_poCam(cam),m_eCameraTypeID(estimateCameraType(cam)){}

    /// sets the current mode of this device
    /** This function may only be called by the DCGrabber*/
    void setMode(const Mode &mode);

    /// resets the camera internally
    /** This function may only be called by the DCGrabber*/
    void reset() { if(!isNull()) dc1394_camera_reset(m_poCam); }
    // PRE7: void reset() { if(!isNull()) dc1394_reset_camera(m_poCam); }
    
    /// associated camera (libdc stays the owner of the pointer)
    dc1394camera_t *m_poCam;

    /// once estimated this flag is used to identify the current camery type
    CameraTypeID m_eCameraTypeID;
    
  };
}

#endif
