/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/PylonGrabber.h                           **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#ifndef ICL_PYLON_GRABBER_H
#define ICL_PYLON_GRABBER_H

//#define PYLON_COLOR_CONVERSION
#define ICL_COLOR_CONVERSION

#include <pylon/PylonIncludes.h>
#include <pylon/PixelFormatConverterBayer.h>
#include <pylon/TransportLayer.h>
#include <pylon/PixelType.h>

#include <ICLIO/GrabberHandle.h>
#include <ICLCC/BayerConverter.h>

namespace icl {

  class PylonGrabberBuffer
  {
    public:
    PylonGrabberBuffer(const size_t imageSize);
    ~PylonGrabberBuffer();

    uint8_t* getBufferPointer(void) { return m_pBuffer; }
    Pylon::StreamBufferHandle getBufferHandle(void) { return m_hBuffer; }
    void setBufferHandle(Pylon::StreamBufferHandle hBuffer) { m_hBuffer = hBuffer; };

    private:
    uint8_t *m_pBuffer;
    Pylon::StreamBufferHandle m_hBuffer;
  };

  class PylonGrabberImpl : public Grabber{
    public:
    friend class PylonGrabber;
    friend class PylonAutoEnv;
    friend class AcquisitionInterruptor;

    /// interface for the setter function for video device properties
    virtual void setProperty(const std::string &property, const std::string &value);
   	/// returns a list of properties, that can be set using setProperty
    virtual std::vector<std::string> 	getPropertyList();
   	/// base implementation for property check (seaches in the property list)
    virtual bool supportsProperty(const std::string &property);
    /// get type of property
    virtual std::string getType(const std::string &name);
    /// get information of a properties valid values
    virtual std::string getInfo(const std::string &name);
    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name);
   	/// Returns whether this property may be changed internally. 
    virtual int isVolatile(const std::string &propertyName);

    /// Destructor
    ~PylonGrabberImpl();
         
    /// grab function grabs an image (destination image is adapted on demand)
    /** @copydoc icl::Grabber::grab(ImgBase**) **/
    virtual const ImgBase* acquireImage();

    static Pylon::DeviceInfoList_t getPylonDeviceList();

    private:
    /// the constructor is private so only the friend class can create instances
    PylonGrabberImpl(const Pylon::CDeviceInfo &dev);
    
    /// A mutex lock for the camera
    icl::Mutex m_CamMutex;
    /// count of buffers for grabbing
    static const int m_NumBuffers = 10;
    /// image height
    int m_Height;
    /// image width
    int m_Width;
    /// image offset in x direction
    int m_Offsetx;
    /// image offset in y direction
    int m_Offsety;
    /// image format
    std::string m_Format;
    /// the camera interface.
    Pylon::IPylonDevice* m_Camera;

    /// the streamGrabber of the camera.
    Pylon::IStreamGrabber* m_Grabber;
    /// a list of used buffers.
    std::vector<PylonGrabberBuffer*> m_BufferList;
    /**
    * indicates whether m_Image and m_ColorConverter should
    * be reinitialized in the next acquireImage call.
    */
    bool m_ResetImage;
    /// an IclImageBase
    icl::Img8u* m_Image2;
    icl::ImgBase* m_Image;
    /// Pylon color format converter
    icl::BayerConverter* m_ColorConverter;
    //Pylon::CPixelFormatConverterBayer m_ColorConv;
    //Pylon::SImageFormat m_InputFormat;
    //Pylon::SOutputImageFormat m_OutputFormat;

    /// a variable to count aquired picture.
    unsigned long m_Aquired;
    /// a variable to count aquiring-errors.
    unsigned long m_Error;
    
