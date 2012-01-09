/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/PylonGrabber.cpp                             **
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

#include <ICLUtils/Exception.h>
#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLIO/PylonGrabber.h>
#include <ICLUtils/Macros.h>

#include <pylon/gige/BaslerGigEDeviceInfo.h>

#include <unistd.h>

using namespace icl;

// Static mutex and counter for clean initialising
// and deleting of Pylon environment
static unsigned int pylon_env_inits = 0;
static icl::Mutex* env_mutex = new icl::Mutex();

// This enum is a helper to get the correct icl-value-type strings
enum icl_val_type {
  range,
  value_list,
  menu,
  command,
  info
} icl_val_type;
const char *icl_val_str[]={ "range", "value-list", "menu", "command", "info" };

// This is a list of dafault sizes strings for
// the 'size' parameter of the camera
const char *default_sizes[]={
      "128x96", "176x144", "160x120", "320x200", "320x240",
      "352x288", "360x240", "480x320", "640x350", "640x480", "768x576",
      "800x480", "800x600", "960x540", "960x640", "1024x768", "1152x864",
      "1200x800", "1280x720", "1280x800", "1440x900", "1280x960",
      "1280x1024", "1600x900", "1400x1050", "1600x1050", "1600x1200",
      "1920x1080", "3840x2160"
};
const int default_sizes_count =sizeof(default_sizes)/sizeof(char*);

// Function definitions
// template function to get the value of an IValue-subclass
template <typename NODE, typename RET>
RET getNodeValue(NODE* node){
  return node -> GetValue();
}

// template function overload to get the int64_t-value of an IEnumeration
template <typename NODE, typename RET>
int64_t getNodeValue(GenApi::IEnumeration* node){
  return node -> GetIntValue();
}

// template function to set the value of an IValue-subclass
template <typename NODE, typename VAL>
void setNodeValue(NODE* node, VAL value){
  node -> SetValue(value, true);
  return;
}

// template function overload to set the value of an IEnumeration
template <typename NODE, typename VAL>
void setNodeValue(GenApi::IEnumeration* node, std::string value){
  node -> FromString(value.c_str(), true);
  return;
}

// template function overload to set the value of an IEnumeration
template <typename NODE, typename VAL>
void setNodeValue(GenApi::IEnumeration* node, int64_t value){
  node -> SetIntValue(value, true);
  return;
}

// used for setting a value of a parameter of a specific type on
// a specific source (camera/grabber)
template <typename OBJ, typename NODE, typename VAL>
bool setParameterValueOf(OBJ* object, std::string parameter, VAL value);

// used for getting a value of a parameter of a specific type from
// a specific source (camera/grabber)
template <typename SOURCE, typename NODE, typename RET>
RET getParameterValueOf(SOURCE* source, std::string param);

// returns a string representation of the value of a parameter of the camera.
GenICam::gcstring getParameterValueString(
  Pylon::IPylonDevice* camera,
  std::string parameter
  );

// returns the Pylon::PixelType enum describing the cameras current pixel type
Pylon::PixelType getCameraPixelType(Pylon::IPylonDevice* camera);

// returns the expected pixel size.
int getCameraPixelSize(Pylon::IPylonDevice* camera);

// returns the expected image size. used to get the needed size of buffers.
long getNeededBufferSize(Pylon::IPylonDevice* camera);

// returns which Streamgrabber-channel is specified in args. returns 0 when
// not specified
int channelFromArgs(std::string args);

// Constructor of PylonGrabberImpl
PylonGrabberImpl::PylonGrabberImpl(const Pylon::CDeviceInfo &dev,
  const std::string args) : m_CamMutex(), m_Aquired(0), m_Error(0),
  m_ResetImage(true)
{
  DEBUG_LOG("args: " << args)
  // getting camera mutex to exclude race-conditions
  icl::Mutex::Locker l(m_CamMutex);
  m_Image = NULL;
  m_Image2 = NULL;
  m_ColorConverter = NULL;
  // Initialization of the pylon Runtime Library
  initPylonEnv();
  Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();
  m_Camera = tlFactory.CreateDevice(dev);

  int channel = channelFromArgs(args);
  if(m_Camera -> GetNumStreamGrabberChannels() == 0){
    throw icl::ICLException("No stream grabber channels avaliable.");
  } else if(m_Camera -> GetNumStreamGrabberChannels() < channel){
    DEBUG_LOG("From args='" << args << "' demanded channel=" << channel <<
              "but available=" << m_Camera -> GetNumStreamGrabberChannels())
    throw icl::ICLException("Demanded StreamGrabberChannel not avaliable.");
  }

  m_Camera -> Open();
  cameraDefaultSettings();
  // getting first Grabber
  m_Grabber = m_Camera -> GetStreamGrabber(channel);

  m_Grabber -> Open();

  prepareGrabbing();

  // Let the camera acquire images
  acquisitionStart();
}

