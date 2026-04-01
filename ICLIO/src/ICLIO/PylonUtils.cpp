// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#include <ICLIO/PylonUtils.h>
#include <ICLUtils/StringUtils.h>
#include <mutex>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::io::pylon;

//static int inits = 0;

// Static mutex and counter for clean initialising
// and deleting of Pylon environment
static unsigned int pylon_env_inits = 0;
static std::recursive_mutex* env_mutex = new std::recursive_mutex();

// Constructor sets all pointers to NULL
ConvBuffers::ConvBuffers(){
  m_Image = nullptr;
  m_ImageBuff = nullptr;
  m_ImageBuff16 = nullptr;
  m_ImageRGBA = nullptr;
  m_Channels = nullptr;
  m_Channels16 = nullptr;
  m_Reset = true;
}

// calls free
ConvBuffers::~ConvBuffers(){
  free();
}

// deletes Objects pointed at, when pointer != NULL
void ConvBuffers::free(){
  ICL_DELETE(m_Image)
  ICL_DELETE_ARRAY(m_ImageBuff)
  ICL_DELETE_ARRAY(m_ImageBuff16)
  ICL_DELETE(m_ImageRGBA)
  ICL_DELETE(m_Channels)
  ICL_DELETE(m_Channels16)
}

// Constructor creates and initializes resources.
ConcGrabberBuffer::ConcGrabberBuffer() :
m_Mutex(), m_Write(0), m_Next(1), m_Read(2) {
  std::lock_guard<std::recursive_mutex> l(m_Mutex);
  m_Buffers[0] = new ConvBuffers();
  m_Buffers[1] = new ConvBuffers();
  m_Buffers[2] = new ConvBuffers();
}

// Destructor frees memory
ConcGrabberBuffer::~ConcGrabberBuffer() {
  std::lock_guard<std::recursive_mutex> l(m_Mutex);
  ICL_DELETE(m_Buffers[0]);
  ICL_DELETE(m_Buffers[1]);
  ICL_DELETE(m_Buffers[2]);
}

// returns a pointer to the most recent actualized ConvBuffers.
ConvBuffers* ConcGrabberBuffer::getNextReadBuffer(){
  std::lock_guard<std::recursive_mutex> l(m_Mutex);
  if(m_Avail){
    // new buffer is available.
    std::swap(m_Next, m_Read);
    m_Avail = false;
  }
  return m_Buffers[m_Read];
}

// returns a pointer to the next write ConvBuffers.
ConvBuffers* ConcGrabberBuffer::getNextWriteBuffer(){
  std::lock_guard<std::recursive_mutex> l(m_Mutex);
  // swap write buffer and next buffer.
  std::swap(m_Next, m_Write);
  // new buffer is available for reading.
  m_Avail = true;
  // return new write buffer.
  return m_Buffers[m_Write];
}

// mark ConvBuffers to be reset on next write-access.
void ConcGrabberBuffer::setReset(){
  std::lock_guard<std::recursive_mutex> l(m_Mutex);
  m_Buffers[0] -> m_Reset = true;
  m_Buffers[1] -> m_Reset = true;
  m_Buffers[2] -> m_Reset = true;
}

// tells whether a new image is available
bool ConcGrabberBuffer::newAvailable(){
  std::lock_guard<std::recursive_mutex> l(m_Mutex);
  return m_Avail;
}

// Initializes Pylon environment if not already done.
PylonAutoEnv::PylonAutoEnv(){
    PylonAutoEnv::initPylonEnv();
}
// Terminates Pylon environment when (calls to term) == (calls to init).
PylonAutoEnv::~PylonAutoEnv(){
    PylonAutoEnv::termPylonEnv();
}

// initializes the Pylon environment
// (returns whether PylonInitialize() was called)
bool PylonAutoEnv::initPylonEnv(){
  std::lock_guard<std::recursive_mutex> l(*env_mutex);
  ICLASSERT(pylon_env_inits >= 0)
  pylon_env_inits++;
  if(pylon_env_inits == 1){
    DEBUG_LOG("Initializing Pylon environment")
    Pylon::PylonInitialize();

    // print available pylon devices
    Pylon::DeviceInfoList_t lstDevices;
    // Get all attached cameras
    Pylon::CTlFactory::GetInstance().EnumerateDevices(lstDevices);
    Pylon::DeviceInfoList_t::const_iterator it;
    for (it = lstDevices.begin(); it != lstDevices.end(); ++it){
      DEBUG_LOG("Found pylon device: " << it -> GetFullName());
    }
    return true;
  } else {
    return false;
  }
}

