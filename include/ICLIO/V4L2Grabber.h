/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/V4L2Grabber.h                            **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_VIDEO_4_LINUX_2_GRABBER_H
#define ICL_VIDEO_4_LINUX_2_GRABBER_H

#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  
  /// The Video for Linux 2 Grabber uses the v4l2-api to access video capturing devices \ingroup GRABBER_G \ingroup V4L_G
  /** This grabber backend is usually used for USB-Webcams as well as for Grabber cards */
  class V4L2GrabberImpl : public Grabber{
    class Impl; //!< internal implementation
    Impl *impl; //!< internal data structure
    Mutex implMutex; //!< protects the impl which is reallocated when the format is changed
    public:
    
    /// create a new grabbers instance, with given device name (
    V4L2GrabberImpl(const std::string &device="/dev/video0");

    /// Destruktoer
    ~V4L2GrabberImpl();
    
    /// obtains the next image
    virtual const ImgBase *acquireImage();

    /// returns the device property list
    virtual std::vector<std::string> getPropertyList();    
    
    /// sets a specific property
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// returns a property's type
    virtual std::string getType(const std::string &name);

    /// returns information about possible property values
    virtual std::string getInfo(const std::string &name);
    
    /// returns a current property value
    virtual std::string getValue(const std::string &name);
    
    /// returns a properties 'volatileness' (this is acutally not supported by this Grabber type)
    virtual int isVolatile(const std::string &propertyName);
    
    /// returns a list of all supported video devices
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);
  };


  /// Video4Linux2 based grabber \ingroup GRABBER_G
  /** for more details: @see V4L2GrabberImpl */
  class V4L2Grabber : public GrabberHandle<V4L2GrabberImpl>{
    public:
    /// returns a list of available pwc devices 
    /** @see V4L2GrabberImpl for more details*/
    static inline const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan){
      return V4L2GrabberImpl::getDeviceList(rescan);
    }
    
    /// creates a new V4L2Grabber instance
    /** @see V4L2GrabberImpl for more details */
    inline V4L2Grabber(const std::string &device="/dev/video0"){
      if(isNew(device)){
        initialize(new V4L2GrabberImpl(device), device);
      }else{
        initialize(device);
      }
    }
    /// empty constructor (initialize late using init())
    /** @see V4L2GrabberImpl for more details */
    inline V4L2Grabber(){}
  };

}

#endif
