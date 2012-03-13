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

using namespace icl;
using namespace icl::pylon;

// Constructor of PylonGrabberImpl
PylonGrabberImpl::PylonGrabberImpl(
        const Pylon::CDeviceInfo &dev, const std::string args)
    : m_CamMutex(), m_PylonEnv()
{
  DEBUG_LOG("args: " << args)
  // getting camera mutex to exclude race-conditions
  icl::Mutex::Locker l(m_CamMutex);
  // Initialization of the pylon Runtime Library
  m_Camera = Pylon::CTlFactory::GetInstance().CreateDevice(dev);

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

  m_CameraOptions = new PylonCameraOptions(m_Camera, this);
  m_ColorConverter = new PylonColorConverter();
  m_GrabberThread = new PylonGrabberThread(m_Grabber, &m_CamMutex);
  // prepare grabbing
  grabbingStart();
  // Let the camera acquire images
  ///
  if(0){
    acquisitionStart();
  } else  {
    m_CameraOptions -> acquisitionStart();
    m_GrabberThread -> start();
  }
  DEBUG_LOG("ready")
  DEBUG_LOG("cam mutex = " << &m_CamMutex)
}

PylonGrabberImpl::~PylonGrabberImpl(){
  // getting camera mutex to exclude race-conditions
  icl::Mutex::Locker l(m_CamMutex);

  DEBUG_LOG("Pylongrabber destructor called");

  // Stop acquisition
  if (0){
   acquisitionStop();
  } else {
      m_GrabberThread -> stop();
      m_CameraOptions -> acquisitionStop();
  }
  // deregister buffers
  grabbingStop();

  // Close stream grabber
  m_Grabber -> Close();
  // Close camera
  m_Camera -> Close();
  if (m_ColorConverter) delete m_ColorConverter;
  if (m_CameraOptions) delete m_CameraOptions;
  if (m_GrabberThread) delete m_GrabberThread;
  // Free resources allocated by the pylon runtime system automated.
}


void PylonGrabberImpl::grabbingStart(){
  DEBUG_LOG("grab start called")
  // Get the image buffer size
  const size_t imageSize = m_CameraOptions -> getNeededBufferSize();
  DEBUG_LOG("Buffer size: " << imageSize)

  // We won't use image buffers greater than imageSize
  setParameterValueOf<Pylon::IStreamGrabber, GenApi::IInteger, int>
    (m_Grabber, "MaxBufferSize", imageSize);

  // We won't queue more than m_NumBuffers image buffers at a time
  setParameterValueOf<Pylon::IStreamGrabber, GenApi::IInteger, int>
    (m_Grabber, "MaxNumBuffer", m_NumBuffers);

  // Allocate all resources for grabbing. Critical parameters like image
  // size now must not be changed until finishGrab() is called.
  m_Grabber -> PrepareGrab();
  // Buffers used for grabbing must be registered at the stream grabber -> 
  // The registration returns a handle to be used for queuing the buffer.
  for (int i = 0; i < m_NumBuffers; ++i){
    PylonGrabberBuffer<uint16_t> *pGrabBuffer =
      new PylonGrabberBuffer<uint16_t>(imageSize);
    if (!pGrabBuffer) DEBUG_LOG("pgrabBuffer is null")
    Pylon::StreamBufferHandle handle =
      m_Grabber -> RegisterBuffer(pGrabBuffer -> getBufferPointer(), imageSize);
    pGrabBuffer -> setBufferHandle(handle);

    // Put the grab buffer object into the buffer list
    m_BufferList.push_back(pGrabBuffer);

    // Put buffer into the grab queue for grabbing
    m_Grabber -> QueueBuffer(handle);
  }
  m_GrabberThread -> resetBuffer(imageSize, m_ThreadNumBuffers);
  m_ColorConverter -> resetConversion(
              m_CameraOptions -> getWidth(),
              m_CameraOptions -> getHeight(),
              m_CameraOptions -> getCameraPixelType(),
              m_CameraOptions -> getCameraPixelSize(),
              m_CameraOptions -> getNeededBufferSize()
              );
}