void PylonGrabberImpl::prepareGrabbing(){
  // Get the image buffer size
  const size_t imageSize = getNeededBufferSize(m_Camera);
  DEBUG_LOG("Buffer size: " << imageSize)
  //unsigned char* ppBuffers[m_NumBuffers];
  //Pylon::StreamBufferHandle handles[m_NumBuffers];

  // We won't use image buffers greater than imageSize
  setParameterValueOf<Pylon::IStreamGrabber, GenApi::IInteger, int>
    (m_Grabber, "MaxBufferSize", imageSize);
  // We won't queue more than m_NumBuffers image buffers at a time
  setParameterValueOf<Pylon::IStreamGrabber, GenApi::IInteger, int>
    (m_Grabber, "MaxNumBuffer", m_NumBuffers);

  // Allocate all resources for grabbing. Critical parameters like image
  // size now must not be changed until FinishGrab() is called.
  m_Grabber -> PrepareGrab();

  // Buffers used for grabbing must be registered at the stream grabber -> 
  // The registration returns a handle to be used for queuing the buffer.
  for (int i = 0; i < m_NumBuffers; ++i){
    PylonGrabberBuffer<uint16_t> *pGrabBuffer =
      new PylonGrabberBuffer<uint16_t>(imageSize);
    Pylon::StreamBufferHandle handle =
      m_Grabber -> RegisterBuffer(pGrabBuffer -> getBufferPointer(), imageSize);
    pGrabBuffer -> setBufferHandle(handle);

    // Put the grab buffer object into the buffer list
    m_BufferList.push_back(pGrabBuffer);

    // Put buffer into the grab queue for grabbing
    m_Grabber -> QueueBuffer(handle);
    m_ResetImage = true;
  }
}

void PylonGrabberImpl::finishGrabbing(){
  m_Grabber -> CancelGrab();
  Pylon::GrabResult result;
  while (m_Grabber -> GetWaitObject().Wait(0)) {
    if (!m_Grabber -> RetrieveResult(result)) {
      std::cerr << "Failed to retrieve item from output queue" << std::endl;
    }
  }

  // deregister the buffers before freeing the memory
  std::vector<PylonGrabberBuffer<uint16_t>*>::iterator it;
  for (it = m_BufferList.begin(); it != m_BufferList.end(); it++){
    m_Grabber -> DeregisterBuffer((*it) -> getBufferHandle());
    delete *it;
    *it = NULL;
  }
  m_BufferList.clear();

  // Free all resources used for grabbing
  m_Grabber -> FinishGrab();
}

void PylonGrabberImpl::acquisitionStart(){
  GenApi::INode* node = m_Camera -> GetNodeMap() -> GetNode("AcquisitionStart");
  try {
  GenApi::ICommand* node2 = dynamic_cast<GenApi::ICommand*>(node);
  node2 -> Execute();
  } catch (std::exception &e){
    DEBUG_LOG("Could not cast 'AcquisitionStart' to an ICommand")
  }
}

void PylonGrabberImpl::acquisitionStop(){
  GenApi::ICommand* node =
    (GenApi::ICommand*) m_Camera -> GetNodeMap() -> GetNode("AcquisitionStop");
  try {
  GenApi::ICommand* node2 = dynamic_cast<GenApi::ICommand*>(node);
  node2 -> Execute();
  } catch (std::exception &e){
    DEBUG_LOG("Could not cast 'AcquisitionStop' to an ICommand")
  }
}

PylonGrabberImpl::~PylonGrabberImpl(){
  // getting camera mutex to exclude race-conditions
  icl::Mutex::Locker l(m_CamMutex);
  
  std::cout << "pylongrabber destructor called" << std::endl;
  std::cout << "pictures aquired: " << m_Aquired << " errors: " << m_Error 
  << " rate: " << (float) m_Aquired / (float) m_Error << std::endl;

  // Stop acquisition
  std::cout << "stopping image aquisition" << std::endl;
  acquisitionStop();
  std::cout << "image aquisition stopped" << std::endl;
  
  // deregister buffers
  finishGrabbing();
  
  // Close stream grabber
  m_Grabber -> Close();
  // Close camera
  m_Camera -> Close();
  // Free resources allocated by the pylon runtime system.
  std::cout << "terminating pylon" << std::endl;
  delete m_ColorConverter;
  delete m_Image;
  termPylonEnv();
}

