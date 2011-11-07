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

#include <unistd.h>

#include <pylon/PylonIncludes.h>
#include <pylon/TransportLayer.h>

#define USE_SMALL_PICTURES

using namespace icl;

static unsigned int pylon_env_inits = 0;
static icl::Mutex* env_mutex = new icl::Mutex();

enum icl_val_type {
  range,
  value_list,
  menu,
  command,
  info
} icl_val_type;

const char *icl_val_str[]={ "range", "value-list", "menu", "command", "info" };

const int default_sizes_count = 30;
const char *default_sizes[]={
      "0x0", "128x96", "176x144", "160x120", "320x200", "320x240",
      "352x288", "360x240", "480x320", "640x350", "640x480", "768x576",
      "800x480", "800x600", "960x540", "960x640", "1024x768", "1152x864",
      "1200x800", "1280x720", "1280x800", "1440x900", "1280x960",
      "1280x1024", "1600x900", "1400x1050", "1600x1050", "1600x1200",
      "1920x1080", "3840x2160"
};

// Constructor allocates the image buffer
PylonGrabberBuffer::PylonGrabberBuffer(const size_t imageSize):
m_pBuffer(NULL)
{
  m_pBuffer = new uint8_t[imageSize];
  if (m_pBuffer == NULL)
  {
    throw icl::ICLException("Not enough memory to allocate image buffer");
  }
}

// Freeing the memory
PylonGrabberBuffer::~PylonGrabberBuffer()
{
  if (m_pBuffer != NULL)
    delete[] m_pBuffer;
}

// Constructor of PylonGrabberImpl
PylonGrabberImpl::PylonGrabberImpl(const Pylon::CDeviceInfo &dev) : m_CamMutex(), m_Aquired(0), m_Error(0){
  // getting camera mutex to exclude race-conditions
  icl::Mutex::Locker l(m_CamMutex);
  // Initialization of the pylon Runtime Library
  initPylonEnv();
  Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();
  
  m_Camera = tlFactory.CreateDevice(dev);

  if(m_Camera -> GetNumStreamGrabberChannels() == 0){
    throw icl::ICLException("No stream grabber channels avaliable!");
  }

  m_Camera -> Open();
  cameraDefaultSettings();
  // getting first Grabber
  m_Grabber = m_Camera -> GetStreamGrabber(0);

  m_Grabber -> Open();

  prepareGrabbing();

  // Let the camera acquire images
  acquisitionStart();
}

