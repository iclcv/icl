/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PylonGrabber.cpp                       **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/Exception.h>
#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLIO/PylonGrabber.h>
#include <ICLUtils/Macros.h>

using namespace icl;
using namespace icl::io::pylon;

// Constructor of PylonGrabberImpl
PylonGrabber::PylonGrabber(
    const Pylon::CDeviceInfo &dev, const std::string args)
  : m_ImgMutex(), m_PylonEnv(), m_LastBuffer(NULL)
{
  FUNCTION_LOG("args: " << args);
  utils::Mutex::Locker l(m_ImgMutex);
  // Initialization of the pylon Runtime Library
  m_Camera = Pylon::CTlFactory::GetInstance().CreateDevice(dev);

  unsigned int channel = channelFromArgs(args);
  if(m_Camera -> GetNumStreamGrabberChannels() == 0){
    throw utils::ICLException("No stream grabber channels avaliable.");
  } else if(m_Camera -> GetNumStreamGrabberChannels() < channel){
    DEBUG_LOG("From args='" << args << "' demanded channel=" << channel <<
              "but available=" << m_Camera -> GetNumStreamGrabberChannels());
    throw utils::ICLException("Demanded StreamGrabberChannel not avaliable.");
  }

  m_Camera -> Open();
  cameraDefaultSettings();
  // getting first Grabber
  m_Grabber = m_Camera -> GetStreamGrabber(channel);
  m_Grabber -> Open();

  m_CameraOptions = new PylonCameraOptions(m_Camera, this);
  Configurable::addChildConfigurable(m_CameraOptions);
  m_ColorConverter = new PylonColorConverter();
  m_GrabberThread = new PylonGrabberThread(
        m_Grabber, m_ColorConverter, m_CameraOptions);
  // prepare grabbing
  grabbingStart();
  // Let the camera acquire images
  m_CameraOptions -> acquisitionStart();
  m_GrabberThread -> start();
}

PylonGrabber::~PylonGrabber(){
  FUNCTION_LOG("");
  // Stop acquisition
  acquisitionStop();
  // deregister buffers
  grabbingStop();
  // Close stream grabber
  m_Grabber -> Close();
  // Close camera
  m_Camera -> Close();
  ICL_DELETE(m_ColorConverter);
  ICL_DELETE(m_CameraOptions);
  ICL_DELETE(m_GrabberThread);
  // Free resources allocated by the pylon runtime system automated.
}


void PylonGrabber::grabbingStart(){
  FUNCTION_LOG("");
  // Get the image buffer size
  const size_t imageSize = m_CameraOptions -> getNeededBufferSize();
  DEBUG_LOG("Buffer size: " << imageSize);

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
    Pylon::StreamBufferHandle handle =
        m_Grabber -> RegisterBuffer(pGrabBuffer -> getBufferPointer(), imageSize);
    pGrabBuffer -> setBufferHandle(handle);

    // Put the grab buffer object into the buffer list
    m_BufferList.push_back(pGrabBuffer);

    // Put buffer into the grab queue for grabbing
    m_Grabber -> QueueBuffer(handle);
  }
  m_GrabberThread -> resetBuffer();
  m_ColorConverter -> resetConversion(
        m_CameraOptions -> getWidth(),
        m_CameraOptions -> getHeight(),
        m_CameraOptions -> getCameraPixelSize(),
        m_CameraOptions -> getNeededBufferSize(),
        m_CameraOptions -> getCameraPixelType(),
        m_CameraOptions -> getFormatString()
        );
}

void PylonGrabber::grabbingStop(){
  FUNCTION_LOG("");
  m_Grabber -> CancelGrab();
  Pylon::GrabResult result;
  while (m_Grabber -> GetWaitObject().Wait(0)) {
    if (!m_Grabber -> RetrieveResult(result)) {
      DEBUG_LOG("Failed to retrieve item from output queue");
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

void PylonGrabber::acquisitionStart(){
  FUNCTION_LOG("");
  m_CameraOptions -> acquisitionStart();
  m_GrabberThread -> start();
  m_ImgMutex.unlock();
}

void PylonGrabber::acquisitionStop(){
  FUNCTION_LOG("");
  m_ImgMutex.lock();
  m_GrabberThread -> stop();
  m_CameraOptions -> acquisitionStop();
}

void PylonGrabber::cameraDefaultSettings(){
  // activate timestamp-chunks
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IBoolean, bool>
      (m_Camera, "ChunkModeActive", true);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
      (m_Camera, "ChunkSelector", "Timestamp");
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IBoolean, bool>
      (m_Camera, "ChunkEnable", true);
  // set PacketSize
  //setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
  //(m_Camera, "GevSCPSPacketSize", 8192);
}

const core::ImgBase* PylonGrabber::acquireImage(){
  core::ImgBase* ret = NULL;
  int counter = 0;
  while(1){
    // lock image lock so buffers are safe till release.
    m_ImgMutex.lock();
    // Get the image from the grabber thread
    ret = m_GrabberThread -> getCurrentImage();
    if(m_CameraOptions-> omitDoubleFrames() && ret == m_LastBuffer
       && counter <= 1000)
    {
      // old frame release and sleep
      m_ImgMutex.unlock();
      ret = NULL;
      ++counter;
      utils::Thread::msleep(1);
    } else {
      m_ImgMutex.unlock();
      break;
    }
  }
  m_LastBuffer = ret;
  return ret;
}

icl::io::Grabber* createGrabber(const std::string &param){
  Pylon::CDeviceInfo dev = getDeviceFromArgs(param);
  return new io::pylon::PylonGrabber(dev, param);
}

const std::vector<io::GrabberDeviceDescription>& getPylDeviceList(std::string hint, bool rescan){
  static std::vector<io::GrabberDeviceDescription> deviceList;
  if(rescan){
    deviceList.clear();
    Pylon::DeviceInfoList_t devs = getPylonDeviceList();
    for(unsigned int i = 0 ; i < devs.size() ; ++i){
      deviceList.push_back(
            io::GrabberDeviceDescription(
              "pylon",
              utils::str(i), // + "|||" + devs.at(i).GetFullName().c_str(),
              devs.at(i).GetFullName().c_str()
              )
            );
    }
  }
  return deviceList;
}

REGISTER_GRABBER(pylon,utils::function(createGrabber),utils::function(getPylDeviceList), "pylon:camera ID or IP-address:Basler Pylon based gigabit-ethernet (GIG-E) camera source");