void PylonGrabberImpl::cameraDefaultSettings(){
  m_Height = getParameterValueOf
    <Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Height");
  m_Width = getParameterValueOf
    <Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Width");
  m_Offsetx = getParameterValueOf
    <Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "OffsetX");
  m_Offsety = getParameterValueOf
    <Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "OffsetY");
  m_Format = getParameterValueString(m_Camera, "PixelFormat");

  //std::cout << "ImageFormat at startup: " << m_Width << "x" << m_Height
  //<< " , " << m_Format << std::endl;
  //std::cout << "Payload: " << getParameterValueString("PayloadSize")
  //<< std::endl;

  m_Height = 480;
  m_Width = 640;
  m_Offsetx = 640;
  m_Offsety = 300;
  //m_Format = "BayerGB16";
  m_Format = "Mono8";

  DEBUG_LOG("setting camera to " << m_Format)
  // default camera settings: image format, AOI, acquisition mode and exposure
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
    (m_Camera, "PixelFormat", m_Format);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
    (m_Camera, "OffsetX", m_Offsetx);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
    (m_Camera, "OffsetY", m_Offsety);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
    (m_Camera, "Width", m_Width);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
    (m_Camera, "Height", m_Height);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
    (m_Camera, "GevSCPSPacketSize", 1500);
  //std::cout << "ImageFormat after settings: " 
  //  << getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
  //    (m_Camera, "Width") << "x"
  //  << getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
  //    (m_Camera, "Height") << " , "
  //  << getParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration,
  //    std::string>(m_Camera, "PixelFormat")
  //  << std::endl;
  //std::cout << "Payload: "
  //  << getParameterValueString("PayloadSize") << std::endl;
  //std::cout << "PixelSize: "
  //  << getParameterValueString("PixelSize") << std::endl;

  // default Aquisition Mode
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
    (m_Camera, "AcquisitionMode", "Continuous");
}

const icl::ImgBase* PylonGrabberImpl::acquireImage(){
  // getting camera mutex so camera will not be stopped
  icl::Mutex::Locker l(m_CamMutex);
  // Wait for the grabbed image with timeout of 0.5 seconds
  if (!m_Grabber -> GetWaitObject().Wait(500)){
    // Timeout
    DEBUG_LOG("Timeout occurred!")
    ++m_Error;
    return m_Image;
  }
  // Get the grab result from the grabber's result queue
  Pylon::GrabResult result;
  m_Grabber -> RetrieveResult(result);

  if (result.Succeeded()){
    ++m_Aquired;
    // Grabbing was successful, process image
    const void *pImageBuffer = result.Buffer();
    initImgBase();

    /*DEBUG_LOG("w = " << m_Width << " h = " << m_Height << " o = "
      << m_Offsetx << "x" << m_Offsety)
    DEBUG_LOG("BufferSize: " << getNeededBufferSize(m_Camera)
      << " BytesPerPixel: " << getCameraPixelSize(m_Camera))
    DEBUG_LOG("channels: " << m_Image -> getChannels())
    DEBUG_LOG("width: " << m_Image -> getWidth())
    DEBUG_LOG("height: " << m_Image -> getHeight())
    DEBUG_LOG("buffer psize: " << result.GetPayloadSize())
    DEBUG_LOG("buffer ptype: " << result.GetPayloadType())
    DEBUG_LOG("buffer pixtype: " << result.GetPixelType())
    DEBUG_LOG("buffer size x: " << result.GetSizeX())
    DEBUG_LOG("buffer size y: " << result.GetSizeY())*/
    convert(pImageBuffer);

    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);

    return m_Image;

  } else {
    ++m_Error;
    // Error handling
    DEBUG_LOG("No image acquired!" << "Error description : "
      << result.GetErrorDescription())

    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
    return m_Image;
  }
}

