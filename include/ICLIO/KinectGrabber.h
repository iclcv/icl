/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/KinectGrabber.h                          **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Size.h>

#include <utility>
#include <map>

#ifdef HAVE_LIBFREENECT

namespace icl {
  /// Special Grabber implementation for Microsoft's Kinect Device
  /** This class implements ICL's Grabber interface for Microsofts Kinect
      Device. Internally, it uses libfreenect to access the device. 
      
      \section GRA Which grabbers can be instantiated simultaneously
      
      The Kinect grabber class implements a abstraction layer around the
      wrapped libfreenect. However, libfreenect does not allow for grabbing
      color and IR images simultaneously from one device (even though
      the IR image is actually grabbed from the depth camera). Therefore,
      the <em>logical</em> source of the IR images is the color camera.
      
      \section DEVEL Developers notes
      Raw Depth Value should be interpreted as integers between 0 and 2047,
      anything above 2047 equals a distance of 0.
  */
  struct KinectGrabber : public Grabber {

    /// Capturing format
    enum Format {
      GRAB_RGB_IMAGE, //!< grabs rgb images form the kinects rgb camera
      GRAB_BAYER_IMAGE,
      GRAB_IR_IMAGE, //!< grabs the kinects IR-image
      GRAB_YUV_IMAGE,
      GRAB_DEPTH_IMAGE, //!< grabs the depth image from kinect
      GRAB_DEPTH_MM_IMAGE, //!< grabs the depth image from kinect
      GRAB_DEPTH_CM_IMAGE, //!< grabs the depth image from kinect
      GRAB_DEPTH_M_IMAGE, //!< grabs the depth image from kinect
   };

    /// capturing resolution
    enum Resolution {
      RESOLUTION_LOW,    //!< QVGA resolution (doesn't work)
      RESOLUTION_MEDIUM, //!< VGA (only for IR-Grabber 640x488 is used)
      RESOLUTION_HIGH    //!< SXGA resolution (not for the depth camera)
    };

    /// combination of a Format and Resolution
    typedef std::pair<Format, Resolution> Mode;
    
    /// returns a list of attached kinect devices
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);
    
    
    /// Creates a new grabber instance with given format and device ID
    /** By default, the */
    KinectGrabber(Format format = GRAB_DEPTH_IMAGE, int deviceID=0) throw (ICLException);

    /// Destructor
    ~KinectGrabber();
    
    /// grabs a new image
    virtual const ImgBase* acquireImage();

    /// get type of property 
    virtual std::string getType(const std::string &name);
    
    /// get information of a properties valid values
    virtual std::string getInfo(const std::string &name);
    
    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name);

    /// Returns whether this property may be changed internally
    virtual int isVolatile(const std::string &propertyName);

    /// Sets a specific property value
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// returns a list of properties, that can be set using setProperty
    virtual std::vector<std::string> getPropertyList();

    protected:
    struct Impl; //!< internal hidden implementation class
    Impl *m_impl;//!< hidden internal data
  };
}

#endif
