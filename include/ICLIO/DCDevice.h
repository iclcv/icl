#ifndef ICL_DC_DEVICE_H
#define ICL_DC_DEVICE_H

#include <ICLIO/DC.h>
#include <string>
#include <vector>
#include <ICLCore/Types.h>
#include <ICLUtils/Size.h>

namespace icl{
  /** \cond */
  class DCGrabberImpl;
  namespace dc{
    class DCGrabberThread;
  }
  /** \endcond */
  
  /// Device struct, used by the DCGrabber class to identify devices \ingroup DC_G
  class DCDevice{
    public:

    /// returns a type ID specifier (vendor+" -- "+model)
    static std::string getTypeID(const std::string &model, const std::string &vendor);
    
    /// returns getTypeID(cam->model,cam->vendor)
    static std::string getTypeID(const dc1394camera_t *cam);
    
    /// returns an instances type ID (see also static functions)
    /** @see getTypeID(const std::string&,const std::string &)*/
    std::string getTypeID() const{ return getTypeID(m_poCam); }
    
    /// static null device (m_poCam is null)
    static const DCDevice null;
    
    /// DCDevices may only be created by the DCGrabbers private function
    friend class icl::DCGrabberImpl;

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
    dc1394camera_t *getCam() const { return m_poCam; }

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

    /// returns a unique string identifier for this device
    /** currently: getModelID + "-" +getGUID() */
    std::string getUniqueStringIdentifier() const;
    
    /// returns wheather the device is associated to a dc-camera
    bool isNull() const { return m_poCam == 0; }
    
    /// shows some device information 
    void show(const std::string &title="DCDevice") const;
       
    /// returns whether the Device supports a given mode
    bool supports(const Mode &mode) const;
    
    /// returns the cameras bayer filter layout
    /** @return bayer filter layout
                - 0 if no bayer filter is needed
                - 1 if the bayer filter layout is given by a camera feature
                - otherwise a valid dc1394color_filter_t value
    */
    dc1394color_filter_t getBayerFilterLayout() const;

    /// sets the cameras iso speed
    /** @see icl::dc::set_iso_speed(int) */
    void setISOSpeed(int mbits);
    
    private:    
    /// Creates a new device (pivate; called by DCGrabber::getDeviceList())
    DCDevice(dc1394camera_t *cam):
    m_poCam(cam){//,m_eCameraTypeID(estimateCameraType(cam)){
      estimateBayerFilterMode();
    }

    /// sets the current mode of this device
    /** This function may only be called by the DCGrabber*/
    void setMode(const Mode &mode);

    /// resets the camera internally
    /** This function may only be called by the DCGrabber*/
    void reset() { if(!isNull()) dc1394_camera_reset(m_poCam); }


    /// associated camera (libdc stays the owner of the pointer)
    dc1394camera_t *m_poCam;

    /// this function is called by the constructor 
    void estimateBayerFilterMode();

    enum BayerFilterMode{
      BF_RGGB = DC1394_COLOR_FILTER_RGGB,
      BF_GBRG = DC1394_COLOR_FILTER_GBRG,
      BF_GRBG = DC1394_COLOR_FILTER_GRBG,
      BF_BGGR = DC1394_COLOR_FILTER_BGGR,
      BF_NONE,
      BF_FROM_MODE,
      BF_FROM_FEATURE
    };

    /// once estimated this flag is used to identify the current camery type
    // CameraTypeID m_eCameraTypeID;
    BayerFilterMode m_eBayerFilterMode;
  };
}

#endif
