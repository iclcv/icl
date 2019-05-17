/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PylonUtils.cpp                         **
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

#include <ICLIO/PylonUtils.h>
#include <ICLUtils/StringUtils.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::io::pylon;

//static int inits = 0;

// Static mutex and counter for clean initialising
// and deleting of Pylon environment
static unsigned int pylon_env_inits = 0;
static utils::Mutex* env_mutex = new icl::utils::Mutex();

// Constructor sets all pointers to NULL
ConvBuffers::ConvBuffers(){
  m_Image = NULL;
  m_ImageBuff = NULL;
  m_ImageBuff16 = NULL;
  m_ImageRGBA = NULL;
  m_Channels = NULL;
  m_Channels16 = NULL;
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
  Mutex::Locker l(m_Mutex);
  m_Buffers[0] = new ConvBuffers();
  m_Buffers[1] = new ConvBuffers();
  m_Buffers[2] = new ConvBuffers();
}

// Destructor frees memory
ConcGrabberBuffer::~ConcGrabberBuffer() {
  Mutex::Locker l(m_Mutex);
  ICL_DELETE(m_Buffers[0]);
  ICL_DELETE(m_Buffers[1]);
  ICL_DELETE(m_Buffers[2]);
}

// returns a pointer to the most recent actualized ConvBuffers.
ConvBuffers* ConcGrabberBuffer::getNextReadBuffer(){
  Mutex::Locker l(m_Mutex);
  if(m_Avail){
    // new buffer is available.
    std::swap(m_Next, m_Read);
    m_Avail = false;
  }
  return m_Buffers[m_Read];
}

// returns a pointer to the next write ConvBuffers.
ConvBuffers* ConcGrabberBuffer::getNextWriteBuffer(){
  Mutex::Locker l(m_Mutex);
  // swap write buffer and next buffer.
  std::swap(m_Next, m_Write);
  // new buffer is available for reading.
  m_Avail = true;
  // return new write buffer.
  return m_Buffers[m_Write];
}

// mark ConvBuffers to be reset on next write-access.
void ConcGrabberBuffer::setReset(){
  Mutex::Locker l(m_Mutex);
  m_Buffers[0] -> m_Reset = true;
  m_Buffers[1] -> m_Reset = true;
  m_Buffers[2] -> m_Reset = true;
}

// tells whether a new image is available
bool ConcGrabberBuffer::newAvailable(){
  Mutex::Locker l(m_Mutex);
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
  icl::utils::Mutex::Locker l(env_mutex);
  ICLASSERT(pylon_env_inits >= 0)
  pylon_env_inits++;
  if(pylon_env_inits == 1){
    DEBUG_LOG2("Initializing Pylon environment")
    Pylon::PylonInitialize();

    // print available pylon devices
    Pylon::DeviceInfoList_t lstDevices;
    // Get all attached cameras
    Pylon::CTlFactory::GetInstance().EnumerateDevices(lstDevices);
    Pylon::DeviceInfoList_t::const_iterator it;
    for (it = lstDevices.begin(); it != lstDevices.end(); ++it){
      DEBUG_LOG2("Found pylon device: " << it -> GetFullName());
    }
    return true;
  } else {
    return false;
  }
}

// terminates the Pylon environment
// (returns whether PylonTerminate() was called).
bool PylonAutoEnv::termPylonEnv(){
  utils::Mutex::Locker l(env_mutex);
  ICLASSERT(pylon_env_inits > 0)
  pylon_env_inits--;
  if(pylon_env_inits == 0){
    DEBUG_LOG2("Terminating Pylon environment")
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
    DEBUG_LOG2("stop acquisition")
    m_Interu -> acquisitionStop();
  } else {
    m_Interu = NULL;
  }
}

// Releases the mutex lock, if acquired before. Starts acquisition.
AcquisitionInterruptor::~AcquisitionInterruptor(){
  if(m_Interu){
    DEBUG_LOG2("start acquisition")
    m_Interu -> acquisitionStart();
  }
}

// Frees all grabbing resources
GrabbingInterruptor::GrabbingInterruptor(Interruptable* i, bool mock){
  if(!mock){
    DEBUG_LOG2("stop grabbing")
    m_Interu = i;
    m_Interu -> grabbingStop();
  } else {
    m_Interu = NULL;
  }
}

// Reallocates all needed resources.
GrabbingInterruptor::~GrabbingInterruptor(){
  if(m_Interu){
    DEBUG_LOG2("start grab")
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
    DEBUG_LOG2("Trying to start camera from IP: " << argvec.at(0).c_str())
    Pylon::DeviceInfoList_t devices = getPylonDeviceList(&filter);
    if(devices.empty()){
      std::cout << "Camera not found." << argvec.at(0).c_str();
      throw ICLException("Pylon device with specific IP not found.");
    }
    return devices.at(0);
  }
  DEBUG_LOG2("Wrong parameters: " << args)
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
  if(filter==NULL){
    Pylon::CTlFactory::GetInstance().EnumerateDevices(lstDevices);
  } else {
    Pylon::CTlFactory::GetInstance().EnumerateDevices(lstDevices, *filter);
  }
  return lstDevices;
}