// terminates the Pylon environment
// (returns whether PylonTerminate() was called).
bool PylonAutoEnv::termPylonEnv(){
  std::lock_guard<std::recursive_mutex> l(*env_mutex);
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

// Gets the mutex lock and stops the acquisiton
AcquisitionInterruptor::AcquisitionInterruptor(Interruptable* i, bool mock)
{
  if(!mock){
    m_Interu = i;
    DEBUG_LOG("stop acquisition")
    m_Interu -> acquisitionStop();
  } else {
    m_Interu = nullptr;
  }
}

// Releases the mutex lock, if acquired before. Starts acquisition.
AcquisitionInterruptor::~AcquisitionInterruptor(){
  if(m_Interu){
    DEBUG_LOG("start acquisition")
    m_Interu -> acquisitionStart();
  }
}

// Frees all grabbing resources
GrabbingInterruptor::GrabbingInterruptor(Interruptable* i, bool mock){
  if(!mock){
    DEBUG_LOG("stop grabbing")
    m_Interu = i;
    m_Interu -> grabbingStop();
  } else {
    m_Interu = nullptr;
  }
}

// Reallocates all needed resources.
GrabbingInterruptor::~GrabbingInterruptor(){
  if(m_Interu){
    DEBUG_LOG("start grab")
    m_Interu -> grabbingStart();
  }
}

void icl::io::pylon::printHelp(){
  std::cout << std::endl;
  std::cout << "The pylon grabber can be called with" << std::endl;
  std::cout << "     -i pylon [CAM]:[BUFFER]" << std::endl << std::endl;
  std::cout << "  [CAM] can be a positive integer value telling" << std::endl;
  std::cout << "        the grabber to choose the corresponding " << std::endl;
  std::cout << "        device from its known devices." << std::endl;
  std::cout << std::endl;

  std::cout << "  [CAM] can alternatively be an IP-address" << std::endl;
  std::cout << "        corresponding directly to a GigE camera." << std::endl;
  std::cout << std::endl;

  std::cout << "  [BUFFER] is an optional, positive integer telling" << std::endl;
  std::cout << "           the grabber to choose the corresponding" << std::endl;
  std::cout << "           Framebuffer." << std::endl;
  std::cout << std::endl;

  std::cout << "A convenient call could look like that:" << std::endl;
  std::cout << "     -i pylon 0" << std::endl;
  std::cout << "  it will make pylon grab from the first" << std::endl;
  std::cout << "  Framebuffer of the first camera." << std::endl;
  std::cout << std::endl;

  std::cout << "  'help'   will print this mesage." << std::endl;
  std::cout << std::endl;
}

Pylon::CDeviceInfo icl::io::pylon::getDeviceFromArgs(std::string args) {
  if(args.find("h")!=std::string::npos){
    printHelp();
    throw ICLException("Help called");
  }

  std::vector<std::string> 	argvec = icl::utils::tok(args, ":");
  ICLASSERT(argvec.size() <= 2)
  if(argvec.at(0).find('.') == std::string::npos){
    unsigned int nr = icl::utils::parse<int>(argvec.at(0));
    Pylon::DeviceInfoList_t devices = getPylonDeviceList();
    if(devices.size() < nr + 1){
      std::cout << "Demanded device Nr. " << nr << " but only "
                << devices.size() << " available.";
          throw ICLException("Could not find demanded device.");
    } else {
      return devices.at(nr);
    }
  } else { // cam by IP
    Pylon::DeviceInfoList_t filter;
    filter.push_back(Pylon::CBaslerGigEDeviceInfo().SetIpAddress(argvec.at(0).c_str()));
    DEBUG_LOG("Trying to start camera from IP: " << argvec.at(0).c_str())
    Pylon::DeviceInfoList_t devices = getPylonDeviceList(&filter);
    if(devices.empty()){
      std::cout << "Camera not found." << argvec.at(0).c_str();
      throw ICLException("Pylon device with specific IP not found.");
    }
    return devices.at(0);
  }
  DEBUG_LOG("Wrong parameters: " << args)
  throw ICLException("PylonDevice not found");
}

int icl::io::pylon::channelFromArgs(std::string args){
  std::vector<std::string> 	argvec = icl::utils::tok(args, ",");
  if(argvec.size() < 2){
      return 0;
  } else {
    return icl::utils::parse<int>(argvec.at(1));
  }
}

Pylon::DeviceInfoList_t
icl::io::pylon::getPylonDeviceList(Pylon::DeviceInfoList_t* filter){
  // Initialization and auto termination of pylon runtime library
  PylonAutoEnv();

  Pylon::DeviceInfoList_t lstDevices;
  // Get all attached cameras
  if(filter==nullptr){
    Pylon::CTlFactory::GetInstance().EnumerateDevices(lstDevices);
  } else {
    Pylon::CTlFactory::GetInstance().EnumerateDevices(lstDevices, *filter);
  }
  return lstDevices;
}
