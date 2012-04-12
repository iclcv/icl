/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

//#define SPEED_TEST

// Constructor of PylonGrabberImpl
PylonGrabberImpl::PylonGrabberImpl(
        const Pylon::CDeviceInfo &dev, const std::string args)
    : m_ImgMutex(), m_PylonEnv(), m_LastBuffer(NULL)
{
  FUNCTION_LOG("args: " << args)
  icl::Mutex::Locker l(m_ImgMutex);
  // Initialization of the pylon Runtime Library
  m_Camera = Pylon::CTlFactory::GetInstance().CreateDevice(dev);

  unsigned int channel = channelFromArgs(args);
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
  m_GrabberThread = new PylonGrabberThread(
                            m_Grabber, m_ColorConverter, m_CameraOptions);
  // prepare grabbing
  grabbingStart();
  // Let the camera acquire images
  m_CameraOptions -> acquisitionStart();
  m_GrabberThread -> start();
}

PylonGrabberImpl::~PylonGrabberImpl(){
  FUNCTION_LOG();
  // Stop acquisition
  acquisitionStop();
  // deregister buffers
  grabbingStop();
  // Close stream grabber
  m_Grabber -> Close();
  // Close camera
  m_Camera -> Close();
  ICL_DELETE(m_ColorConverter)
  ICL_DELETE(m_CameraOptions)
  ICL_DELETE(m_GrabberThread)
  // Free resources allocated by the pylon runtime system automated.
}


void PylonGrabberImpl::grabbingStart(){
  FUNCTION_LOG()
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

void PylonGrabberImpl::grabbingStop(){
  FUNCTION_LOG()
  m_Grabber -> CancelGrab();
  Pylon::GrabResult result;
  while (m_Grabber -> GetWaitObject().Wait(0)) {
    if (!m_Grabber -> RetrieveResult(result)) {
      DEBUG_LOG("Failed to retrieve item from output queue")
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
  FUNCTION_LOG()
  m_CameraOptions -> acquisitionStart();
  m_GrabberThread -> start();
  m_ImgMutex.unlock();
}

void PylonGrabberImpl::acquisitionStop(){
  FUNCTION_LOG()
  m_ImgMutex.lock();
  m_GrabberThread -> stop();
  m_CameraOptions -> acquisitionStop();
}

void PylonGrabberImpl::cameraDefaultSettings(){
  // activate timestamp-chunks
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IBoolean, bool>
    (m_Camera, "ChunkModeActive", true);
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IEnumeration, std::string>
    (m_Camera, "ChunkSelector", "Timestamp");
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IBoolean, bool>
    (m_Camera, "ChunkEnable", true);
  // set PacketSize
  setParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>
    (m_Camera, "GevSCPSPacketSize", 8000);
}

#ifdef SPEED_TEST
Time aqtimesum = Time();
Time cotimesum = Time();
const int iterations = 100;
int timesumcount = iterations;
int step = 0;
std::string last = "";
const std::string colors[] = {"BayerGB12Packed",
                        "BayerGB16","BayerGB8",
                        "Mono8","YUV422Packed",
                        "YUV422_YUYV_Packed"};
const int colorc = 6;
std::vector<std::pair<std::string, std::pair<Time,Time> >* > times
    = std::vector<std::pair<std::string, std::pair<Time,Time> >* >();

std::string reset(PylonCameraOptions* opt, std::string color, int omit){
  if(omit){
    opt -> setProperty("OmitDoubleFrames", "true");
  } else {
    opt -> setProperty("OmitDoubleFrames", "false");
  }
  opt -> setProperty("PixelFormat", color);

  aqtimesum = Time();
  cotimesum = Time();
  timesumcount = 0;
  std::ostringstream ret;
  ret << "color: " << color << " omit: " << omit;
  return ret.str();
}


void test(PylonCameraOptions* opt){
  if(timesumcount >= iterations){
     if(step == 0){
      last = reset(opt, colors[0], true);
      ++step;
    } else if(step < colorc){
      std::pair<std::string, std::pair<Time,Time> > * time
                        = new std::pair<std::string, std::pair<Time,Time> > ();

      time->first = last;
      time->second.first =  aqtimesum / timesumcount;
      time->second.second =  cotimesum / timesumcount;
      last = reset(opt, colors[step], true);
      times.push_back(time);
      ++step;
    } else if(step == colorc){
      //end
      std::cout << std::endl;
      for(int i = 0; i < times.size(); ++i){
        std::pair<std::string, std::pair<Time,Time> > * time = times.at(i);
        std::cout << time->first << ":\n\t aq: " << time->second.first
        << "\t co: " << time->second.second << std::endl;
      }
      std::cout << std::endl;
      exit(0);
    }
  }
}

#endif

const icl::ImgBase* PylonGrabberImpl::acquireImage(){
#ifdef SPEED_TEST
  Time s = Time::now();
#endif
  ImgBase* ret = NULL;
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
      Thread::msleep(1);
    } else {
      m_ImgMutex.unlock();
      break;
    }
  }
  m_LastBuffer = ret;
#ifdef SPEED_TEST
  aqtimesum += (s.age());
  if(timesumcount >= 100){
    std::cout << "\n\n aquisition: " << (aqtimesum/timesumcount) << std::endl;
    aqtimesum = Time();
    timesumcount = 0;
  } else {
    ++timesumcount;
  }
#endif
  return ret;
}

// setter function for video device properties
void PylonGrabberImpl::setProperty(
  const std::string &property, const std::string &value)
{
  m_CameraOptions -> setProperty(property, value);
}

// returns a list of properties, that can be set using setProperty
std::vector<std::string> PylonGrabberImpl::getPropertyList(){
  return m_CameraOptions -> getPropertyList();
}

// checks if property is returned, implemented, available and of processable GenApi::EInterfaceType
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