    /// initializes the Pylon environment.
    /** @return whether Pylon::PylonInitialize() actually was called. */
    static bool initPylonEnv();
    /// terminates the Pylon environment.
    /** @return whether Pylon::PylontTerminate() actually was called. */
    static bool termPylonEnv();
    /// starts the acquisition of pictures by the camera
    void acquisitionStart();
    /// stops the acquisition of pictures by the camera
    void acquisitionStop();
    /// creates buffers and registers them at the grabber
    void prepareGrabbing();
    /// deregisters buffers from grabber and deletes them
    void finishGrabbing();
    /// helper function for getPropertyList.
    void addToPropertyList(std::vector<std::string> &ps, const GenApi::CNodePtr& node);
    /// helper function that makes default settings for the camera.
    void cameraDefaultSettings();
    /// creates a new ImgBase is not already there.
    void initImgBase();
    /// returns the expected image size. used to get the needed size of buffers.
    long getNeededBufferSize();
    /// returns the expected pixel size. used by getNeededBufferSize().
    int getBitsPerPixel();
    /// returns a string representation of the value of a parameter of the camera.
    std::string getParameterValueString(std::string parameter);
    /// used for getting a value of a parameter of a specific type from a specific source (camera/grabber)
    template <typename SOURCE, typename NODE, typename RET>
    RET getParameterValueOf(SOURCE* source, std::string param);
    /// used for setting a value of a parameter of a specific type on a specific source (camera/grabber)
    template <typename OBJ, typename NODE, typename VAL>
    bool setParameterValueOf(OBJ* object, std::string parameter, VAL value);
    /// template function to get the value of an IValue-subclass
    template <typename NODE, typename RET>
    RET getNodeValue(NODE* node);
    /// template function overload to get the value of an IEnumeration
    template <typename NODE, typename RET>
    std::string getNodeValue(GenApi::IEnumeration* node);
    /// template function to set the value of an IValue-subclass
    template <typename NODE, typename VAL>
    void setNodeValue(NODE* node, VAL value);
    /// template function overload to set the value of an IEnumeration
    template <typename NODE, typename VAL>
    void setNodeValue(GenApi::IEnumeration* node, std::string value);
  };

  /** This is just a wrapper class of the underlying PylonGrabberImpl class */
  struct PylonGrabber : public GrabberHandle<PylonGrabberImpl>{

    /// create a new PylonGrabber
    /** @see PylonGrabberImpl for more details*/
    inline PylonGrabber(const Pylon::CDeviceInfo *dev=NULL){
      if(!dev){
        // return when no devices are found
        if(PylonGrabberImpl::getPylonDeviceList().empty()) { return; }
        // initializing grabber for first found camera
        Pylon::CDeviceInfo dev2 = PylonGrabberImpl::getPylonDeviceList().front();
        if(isNew(dev2.GetFullName().c_str())){
          initialize(new PylonGrabberImpl(dev2), dev2.GetFullName().c_str());
        }else{
          initialize(dev2.GetFullName().c_str());
        }
      }
      if(isNew(dev -> GetFullName().c_str())){
        initialize(new PylonGrabberImpl(*dev), dev -> GetFullName().c_str());
      }else{
        initialize(dev -> GetFullName().c_str());
      }
      getPylonDeviceList();
    }
    /// Returns a list of detected pylon devices
    static Pylon::DeviceInfoList_t getPylonDeviceList(){
      return PylonGrabberImpl::getPylonDeviceList();
    }
    
    static std::vector<GrabberDeviceDescription> getDeviceList(bool rescan){
    static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        Pylon::DeviceInfoList_t devs = getPylonDeviceList();
        for(unsigned int i = 0 ; i < devs.size() ; ++i){
          deviceList.push_back(
            GrabberDeviceDescription(
              "pylon",
              str(i) + "|||" + devs.at(i).GetFullName().c_str(),
              devs.at(i).GetFullName().c_str()
            )
          );
        }
      }
    return deviceList;
    }
  
  };
  
  /** 
   * This struct is used to automaticly terminate an initialized pylon 
   * environment on destruction. 
   */
  struct PylonAutoEnv{
    public:
      PylonAutoEnv(){PylonGrabberImpl::initPylonEnv();}
      ~PylonAutoEnv(){PylonGrabberImpl::termPylonEnv();}
  };
  
  /**
   * This struct is used to stop the picture-acquisition and restart it
   * on destruction. It also gets the mutex lock for the camera so the
   * camera can't be stopped twice.
   */
  struct AcquisitionInterruptor{
    private: 
      PylonGrabberImpl* m_Impl;
      icl::Mutex::Locker* m_Locker;

    public:
      /// gets the mutex lock and stops the acquisiton
      AcquisitionInterruptor(PylonGrabberImpl* i, bool mock=false){
        DEBUG_LOG("before stop")
        if(!mock){
          m_Impl = i;
          DEBUG_LOG("Locking camMutex")
          m_Locker = new icl::Mutex::Locker(i -> m_CamMutex);
          m_Impl -> acquisitionStop();
        } else {
          m_Impl = NULL;
          m_Locker = NULL;
        }
        DEBUG_LOG("after stop")

      }
      ~AcquisitionInterruptor(){
        DEBUG_LOG("before start")
        if((m_Locker != NULL) && (m_Impl != NULL)){
          m_Impl -> acquisitionStart();
          delete m_Locker;
          DEBUG_LOG("Releasing camMutex")
        }
        DEBUG_LOG("after start")
      }
  };
  
  
} //namespace

#endif