void PylonGrabberImpl::grabbingStop(){
  DEBUG_LOG("grab stop called")
  m_Grabber -> CancelGrab();
  Pylon::GrabResult result;
  while (m_Grabber -> GetWaitObject().Wait(0)) {
    if (!m_Grabber -> RetrieveResult(result)) {
      std::cerr << "Failed to retrieve item from output queue" << std::endl;
    }
  }
  // deregister the buffers before freeing the memory
  while (!m_BufferList.empty()){
    m_Grabber -> DeregisterBuffer(m_BufferList.back() -> getBufferHandle());
    PylonGrabberBuffer<uint16_t>* tmp = m_BufferList.back();
    delete tmp;
    m_BufferList.pop_back();
  }
  // Free all resources used for grabbing
  m_Grabber -> FinishGrab();
}

void PylonGrabberImpl::acquisitionStart(){
  DEBUG_LOG("acquire start called")
  m_CameraOptions -> acquisitionStart();
  DEBUG_LOG("thread start")
  m_GrabberThread -> start();
  DEBUG_LOG("thread started")
  m_GrabberThread -> m_BufferMutex.unlock();
  DEBUG_LOG("buffer unlocked")
  m_CamMutex.unlock();
  DEBUG_LOG("cam unlocked")
}

void PylonGrabberImpl::acquisitionStop(){
  DEBUG_LOG("locking cam")
  m_CamMutex.lock();
  DEBUG_LOG("acquire stop called, stopping thread..." << m_GrabberThread->pos)
  m_GrabberThread -> stop();
  DEBUG_LOG("thread stopped")
  m_GrabberThread -> m_BufferMutex.lock();
  DEBUG_LOG("buffer locked")
  m_CameraOptions -> acquisitionStop();
}

void PylonGrabberImpl::cameraDefaultSettings(){

  setParameterValueOf<Pylon::IPylonDevice, GenApi::IBoolean, bool>
    (m_Camera, "ChunkModeActive", true);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
    (m_Camera, "ChunkSelector", "Timestamp");
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IBoolean, bool>
    (m_Camera, "ChunkEnable", true);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
    (m_Camera, "GevSCPSPacketSize", 8000);

  //m_Height = 480;
  //m_Width = 640;
  //m_Offsetx = 640;
  //m_Offsety = 300;
  //m_Format = "BayerGB16";

  //DEBUG_LOG("setting camera to " << m_Format)
  // default camera settings: image format, AOI, acquisition mode and exposure
  //setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
  //  (m_Camera, "PixelFormat", m_Format);
  //setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
  //  (m_Camera, "OffsetX", m_Offsetx);
  //setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
  //  (m_Camera, "OffsetY", m_Offsety);
  //setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
  //  (m_Camera, "Width", m_Width);
  //setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
  //  (m_Camera, "Height", m_Height);
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
  //setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
  //  (m_Camera, "AcquisitionMode", "Continuous");
}

const icl::ImgBase* PylonGrabberImpl::acquireImage(){//TODO: Threading issues
  // Get the grab result from the grabber thread
  // getCurrentImage is Thread safe
  if(TsBuffer<int16_t>* tmp = m_GrabberThread -> getCurrentImage()){
    icl::ImgBase* ptr = m_ColorConverter -> convert(tmp -> m_Buffer);
    return ptr;
  } else {
    return NULL;
  }
}

// interface for the setter function for video device properties
void PylonGrabberImpl::setProperty(
  const std::string &property, const std::string &value)
{
  m_CameraOptions -> setProperty(property, value);
}

// returns a list of properties, that can be set using setProperty
std::vector<std::string> PylonGrabberImpl::getPropertyList(){
  return m_CameraOptions -> getPropertyList();
}

// base implementation for property check (seaches in the property list)
bool PylonGrabberImpl::supportsProperty(const std::string &property){
  return m_CameraOptions -> supportsProperty(property);
}

// get type of property
std::string PylonGrabberImpl::getType(const std::string &name){
  return m_CameraOptions -> getType(name);
}

// get information of a properties valid values
std::string PylonGrabberImpl::getInfo(const std::string &name){
  return m_CameraOptions -> getInfo(name);
}

// returns the current value of a property or a parameter
std::string PylonGrabberImpl::getValue(const std::string &name){
  return m_CameraOptions -> getValue(name);
}

// Returns whether this property may be changed internally.
int PylonGrabberImpl::isVolatile(const std::string &propertyName){
  return m_CameraOptions -> isVolatile(propertyName);
}