void PylonGrabberImpl::convert(const void *pImageBuffer){
  if (m_Convert == yes_rgba){
    m_ColorConverter -> Convert(m_Image2, m_Width*m_Height*4, pImageBuffer,
      getNeededBufferSize(m_Camera), m_InputFormat, m_OutputFormat);
    Img8u* img = dynamic_cast<Img8u*>(m_Image);
    for(int y = 0; y < m_Image -> getHeight(); ++y){
      for(int x = 0; x < m_Image -> getWidth(); ++x){
        (*img)(x,y,0) = m_Image2[m_Width*y*4 + x*4 + 2];//2
        (*img)(x,y,1) = m_Image2[m_Width*y*4 + x*4 + 1];//1
        (*img)(x,y,2) = m_Image2[m_Width*y*4 + x*4 + 0];//0
      }
    }
  } else if (m_Convert == yes_mono8u){
    // m_Image2 is already registered as content of m_Image
    m_ColorConverter -> Convert(m_Image2, m_Width*m_Height*4, pImageBuffer,
      getNeededBufferSize(m_Camera), m_InputFormat, m_OutputFormat);
  } else if (m_Convert == no_mono8u){
  Img8u* img = dynamic_cast<Img8u*>(m_Image);
  uint8_t* pImageBuffer8 = (uint8_t*) pImageBuffer;
    for(int y = 0; y < m_Image -> getHeight(); ++y){
      for(int x = 0; x < m_Image -> getWidth(); ++x){
        (*img)(x,y,0) = pImageBuffer8[m_Width*y + x];
      }
    }
  } else if (m_Convert == no_mono16){
  Img16s* img = dynamic_cast<Img16s*>(m_Image);
  int16_t* pImageBuffer16 = (int16_t*) pImageBuffer;
    for(int y = 0; y < m_Image -> getHeight(); ++y){
      for(int x = 0; x < m_Image -> getWidth(); ++x){
        (*img)(x,y,0) = pImageBuffer16[m_Width*y + x];
      }
     }
  } else {
    std::stringstream ex("Conversion to color format convert_to=");
    ex << m_Convert << " not defined";
    throw new ICLException(ex.str());
  }
}

void PylonGrabberImpl::initImgBase(){
  //DEBUG_LOG("initImgBase here")
  if(m_ResetImage) {
    if(m_Image){
      delete m_Image;
      m_Image = NULL;
    }
    if(m_Image2){
      delete [] m_Image2;
      m_Image2 = NULL;
    }
    if(m_ColorConverter){
      delete m_ColorConverter;
      m_ColorConverter = NULL;
    }
    m_InputFormat = Pylon::SImageFormat();
    m_OutputFormat = Pylon::SOutputImageFormat();
    m_ResetImage = false;
  }

  if(!m_Image){
    DEBUG_LOG("creating image")
    // create color format converter
    Pylon::PixelType pixelType = getCameraPixelType(m_Camera);
    m_InputFormat.Width = m_Width;
    m_InputFormat.Height = m_Height;

    if(Pylon::IsPacked(pixelType)){
      m_InputFormat.LinePitch = 0;
    //} else if (Pylon::IsYUV(pixelType)){
    } else {
      m_InputFormat.LinePitch =
        (int) (m_Width * (getCameraPixelSize(m_Camera)/8.0) + 0.5);
    }

    // Settings for input format Bayer
    if (Pylon::IsBayer(pixelType)){
      m_Convert = yes_rgba;
      m_InputFormat.PixelFormat = pixelType;
      m_OutputFormat.LinePitch = m_Width*4;
      m_OutputFormat.PixelFormat = Pylon::PixelType_RGBA8packed;

      m_Image = new Img8u(Size(m_Width, m_Height), icl::formatRGB);
      m_Image2 = new icl8u[m_Width*m_Height*4];

      m_ColorConverter = new Pylon::CPixelFormatConverterBayer();
      m_ColorConverter -> Init(m_InputFormat);
    // Settings for input format Yuv422-UYVY
    } else if (pixelType == Pylon::PixelType_YUV422packed){
      m_Convert = yes_rgba;
      m_InputFormat.PixelFormat = pixelType;
      m_OutputFormat.LinePitch = m_Width*4;
      m_OutputFormat.PixelFormat = Pylon::PixelType_RGBA8packed;

      m_Image = new Img8u(Size(m_Width, m_Height), icl::formatRGB);
      m_Image2 = new icl8u[m_Width*m_Height*4];

      m_ColorConverter = new Pylon::CPixelFormatConverterYUV422;
      m_ColorConverter -> Init(m_InputFormat);
    // Settings for input format Yuv422-YUYV
    } else if (pixelType == Pylon::PixelType_YUV422_YUYV_Packed){
      m_Convert = yes_rgba;
      m_InputFormat.PixelFormat = pixelType;
      m_OutputFormat.LinePitch = m_Width*4;
      m_OutputFormat.PixelFormat = Pylon::PixelType_RGBA8packed;

      m_Image = new Img8u(Size(m_Width, m_Height), icl::formatRGB);
      m_Image2 = new icl8u[m_Width*m_Height*4];

      m_ColorConverter = new Pylon::CPixelFormatConverterYUV422YUYV;
      m_ColorConverter -> Init(m_InputFormat);
    // Settings for input format Mono
    } else if (Pylon::IsMono(pixelType)){
      // Settings for input format Mono8
      if (pixelType == Pylon::PixelType_Mono8
          || pixelType == Pylon::PixelType_Mono8signed){
        m_Convert = no_mono8u;
        m_Image = new Img8u(Size(m_Width, m_Height), icl::formatGray);
        // Settings for input format Mono16
      } else if (pixelType == Pylon::PixelType_Mono16){
        m_Convert = no_mono16;
        m_Image = new Img16s(Size(m_Width, m_Height), icl::formatGray);
        // Settings for other Mono-input formats (convert using truncation)
      } else {
        m_Convert = yes_mono8u;
        m_InputFormat.PixelFormat = pixelType;
        m_OutputFormat.LinePitch = m_Width;
        m_OutputFormat.PixelFormat = Pylon::PixelType_Mono8;

        m_Image2 = new icl8u[m_Width*m_Height];
        std::vector<icl8u*> channels;
        channels.push_back(m_Image2);
        m_Image = new Img8u(Size(m_Width, m_Height),
                            icl::formatGray, channels, false);
        if(Pylon::IsPacked(pixelType)){
          m_ColorConverter = new Pylon::CPixelFormatConverterTruncatePacked();
        } else {
          m_ColorConverter = new Pylon::CPixelFormatConverterTruncate();
        }
        m_ColorConverter -> Init(m_InputFormat);
      }
    } else {
      std::stringstream error("PixelColor type ");
      error << pixelType << " not supported by PylonGrabber.";
      throw ICLException(error.str());
    }
  }
}