void PylonGrabberImpl::prepareGrabbing(){
  // Get the image buffer size
  const size_t imageSize = getNeededBufferSize();
  unsigned char* ppBuffers[m_NumBuffers];
  Pylon::StreamBufferHandle handles[m_NumBuffers];

  // We won't use image buffers greater than ImageSize
  setParameterValueOf<Pylon::IStreamGrabber, GenApi::IInteger, int>(m_Grabber, "MaxBufferSize", imageSize);
  // We won't queue more than c_nBuffers image buffers at a time
  setParameterValueOf<Pylon::IStreamGrabber, GenApi::IInteger, int>(m_Grabber, "MaxNumBuffer", m_NumBuffers);

  // Allocate all resources for grabbing. Critical parameters like image
  // size now must not be changed until FinishGrab() is called.
  m_Grabber -> PrepareGrab();

  // Buffers used for grabbing must be registered at the stream grabber -> 
  // The registration returns a handle to be used for queuing the buffer.
  for (int i = 0; i < m_NumBuffers; ++i){
    PylonGrabberBuffer *pGrabBuffer = new PylonGrabberBuffer(imageSize);
    pGrabBuffer -> setBufferHandle(m_Grabber -> RegisterBuffer(pGrabBuffer -> getBufferPointer(), imageSize));

    // Put the grab buffer object into the buffer list
    m_BufferList.push_back(pGrabBuffer);
  }

  for (std::vector<PylonGrabberBuffer*>::const_iterator it = m_BufferList.begin(); it != m_BufferList.end(); ++it){
    // Put buffer into the grab queue for grabbing
    m_Grabber -> QueueBuffer((*it) -> getBufferHandle(), NULL);
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
  std::vector<PylonGrabberBuffer*>::iterator it;
  for (it = m_BufferList.begin(); it != m_BufferList.end(); it++){
    m_Grabber -> DeregisterBuffer((*it) -> getBufferHandle());
    delete *it;
    *it = NULL;
  }
  std::cout << "deregistered buffers" << std::endl;
  // Free all resources used for grabbing
  m_Grabber -> FinishGrab();
}

void PylonGrabberImpl::acquisitionStart(){
  GenApi::ICommand* node = (GenApi::ICommand*) m_Camera -> GetNodeMap() -> GetNode("AcquisitionStart");
  node -> Execute();
}

void PylonGrabberImpl::acquisitionStop(){
  GenApi::ICommand* node = (GenApi::ICommand*) m_Camera -> GetNodeMap() -> GetNode("AcquisitionStop");
  node -> Execute();
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
  termPylonEnv();
}

long PylonGrabberImpl::getNeededBufferSize(){
  try {
    return getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "PayloadSize");
  } catch (ICLException &e){
    std::cerr << "The camera does not have a parameter calles PayloadSize." << std::endl;
    std::cerr << "Assuming that the image size is width * height." << std::endl;
    return m_Width * m_Height;
  }
}

template <typename NODE, typename RET>
RET PylonGrabberImpl::getNodeValue(NODE* node){
  return node -> GetValue();
}

template <typename NODE, typename RET>
std::string PylonGrabberImpl::getNodeValue(GenApi::IEnumeration* node){
  return (node -> ToString()).c_str();
}

template <typename NODE, typename VAL>
void PylonGrabberImpl::setNodeValue(NODE* node, VAL value){
  node -> SetValue(value, true);
  return;
}

template <typename NODE, typename VAL>
void PylonGrabberImpl::setNodeValue(GenApi::IEnumeration* node, std::string value){
  node -> FromString(value.c_str(), true);
  return;
}

//template function to set parameters of camera and grabber
template <typename OBJ, typename NODE, typename VAL>
bool PylonGrabberImpl::setParameterValueOf(OBJ* object, std::string parameter, VAL value){
  GenApi::INode* node = object -> GetNodeMap() -> GetNode(parameter.c_str());
  if (!node) {
    std::cerr << "There is no parameter called '" << parameter << "'" << std::endl;
    return false;
  }
  //dynamic cast to needed node-type
  NODE* node2;
  try{
    node2 = dynamic_cast<NODE*>(node);
  } catch (std::exception &e){
    std::cerr << "Could not cast node '"<< parameter << "' to desired type" << std::endl;
    return false;
  }
  if(!GenApi::IsWritable(node2)){
    std::cerr << "Parameter called '" << parameter << "' is not writable." << std::endl;
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

//template function to get parameter values of camera and grabber
template <typename SOURCE, typename NODE, typename RET>
RET PylonGrabberImpl::getParameterValueOf(SOURCE* source, std::string param){
  GenApi::INode* node = source -> GetNodeMap() -> GetNode(param.c_str());
  if(!node){
    throw icl::ICLException("parameter of this type not found");
  }
  //dynamic cast to needed node-type
  try{
    NODE* node2 = dynamic_cast<NODE*>(node);
    if(!GenApi::IsReadable(node2)){
      throw icl::ICLException("The node " + param + " is not Readable");
    }
    return getNodeValue<NODE, RET>(node2);
  } catch (std::exception &e){
    throw icl::ICLException(e.what());
  }
}

std::string PylonGrabberImpl::getParameterValueString(std::string parameter){
  GenApi::INode* node = m_Camera -> GetNodeMap() -> GetNode(parameter.c_str());
  if (!node) {
    std::cerr << "There is no parameter called '" << parameter << "'" << std::endl;
    return NULL;
  }
  GenApi::IValue* value = dynamic_cast<GenApi::IValue*>(node);
  return (value -> ToString()).c_str();
}

void PylonGrabberImpl::cameraDefaultSettings(){
  m_Height = getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Height");
  m_Width = getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Width");
  m_Offsetx = getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "OffsetX");
  m_Offsety = getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "OffsetY");
  m_Format = getParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>(m_Camera, "PixelFormat");
  std::cout << "ImageFormat at startup: " << m_Width << "x" << m_Height << " , " << m_Format << std::endl;
  std::cout << "Payload: " << getParameterValueString("PayloadSize") << std::endl;

#ifdef USE_SMALL_PICTURES
  m_Height = 480;
  m_Width = 640;
  m_Offsetx = 640;
  m_Offsety = 300;
  m_Format = "Mono8";
#endif
 
  // default camera settings: image format, AOI, acquisition mode and exposure
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
    (m_Camera, "PixelFormat", m_Format);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "OffsetX", 0);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "OffsetY", 0);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Width", m_Width);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Height", m_Height);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "PacketSize", 1400);
  std::cout << "ImageFormat after settings: " 
  << getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Width") << "x" 
  << getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(m_Camera, "Height") << " , " 
  << getParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>(m_Camera, "PixelFormat")
  << std::endl;
  std::cout << "Payload: " << getParameterValueString("PayloadSize") << std::endl;

  // default Aquisition Mode
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
    (m_Camera, "AcquisitionMode", "Continuous");
}

