/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/GenericGrabber.h                         **
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

#ifndef ICL_GENERIC_GRABBER_H
#define ICL_GENERIC_GRABBER_H

#include <ICLIO/Grabber.h>
#include <string>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Lockable.h>
#include <ICLUtils/ProgArg.h>

namespace icl {

  /// Common interface class for all grabbers \ingroup GRABBER_G
  /** The generic grabber provides an interface for a multi-platform
      compatible grabber.
      Image processing applications should use this Grabber 
      class. The GenericGrabber is also integrated with the 
      "camcfg"-GUI component (see icl::GUI)
  */
  class GenericGrabber: public Grabber{
    
    Grabber *m_poGrabber; //!< internally wrapped grabber instance
    
    std::string m_sType; //!< type of current grabber implementation
    
    mutable Mutex m_mutex; //! << internal protection for re-initialization
    public:
    
    /// Empty default constructor, which creates a null-instance
    /** null instances of grabbers can be adapted using the init-function*/
    GenericGrabber():m_poGrabber(0){}
    
    /// Initialized the grabber from given prog-arg 
    /** The progarg needs two sub-parameters */
    GenericGrabber(const ProgArg &pa) throw (ICLException);

    /// Create a generic grabber instance with given device priority list
    /** internally this function calls the init function immediately*/
    GenericGrabber(const std::string &devicePriorityList,
                   const std::string &params,
                   bool notifyErrors = true) throw (ICLException);
    
    /// initialization function to change/initialize the grabber back-end
    /** @param devicePriorityList Comma separated list of device tokens (no white spaces).
                                  something like "dc,pwc,file,unicap" with arbitrary order
                                  undesired devices can be left out. In particular you can 
                                  also give only a single desired device type e.g. "pwc".
                                  The following device types are supported:
                                  - <b>pwc</b> pwc grabber 
                                  - <b>v4l2</b> Video for Linux 2 based grabber
                                  - <b>dc</b> dc grabber 
                                  - <b>dc800</b> dc grabber but with 800MBit iso-speed 
                                  - <b>unicap</b> unicap grabber 
                                  - <b>file</b> file grabber
                                  - <b>demo</b> demo grabber (moving red spot)
                                  - <b>create</b> create grabber (create an image using ICL's create function)
                                  - <b>xcfp</b> xcf publisher grabber
                                  - <b>xcfs</b> xcf server grabber
                                  - <b>xcfm</b> xcf memory grabber
                                  - <b>mv</b> matrix vision grabber
                                  - <b>sr</b> SwissRanger camera (mesa-imaging)
                                  - <b>video</b> Xine based video grabber (grabbing videos frame by frame)
                                  - <b>cvcam</b> OpenCV based camera grabber (supporting video 4 linux devices)
                                  - <b>cvvideo</b> OpenCV based video grabber 
                                  - <b>sm</b> Qt-based Shared-Memory grabber (using QSharedMemoryInstance)
                                  - <b>myr</b> Uses Myrmex tactile input device as image source
                                  - <b>kinectd</b> Uses libfreenect to grab Microsoft-Kinect's depth images
                                  - <b>kinectc</b> Uses libfreenect to grab Microsoft-Kinect's rgb color images
                                  - <b>kinecti</b> Uses libfreenect to grab Microsoft-Kinect's IR images
                                  - <b>rsb</b> Robotics Service Bus Source 
        
        
        
        @param params comma separated device depend parameter list: e.g.
                                  "pwc=0,file=images//image*.ppm,dc=0" with self-explaining syntax\n
                                  Additionally, each token a=b can be extended by device property that are directly
                                  set after device instantiation. E.g. demo=0@size=QVGA@blob-red=128, instantiates
                                  a demo-grabber, where the two additionally given properties (size and blob-red) 
                                  are set immediately after grabber instantiation. By these means particularly a 
                                  grabber's format can be set in the grabber instantiation call. Furthermore, three
                                  special \@-tokens are possible: \@info (e.g. dc=0\@info) lists the 0th dc device's
                                  available properties. \@load=filename loads a given property filename directly. 
                                  \@udist=filename loads a given undistortion parameter filename directly and therefore
                                  makes the grabber grab undistorted images according to the undistortion parameters
                                  and model type (either 3 or 5 parameters) that is found in the given xml-file. 
                                  <b>todo fix this sentence according to the fixed application names</b>
                                  Please note, that valid xml-undistortion files can be created using the
                                  undistortion-calibration tools icl-opencvcamcalib-demo,
                                  icl-intrinsic-camera-calibration and icl-intrinsic-calibrator-demo.
                                  On the C++-level, this is only a minor advantage, since all these things can 
                                  also be achieved via function calls, however if you use the most recommended way
                                  for ICL-Grabber instantiation using ICL's program-argument evaluation framework,
                                  The GenericGrabber is instantiated using grabber.init(pa("-i")) which then allows
                                  the application user to set grabber parameters via addiation \@-options on the 
                                  command line: e.g.: "icl-camviewer -input dc 0\@size=VGA"
  
                                  Semantics:\n
                                  - pwc=device-index (int)
                                  - v4l2=device-name (e.g. "/dev/video0")
                                  - dc=device-index (int) or dc=UniqueID (string) 
                                    (the unique ID can be found with 'icl-cam-cfg d -list-devices-only')
                                  - dc800=device-index (int)
                                  - file=pattern (string)
                                  - unicap=device pattern (string) or device index (int)
                                  - demo=anything (not regarded)
                                  - create=image name (see also icl::TestImages::create)
                                  - xcfp=publisher's-stream-name (string)
                                  - xcfs=server-name (string) (currently method name is always "retreiveImage")
                                  - xcfm=memory-name (string) (currently image-xpath is always "//IMAGE")
                                  - mv=device-name (string)
                                  - sr=device-serial-number (-1 -> menu, 0 -> auto-select)
                                    <b>or</b>
                                    sr=NcC where N is the device numer as above, c is the character 'c' and C is
                                    the channel index to pick (0: depth-map, 1: confidence map, 2: intensity image
                                  - video=video-filename (string)
                                  - cvcam=camera index (0=first device,1=2nd device, ...) here, you can also use
                                    opencv's so called 'domain offsets': current values are: 
                                    - 100 MIL-drivers (proprietary)
                                    - 200 V4L,V4L2 and VFW, 
                                    - 300 Firewire, 
                                    - 400 TXYZ (proprietary)
                                    - 500 QuickTime
                                    - 600 Unicap
                                    - 700 Direct Show Video Input
                                    (e.g. device ID 301 selects the 2nd firewire device)
                                  - cvvideo=video-filename (string)
                                  - sm=Shared-memory-segment-id (string)
                                  - myr=deviceIndex (int) (the device index is used to create the /dev/videoX device)
                                  - kinectd=device-index (int) 
                                  - kinectc=device-index (int) 
                                  - kinecti=device-index (int) 
                                  - rsb=[comma-sep. transport-list=spread]:scope)

        @param notifiyErrors if set to false, no exception is thrown if no suitable device was found
    **/
    void init(const std::string &devicePriorityList,
              const std::string &params,
              bool notifyErrors = true) throw (ICLException);