Pylon::DeviceInfoList_t PylonGrabberImpl::getPylonDeviceList(){
  // Initialization and auto termination of pylon runtime library
  PylonAutoEnv();

  Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();
  Pylon::DeviceInfoList_t lstDevices;

  // Get all attached cameras
  tlFactory.EnumerateDevices(lstDevices);

  std::cout << "-------------Devices---------------" << std::endl;
  Pylon::DeviceInfoList_t::const_iterator it;
  for (it = lstDevices.begin(); it != lstDevices.end(); ++it)
    std::cout << it -> GetFullName() << std::endl;
  std::cout << "-----------------------------------" << std::endl;
  return lstDevices;
}

// interface for the setter function for video device properties
void PylonGrabberImpl::setProperty(
  const std::string &property, const std::string &value)
{
  std::cout << "setProperty(" << property << ", " << value << ")" << std::endl;
  if(property.compare("size") == 0){
    icl::Size size(value);
    std::ostringstream w;
    w << size.width;
    std::ostringstream h;
    h << size.height;
    setProperty("Width", w.str());
    setProperty("Height", h.str());
    return;
  }
  if(property.compare("format") == 0){
    setProperty("PixelFormat", value);
    return;
  }
  
  GenApi::INodeMap* nodeMap = m_Camera -> GetNodeMap();
  GenApi::CValuePtr node = nodeMap -> GetNode(property.c_str());
  if (!node) {
    std::cerr << "There is no parameter called '"
      << property << "'" << std::endl;
    return;
  }

  AcquisitionInterruptor acqInt(this, GenApi::IsWritable(node));

  bool stopped = false;
  if (!GenApi::IsWritable(node)){
    DEBUG_LOG("Stop grabbing to set '" << property << "' parameter")
    finishGrabbing();
    stopped = true;
    if(!GenApi::IsWritable(node)){
      DEBUG_LOG("The parameter '" << property << "' is still not writable")
      prepareGrabbing();
      return;
    }
  }

  try {
  node -> FromString(value.c_str(), true);
  } catch (GenICam::GenericException &e) {
    DEBUG_LOG("catched exception: " << e.what())
  }
  
  if(stopped) {
    prepareGrabbing(); 
  }
}

// returns a list of properties, that can be set using setProperty
std::vector<std::string> PylonGrabberImpl::getPropertyList(){
  DEBUG_LOG("get property list")
  std::vector<std::string> ps;
  ps.push_back("size");
  ps.push_back("format");
  GenApi::INode* rootNode = m_Camera -> GetNodeMap() -> GetNode("Root");
  addToPropertyList(ps, rootNode);
  return ps;
}

