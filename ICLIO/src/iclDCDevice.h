#ifndef ICL_DC_DEVICE_H
#define ICL_DC_DEVICE_H

#include "iclDC.h"
#include <string>
#include <vector>
#include <iclTypes.h>

namespace icl{
  /** \cond */
  class DCGrabber;
  /** \endcond */
  
  /// Device struct, used by the DCGrabber class to identify devices
  class DCDevice{
    public:
    
    static const DCDevice null;
    
    /// DCDevices may only be created by the DCGrabbers private function
    friend class icl::DCGrabber;
    
    /// Internally used Mode struct (combination of videomode and framerate)
    struct Mode{
      /// creates a new Mode with given videomode and framerate
      Mode(dc1394video_mode_t vm, dc1394framerate_t fr):
        videomode(vm),framerate(fr){}
      
      /// creates a new Mode by a given string representation
      /** syntax: videomode@framerate */
      Mode(const std::string &stringRepr);
      
      /// returns a string representation of the mode
      /** syntax: videomode@framerate */
      std::string toString() const;

      /// returns whether the given camera supports this mode or not
      bool supportedBy(dc1394camera_t *cam) const;

      /// corresponding videomode
      dc1394video_mode_t videomode;
      
      /// corresponding framerate;
      dc1394framerate_t framerate;
    };
    
    /// returns the camera, which is associated with this device (fixed)
    dc1394camera_t *getCam(){ return m_poCam; }
    
    /// returns a list of supported modes for this device
    std::vector<Mode> getModes() const ;

    /// returns the vendor id string from the wrapped camera or "null" if the device is null
    std::string getVendorID() const;
    
    /// returns the model id string from the wrapped camera or "null" if the device is null
    std::string getModelID() const;
    
    /// returns the IEEE1394 Port of the device
    icl32s getPort() const;

    /// returns the IEEE1394 Node of the device
    icl16s getNode() const;
    
    /// returns wheather the device is associated to a dc-camera
    bool isNull() const { return m_poCam == 0; }
    
    private:    
    /// Creates a new device (pivate; called by DCGrabber::getDeviceList())
    DCDevice(dc1394camera_t *cam):m_poCam(cam){}
    
    /// associated camera (libdc stays the owner of the pointer)
    dc1394camera_t *m_poCam;
    
  };
}

#endif