    /// this method works just like the other init method
    void init(const ProgArg &pa) throw (ICLException);

    /// resets resource on given devices (e.g. firewire bus)
    static void resetBus(const std::string &deviceList="dc", bool verbose=false);
   
    /// return the actual grabber type
    std::string getType() const { 
      Mutex::Locker __lock(m_mutex);
      return m_sType; 
    }
    
    /// returns the wrapped grabber itself
    Grabber *getGrabber() const {
      Mutex::Locker __lock(m_mutex); 
      return m_poGrabber; 
    }
    
    /// Destructor
    virtual ~GenericGrabber(){
      //Mutex::Locker __lock(m_mutex);
      ICL_DELETE(m_poGrabber);
    }
    
    /// grabbing function
    virtual const ImgBase* acquireImage(){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),0);
      return m_poGrabber->acquireImage();
    }

    /// returns a list of all properties, that can be set
    virtual std::vector<std::string> getPropertyList(){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),std::vector<std::string>());
      return m_poGrabber->getPropertyList();
    }

    /// setting up properties of underlying grabber
    virtual void setProperty(const std::string &property, const std::string &value){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN(!isNull());
      m_poGrabber->setProperty(property,value);
    }

    /// returns whether property is supported by underlying grabber
    virtual bool supportsProperty(const std::string &property){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),false);
      return m_poGrabber->supportsProperty(property);
    }
    
    /// returns the property type of given property
    virtual std::string getType(const std::string &name){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getType(name);
    }
     
    /// retuns property information
    virtual std::string getInfo(const std::string &name){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getInfo(name);
    }
    
    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getValue(name);
    }

    /// returns volatileness of given property
    virtual int isVolatile(const std::string &propertyName){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),0);
      return m_poGrabber->isVolatile(propertyName);
    }

    /// returns wheter an underlying grabber could be created
    bool isNull() const { return m_poGrabber == 0; }
    
    /// simpler interface for isNull() (returns !isNull()
    operator bool() const { return !isNull(); }


  /// internally set a desired format
    virtual void setDesiredFormatInternal(format fmt){
      ICLASSERT_RETURN(!isNull());
      Mutex::Locker l(m_mutex);
      m_poGrabber->setDesiredFormatInternal(fmt);
    }

    /// internally set a desired format
    virtual void setDesiredSizeInternal(const Size &size){
      ICLASSERT_RETURN(!isNull());
      Mutex::Locker l(m_mutex);
      m_poGrabber->setDesiredSizeInternal(size);
    }

    /// internally set a desired format
    virtual void setDesiredDepthInternal(depth d){
      ICLASSERT_RETURN(!isNull());
      Mutex::Locker l(m_mutex);
      m_poGrabber->setDesiredDepthInternal(d);
    }

    /// returns the desired format
    virtual format getDesiredFormatInternal() const{
      ICLASSERT_RETURN_VAL(!isNull(),(format)-1);
      Mutex::Locker l(m_mutex);
      return m_poGrabber->getDesiredFormatInternal();
    }

    /// returns the desired format
    virtual depth getDesiredDepthInternal() const{
      ICLASSERT_RETURN_VAL(!isNull(),(depth)-1);
      Mutex::Locker l(m_mutex);
      return m_poGrabber->getDesiredDepthInternal();
    }

    /// returns the desired format
    virtual Size getDesiredSizeInternal() const{
      ICLASSERT_RETURN_VAL(!isNull(),Size::null);
      Mutex::Locker l(m_mutex);
      return m_poGrabber->getDesiredSizeInternal();
    }

     /// returns a list of all currently available devices (according to the filter-string)
     /** The filter-string is a comma separated list of single filters like
         <pre> dc=0,unicap </pre>
         If a single token has the format deviceType=deviceID, then only not only the
         device type but also a specific ID is used for the filtering operation. If, otherwise,
         a token has the format deviceType, then all possible devices for this device type are
         listed.
     */
     static const std::vector<GrabberDeviceDescription> &getDeviceList(const std::string &filter, bool rescan=true);
     
     /// initializes the grabber from given FoundDevice instance
     /** calls 'init(dev.type,dev.type+"="+dev.id,false)' */
     inline void init(const GrabberDeviceDescription &dev){
       init(dev.type,dev.type+"="+dev.id,false);
     }
        
    
  };



 
} 

#endif