// helper function for getPropertyList
void PylonGrabberImpl::addToPropertyList(
  std::vector<std::string> &ps, const GenApi::CNodePtr& node)
{
  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) { 
    case GenApi::intfIInteger:
    case GenApi::intfIFloat:
    case GenApi::intfIBoolean:
    case GenApi::intfIEnumeration:
      if (GenApi::IsReadable(node) || GenApi::IsWritable(node)){
  	    // If the feature is implemented, add name of node	  
        ps.push_back(node->GetName().c_str());
      }
      return;

    case GenApi::intfICategory:
    {
      GenApi::CCategoryPtr ptrCategory (node);  
      if (ptrCategory) {
        // add name of category
      	GenApi::FeatureList_t features;
        ptrCategory -> GetFeatures(features);
      	GenApi::FeatureList_t::const_iterator it;
    	  for (it = features.begin(); it != features.end(); ++it){
      		addToPropertyList(ps, *it);
      	}
      }
      return;
    }
    // for now these stay info nodes
    case GenApi::intfICommand:
    case GenApi::intfIEnumEntry:
    case GenApi::intfIString:
    case GenApi::intfIRegister:
    case GenApi::intfIPort:
    case GenApi::intfIBase:
    default: 
      return;
  }
}

// base implementation for property check (seaches in the property list)
bool PylonGrabberImpl::supportsProperty(const std::string &property){
  DEBUG_LOG(property)
  if((property.compare("size") == 0) || (property.compare("format") == 0)){
    return true;
  }
  // Check whether node exists
  GenApi::INode* node = m_Camera -> GetNodeMap() -> GetNode(property.c_str());
  if(!node) return false;

  // Check whether property is implemented
  if(!GenApi::IsImplemented(node)) return false;
  if(!GenApi::IsAvailable(node)) return false;

  // check type
  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) { 
    case GenApi::intfIInteger:
    case GenApi::intfIFloat:
    case GenApi::intfIBoolean:
    case GenApi::intfIEnumeration:
    case GenApi::intfICommand:
      return true;

    case GenApi::intfICategory:
    case GenApi::intfIEnumEntry:
    case GenApi::intfIString:
    case GenApi::intfIRegister:
    case GenApi::intfIPort:
    case GenApi::intfIBase:
    default: 
      return false;
  }
}

// get type of property
std::string PylonGrabberImpl::getType(const std::string &name){
  DEBUG_LOG(name)
  if(name.compare("size") == 0){
    return icl_val_str[menu];
  }
  if(name.compare("format") == 0){
    return getType("PixelFormat");
  }

  GenApi::INode* node = m_Camera -> GetNodeMap() -> GetNode(name.c_str());
  if (!node) {
    std::cerr << "There is no parameter called '" << name << "'" << std::endl;
    return icl_val_str[info];
  }

  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) { 
    case GenApi::intfIInteger:
    case GenApi::intfIFloat:
      return icl_val_str[range];

    case GenApi::intfIBoolean:
      return icl_val_str[value_list];

    case GenApi::intfIEnumeration:
      return icl_val_str[menu];

    case GenApi::intfICommand:
      return icl_val_str[command];

    // for now these stay info nodes
    case GenApi::intfIEnumEntry:
    case GenApi::intfIString:
    case GenApi::intfIRegister:
    case GenApi::intfICategory:
    case GenApi::intfIPort:
    case GenApi::intfIBase:
    default: 
      return icl_val_str[info];
  }
}

