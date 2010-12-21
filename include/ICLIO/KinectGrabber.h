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

#ifdef HAVE_LIBFREENECT

namespace icl{
  /// Special Grabber implementation for Microsoft's Kinect Device
  /** This class implements ICL's Grabber interface for Microsofts Kinect
      Device. Internally, it uses libfreenect to access the device. */
  struct KinectGrabber : public Grabber{
    enum Mode{
      GRAB_RGB_IMAGE,   //!< grabs rgb images form the kinects rgb camera
      GRAB_BAYER_IMAGE, //!< not supported yet
      GRAB_DEPTH_IMAGE, //!< grabs the depth image from kinect
      GRAB_IR_IMAGE,    //!< grabs the kinects IR-image (not supported)
    };
    
    /// returns a list of attached kinect devices
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

    KinectGrabber(Mode mode = GRAB_DEPTH_IMAGE, int deviceID=0) throw (ICLException);

    /// Destructor
    ~KinectGrabber();
    
    /// grabs a new image
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);

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