const icl::ImgBase* PylonGrabberImpl::acquireImage(){
  if(m_Error == 5){
    DEBUG_LOG("setting size")
    setProperty("size", "600x400");
    DEBUG_LOG("size set")
  }
  // getting camera mutex so camera will not be stopped
  icl::Mutex::Locker l(m_CamMutex);
  // Wait for the grabbed image with timeout of 3 seconds
  if (!m_Grabber -> GetWaitObject().Wait(500)){
    // Timeout
    std::cerr << "Timeout occurred!" << std::endl;
    ++m_Error;
    return NULL;
  }
  // Get the grab result from the grabber's result queue
  Pylon::GrabResult result;
  m_Grabber -> RetrieveResult(result);

  if (result.Succeeded()){
    ++m_Aquired;
    // Grabbing was successful, process image
    const uint8_t *pImageBuffer = (uint8_t *) result.Buffer();
    icl::Img8u* byteImage = 
      new icl::Img8u(icl::Size(m_Width, m_Height), icl::formatGray);

    for(int c=0;c<byteImage -> getChannels();++c){
      for(int x=0;x<byteImage -> getWidth();++x){
        for(int y=0;y<byteImage -> getHeight();++y){
           (*byteImage)(x,y,c) = pImageBuffer[m_Width*y + x];
        }
      }
    }

    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);

    return byteImage;

  } else {
    ++m_Error;
    // Error handling
    //std::cerr << "No image acquired!" << "Error description : "
    //<< result.GetErrorDescription() << std::endl;

    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
    return NULL;
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
void PylonGrabberImpl::setProperty(const std::string &property, const std::string &value){
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
  
  // need to stop Acquisition when setting Properties
  AcquisitionInterruptor a(this);
  
  GenApi::INodeMap* nodeMap = m_Camera -> GetNodeMap();
  GenApi::CValuePtr node = nodeMap -> GetNode(property.c_str());
  if (!node) {
    std::cerr << "There is no parameter called '" << property << "'" << std::endl;
    return;
  }

  bool stopped = false;

  if (!GenApi::IsWritable(node)){
    std::cerr << "The parameter '" << property << "' is not writable" << std::endl;
    std::cerr << "Stopping acquisition" << std::endl;
    finishGrabbing();
    stopped = true;
    if(!GenApi::IsWritable(node)){
      std::cerr << "The parameter '" << property << "' is still not writable" << std::endl;
      prepareGrabbing();
      return;
    }
  }
    
  node -> FromString(value.c_str(), true);
  
  if(stopped) {
    prepareGrabbing(); 
  }
}

// returns a list of properties, that can be set using setProperty
std::vector<std::string> PylonGrabberImpl::getPropertyList(){
  std::cout << "getPropertyList()" << std::endl;
  std::vector<std::string> ps;
  ps.push_back("size");
  ps.push_back("format");
  GenApi::INode* rootNode = m_Camera -> GetNodeMap() -> GetNode("Root");
  addToPropertyList(ps, rootNode);
  std::cout << "properties: " << ps.size() << std::endl;
  for (int i = 0 ; i < ps.size(); ++i){
    std::cout << ps.at(i) << ", ";
  }
  ps.resize(35);
  return ps;
}

// helper function for getPropertyList
void PylonGrabberImpl::addToPropertyList(std::vector<std::string> &ps, const GenApi::CNodePtr& node){
  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) { 
    case GenApi::intfIInteger:
    case GenApi::intfIFloat:
    case GenApi::intfIBoolean:
    case GenApi::intfIEnumeration:
      if (GenApi::IsImplemented(node)){
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
  std::cout << "supportsProperty(" << property << ")" << std::endl;
  if((property.compare("size") == 0) || (property.compare("format") == 0)){
    return true;
  }
  // Check whether node exists
  GenApi::INode* node = m_Camera -> GetNodeMap() -> GetNode(property.c_str());
  if(!node) return false;

  // Check whether property is implemented
  if(!GenApi::IsImplemented(node)) return false;
  
  // Check whether property is writable
  if(!GenApi::IsWritable(node)) return false;

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
  std::cout << "getType(" << name << ")" << std::endl;
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
  std::cout << "getInfo(" << name << ")" << std::endl;
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
      if(inode = dynamic_cast<GenApi::IInteger*>(node)) {
        ret << "[" << inode -> GetMin() << "," << inode -> GetMax() << "]:" << inode -> GetInc();
      }
      break;

    case GenApi::intfIFloat:
      GenApi::IFloat* fnode;
      if(fnode = dynamic_cast<GenApi::IFloat*>(node)) {
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
      if(enode = dynamic_cast<GenApi::IEnumeration*>(node)) {
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
  std::cout << "getValue(" << name << ")" << std::endl;
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
    std::cerr << "There is no parameter called '" 
      << name << "'" << std::endl;
    return "";
  }

  GenApi::IValue* val;
  if(val = dynamic_cast<GenApi::IValue*>(node)){
    return (val -> ToString()).c_str();
  }

  std::cerr << "There is no node named '" << name << "'" 
    << " could not be cast to 'IValue'." << std::endl;
  return "";

}

// Returns whether this property may be changed internally. 
int PylonGrabberImpl::isVolatile(const std::string &propertyName){
  // find out which.
  return true;
}

// initializes the Pylon environment (returns whether PylonInitialize() was called)
bool PylonGrabberImpl::initPylonEnv(){
  icl::Mutex::Locker l(env_mutex);
  ICLASSERT(pylon_env_inits >= 0)
  pylon_env_inits++;
  if(pylon_env_inits == 1){
    Pylon::PylonInitialize();
    return true;
  } else {
    return false;
  }
}

// terminates the Pylon environment (returns whether PylonTerminate() was called).
bool PylonGrabberImpl::termPylonEnv(){
  icl::Mutex::Locker l(env_mutex);
  ICLASSERT(pylon_env_inits > 0)
  pylon_env_inits--;
  if(pylon_env_inits == 0){
    Pylon::PylonTerminate();
    return true;
  } else {
    return false;
  }
}