// get information of a properties valid values
std::string PylonGrabberImpl::getInfo(const std::string &name){
  DEBUG_LOG(name)
  std::ostringstream ret;
  if(name.compare("size") == 0){
    ret << "{";
    ret << default_sizes[0];
    for (int i = 1; i < default_sizes_count; i++){
      ret << ",";
      ret << default_sizes[i];
    }
    ret << "}";
    return ret.str();
  }
  if(name.compare("format") == 0){
    return getInfo("PixelFormat");
  }

  GenApi::INode* node = m_Camera -> GetNodeMap() -> GetNode(name.c_str());
  if (!node) {
    std::cerr << "There is no parameter called '" << name << "'" << std::endl;
    return "";
  }

  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) { 
    case GenApi::intfIInteger:
      GenApi::IInteger* inode;
      if((inode = dynamic_cast<GenApi::IInteger*>(node))) {
        ret << "[" << inode -> GetMin() << "," << inode -> GetMax()
          << "]:" << inode -> GetInc();
      }
      break;

    case GenApi::intfIFloat:
      GenApi::IFloat* fnode;
      if((fnode = dynamic_cast<GenApi::IFloat*>(node))) {
        ret << "[" << fnode -> GetMin() << "," << fnode -> GetMax() << "]";
        if(fnode -> HasInc()){
          ret << ":" << fnode -> GetInc();
        }
      }
      break;

    case GenApi::intfIBoolean:
    case GenApi::intfICommand:
      if(dynamic_cast<GenApi::IBoolean*>(node)) {
        ret << "{" << "true" << "," << "false" << "}";
      }
      break;
    case GenApi::intfIEnumeration:
      GenApi::IEnumeration* enode;
      if((enode = dynamic_cast<GenApi::IEnumeration*>(node))) {
        Pylon::StringList_t symbols;
        enode -> GetSymbolics(symbols);
        ret << "{";
        Pylon::StringList_t::iterator it;
        for (it = symbols.begin(); it != symbols.end(); ++it){
          ret << it -> c_str();
          if(it+1 != symbols.end()){
            ret << ",";
          }
        }
        ret << "}";
      }
      break;

    // for now these stay info nodes
    case GenApi::intfIEnumEntry:
    case GenApi::intfIString:
    case GenApi::intfIRegister:
    case GenApi::intfICategory:
    case GenApi::intfIPort:
    case GenApi::intfIBase:
    default:
      break;
  }
    return ret.str();
}

// returns the current value of a property or a parameter
std::string PylonGrabberImpl::getValue(const std::string &name){
  DEBUG_LOG("getValue of " << name)
  if(name.compare("size") == 0){
    std::ostringstream ret;
    ret << getValue("Width") << "x" << getValue("Height");
    return ret.str();
  }
  if(name.compare("format") == 0){
    return getValue("PixelFormat");
  }
  
  GenApi::INode* node = 
    m_Camera -> GetNodeMap() -> GetNode(name.c_str());
  if (!node) {
    DEBUG_LOG("There is no parameter called '" << name << "'")
    return "";
  }
  GenApi::IValue* val;
  if((val = dynamic_cast<GenApi::IValue*>(node))){
    return (val -> ToString()).c_str();
  }
  DEBUG_LOG("Node named '" << name << "' could not be cast to 'IValue'.")
  return "";
}

// Returns whether this property may be changed internally. 
int PylonGrabberImpl::isVolatile(const std::string &propertyName){
  // find out which.
  return true;
}

// initializes the Pylon environment
// (returns whether PylonInitialize() was called)
bool PylonGrabberImpl::initPylonEnv(){
  icl::Mutex::Locker l(env_mutex);
  ICLASSERT(pylon_env_inits >= 0)
  pylon_env_inits++;
  if(pylon_env_inits == 1){
    DEBUG_LOG("Initializing Pylon environment")
    Pylon::PylonInitialize();
    return true;
  } else {
    return false;
  }
}

// terminates the Pylon environment
// (returns whether PylonTerminate() was called).
bool PylonGrabberImpl::termPylonEnv(){
  icl::Mutex::Locker l(env_mutex);
  ICLASSERT(pylon_env_inits > 0)
  pylon_env_inits--;
  if(pylon_env_inits == 0){
    DEBUG_LOG("Terminating Pylon environment")
    Pylon::PylonTerminate();
    return true;
  } else {
    return false;
  }
}

void PylonGrabberImpl::printHelp(){
  std::cout << "The Pylon grabber can be called with the following parameters" << std::endl;
  std::cout << "        -i pylon 'CAM','BUFFER_NR'" << std::endl;
  std::cout << "  CAM_NR can be an integer, choosing one of "
               "the available cameras or the ip-addres of a perticular "
               "ip-camera." << std::endl;
  std::cout << "  BUFFER_NR is optional and chooses one of the Streambuffers "
               "provided by the camera." << std::endl;
  std::cout << "A convenient call could be:" << std::endl;
  std::cout << "        -i pylon 0" << std::endl;
  std::cout << "  to get the first stream of the first camera." << std::endl;
}

Pylon::CDeviceInfo PylonGrabberImpl::getDeviceFromArgs(std::string args)
  throw(ICLException)
{
  Pylon::DeviceInfoList_t devices = getPylonDeviceList();
  if(devices.empty()){
    throw ICLException("No Pylon devices found.");
  }
  std::vector<std::string> 	argvec = icl::tok(args, ",");
  ICLASSERT(argvec.size() <= 2)
    if(argvec.at(0).find('.') == std::string::npos){
      int nr = icl::parse<int>(argvec.at(0));
      if(devices.size() < nr + 1){
        DEBUG_LOG("Demanded device Nr. " << nr << " but only "
                  << devices.size() << " available.")
            throw ICLException("Could not find demanded device.");
      } else {
        return devices.at(nr);
      }
    } else { // cam by IP
      Pylon::CBaslerGigEDeviceInfo di;
      di.SetIpAddress(argvec.at(0).c_str());
      DEBUG_LOG("Trying to start camera from IP: " << argvec.at(0).c_str())
      return di;
    }
  DEBUG_LOG("Wrong parameters: " << args)
  throw ICLException("PylonDevice not found");
}

