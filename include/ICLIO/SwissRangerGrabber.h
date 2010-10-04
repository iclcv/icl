/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/SwissRangerGrabber.h                     **
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

#ifndef ICL_SWISSRANGER_GRABBER_H
#define ICL_SWISSRANGER_GRABBER_H

#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  
  
  /// Grabber-Implementation for the SwissRanger time-of-flight camera
  class SwissRangerGrabberImpl : public Grabber{
    public:
    /// Internally used data-class
    class SwissRanger;
    
    /// Create interface to device with given serial number:
    /** @param if 0 -> automatic select\n
               if < 0 open selection dialog (windows)
               if > 0 specify serial number of device
    */
    SwissRangerGrabberImpl(int serialNumber=0, depth bufferDepth=depth32f, int pickChannel=-1) 
      throw (ICLException);

    /// Destructor
    ~SwissRangerGrabberImpl();
    
    /// returns a list of all found devices
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);
    
    /// grab an undistorted image 
    const ImgBase *grabUD(ImgBase **dst=0);

    /** Sets a property */
    virtual void setProperty(const std::string &property, const std::string &value);
     
    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();

    /** returs the type of a property */
    virtual std::string getType(const std::string &name);

    /** returs information for a property */
    virtual std::string getInfo(const std::string &name);

    /** returs property value */
    virtual std::string getValue(const std::string &name);
    
    /// Internally used utility function, that might be interesting elsewhere
    static float getMaxRangeMM(const std::string &modulationFreq) throw (ICLException);


    private:
    /// utility function
    float getMaxRangeVal() const;

    /// Internal data
    SwissRanger *m_sr; 
    
    /// Internally used mutex locks grabbing and setting of properties
    Mutex m_mutex;
  };

  
  /// SwissRanger grabber using the libMesaSR library \ingroup GRABBER_G
  /** for more details: @see SwissRangerGrabberImpl */
  class SwissRangerGrabber : public GrabberHandle<SwissRangerGrabberImpl>{

    public:
    /// ID-creation function for the GrabberHandles internal unique ID
    static inline std::string create_id(int dev){
      return std::string("device-")+str(dev);
    }

    /// Constructor 
    /** see SwissRangerGrabberImpl::SwissRangerGrabberImpl for details */
    SwissRangerGrabber(int serialNumber=0, depth bufferDepth=depth32f, int pickChannel=-1) throw (ICLException){
      std::string id = create_id(serialNumber);
      if(isNew(id)){
        initialize(new SwissRangerGrabberImpl(serialNumber,bufferDepth,pickChannel),id);
      }else{
        initialize(id);
      }
    }
    
    /// returns a list of all found devices
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan){
      return SwissRangerGrabberImpl::getDeviceList(rescan);
    }

    /// Utility function
    static inline float getMaxRangeMM(const std::string &modulationFreq) throw (ICLException){
      return SwissRangerGrabberImpl::getMaxRangeMM(modulationFreq);
    }
  };
  
}

#endif