int channelFromArgs(std::string args){
  std::vector<std::string> 	argvec = icl::tok(args, ",");
  if(argvec.size() < 2){
      return 0;
  } else {
    return icl::parse<int>(argvec.at(1));
  }
}

template <typename OBJ, typename NODE, typename VAL>
bool setParameterValueOf(OBJ* object, std::string parameter, VAL value){
  GenApi::INode* node = object -> GetNodeMap() -> GetNode(parameter.c_str());
  if (!node) {
    DEBUG_LOG("There is no parameter called '" << parameter << "'")
    return false;
  }
  //dynamic cast to needed node-type
  NODE* node2;
  try{
    node2 = dynamic_cast<NODE*>(node);
  } catch (std::exception &e){
    DEBUG_LOG ("Could not cast node '"<< parameter << "' to desired type2")
    return false;
  }
  if(!GenApi::IsWritable(node2)){
    DEBUG_LOG("Parameter called '" << parameter << "' is not writable.")
    return false;
  }
  // now setting parameter
  try{
    setNodeValue<NODE, VAL>(node2, value);
    return true;
  }
  catch (GenICam::GenericException &e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

template <typename SOURCE, typename NODE, typename RET>
RET getParameterValueOf(SOURCE* source, std::string param){
  GenApi::INode* node = source -> GetNodeMap() -> GetNode(param.c_str());
  if(!node){
    throw icl::ICLException("parameter of this type not found");
  }
  //dynamic cast to needed node-type
  try{
    NODE* node2 = dynamic_cast<NODE*>(node);
    if(!node2){
      throw icl::ICLException("Could not cast " + param + " to desired type");
    }
    if(!GenApi::IsReadable(node2)){
      throw icl::ICLException("The node " + param + " is not Readable");
    }
    return getNodeValue<NODE, RET>(node2);
  } catch (std::exception &e){
    throw icl::ICLException(e.what());
  }
}

GenICam::gcstring getParameterValueString(Pylon::IPylonDevice* camera,
  std::string parameter)
{
  GenApi::INode* node = camera -> GetNodeMap() -> GetNode(parameter.c_str());
  if (!node) {
    DEBUG_LOG("There is no parameter called '" << parameter << "'")
    return NULL;
  }
  GenApi::IValue* value = dynamic_cast<GenApi::IValue*>(node);
  if (!value) {
    DEBUG_LOG("Could not cast '" << parameter << "' node to IValue")
    return NULL;
  }
  return value -> ToString();
}

Pylon::PixelType getCameraPixelType(Pylon::IPylonDevice* camera){
  GenICam::gcstring tStr = getParameterValueString(camera, "PixelFormat");
  Pylon::PixelType pt = Pylon::CPixelTypeMapper::GetPylonPixelTypeByName(tStr);
  if(pt == Pylon::PixelType_Undefined){
    DEBUG_LOG("PixelType " << tStr.c_str() << "=" << pt << "' is not defined.");
  }
  DEBUG_LOG("Camera PixelType is " << tStr.c_str() << " (" << pt << ")")
  return pt;
}

int getCameraPixelSize(Pylon::IPylonDevice* camera){
  return Pylon::BitPerPixel(getCameraPixelType(camera));
}

long getNeededBufferSize(Pylon::IPylonDevice* camera){
  try {
    return getParameterValueOf
      <Pylon::IPylonDevice, GenApi::IInteger, int>(camera, "PayloadSize");
  } catch (ICLException &e){
    DEBUG_LOG("The camera does not have a parameter called PayloadSize.")
    DEBUG_LOG("Assuming that the image size is width * height * pixelsize.")
    int height = getParameterValueOf
      <Pylon::IPylonDevice, GenApi::IInteger, int>(camera, "Height");
    int width = getParameterValueOf
      <Pylon::IPylonDevice, GenApi::IInteger, int>(camera, "Width");
    return (long) ((width * height * getCameraPixelSize(camera) / 8) + 0.5);
  }
}
