/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/PylonUtils.cpp                               **
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

#include <ICLIO/PylonUtils.h>
#include <ICLUtils/StringUtils.h>

#include <pylon/gige/BaslerGigEDeviceInfo.h>

using namespace icl;
using namespace icl::pylon;

// Static mutex and counter for clean initialising
// and deleting of Pylon environment
static unsigned int pylon_env_inits = 0;
static icl::Mutex* env_mutex = new icl::Mutex();

// Constructor creates and initializes resources.
ConcGrabberBuffer::ConcGrabberBuffer(int bufferSize) :
m_Mutex(), m_Write(0), m_Next(1), m_Read(2) {
  Mutex::Locker l(m_Mutex);
  m_Buffers[0] = new TsBuffer<int16_t>(bufferSize);
  m_Buffers[1] = new TsBuffer<int16_t>(bufferSize);
  m_Buffers[2] = new TsBuffer<int16_t>(bufferSize);
}

// Destructor frees memory
ConcGrabberBuffer::~ConcGrabberBuffer() {
  Mutex::Locker l(m_Mutex);
  ICL_DELETE(m_Buffers[0]);
  ICL_DELETE(m_Buffers[1]);
  ICL_DELETE(m_Buffers[2]);
}

// returns a pointer to the most recent actualized buffer.
TsBuffer<int16_t>* ConcGrabberBuffer::getNextImage(){
  Mutex::Locker l(m_Mutex);
  if(m_Avail){
    // new buffer is available.
    std::swap(m_Next, m_Read);
    m_Avail = false;

  }
  return m_Buffers[m_Read];
}

// returns a pointer to the next write buffer
TsBuffer<int16_t>* ConcGrabberBuffer::getNextBuffer(){
  Mutex::Locker l(m_Mutex);
  // swap write buffer and next buffer.
  std::swap(m_Next, m_Write);
  // new buffer is available for reading.
  m_Avail = true;
  // return new write buffer.
  return m_Buffers[m_Write];
}

// frees allocated memory and reinitializes Buffers to bufferSize.
void ConcGrabberBuffer::reset(int bufferSize){
  Mutex::Locker l(m_Mutex);
  delete m_Buffers[0];
  m_Buffers[0] = new TsBuffer<int16_t>(bufferSize);
  delete m_Buffers[1];
  m_Buffers[1] = new TsBuffer<int16_t>(bufferSize);
  delete m_Buffers[2];
  m_Buffers[2] = new TsBuffer<int16_t>(bufferSize);
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
  icl::Mutex::Locker l(env_mutex);
  ICLASSERT(pylon_env_inits >= 0)
  pylon_env_inits++;
  if(pylon_env_inits == 1){
    DEBUG_LOG("Initializing Pylon environment")
    Pylon::PylonInitialize();

    // print available pylon devices
    Pylon::DeviceInfoList_t lstDevices;
    // Get all attached cameras
    Pylon::CTlFactory::GetInstance().EnumerateDevices(lstDevices);
    std::cout << "-------------Devices---------------" << std::endl;
    Pylon::DeviceInfoList_t::const_iterator it;
    for (it = lstDevices.begin(); it != lstDevices.end(); ++it){
      std::cout << it -> GetFullName() << std::endl;
    }
    std::cout << "-----------------------------------" << std::endl;
    return true;
  } else {
    return false;
  }
}

// terminates the Pylon environment
// (returns whether PylonTerminate() was called).
bool PylonAutoEnv::termPylonEnv(){
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

// Gets the mutex lock and stops the acquisiton
AcquisitionInterruptor::AcquisitionInterruptor(Interruptable* i, bool mock)
{
  if(!mock){
    m_Interu = i;
    DEBUG_LOG("stop acquisition")
    m_Interu -> acquisitionStop();
  } else {
    m_Interu = NULL;
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
    m_Interu = NULL;
  }
}

// Reallocates all needed resources.
GrabbingInterruptor::~GrabbingInterruptor(){
  if(m_Interu){
    DEBUG_LOG("start grab")
    m_Interu -> grabbingStart();
  }
}

void icl::pylon::printHelp(){
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

Pylon::CDeviceInfo icl::pylon::getDeviceFromArgs(std::string args) throw(ICLException) {
  if(args.find("h")!=std::string::npos){
    printHelp();
    throw ICLException("Help called");
  }

  std::vector<std::string> 	argvec = icl::tok(args, ":");
  ICLASSERT(argvec.size() <= 2)
  if(argvec.at(0).find('.') == std::string::npos){
    unsigned int nr = icl::parse<int>(argvec.at(0));
    Pylon::DeviceInfoList_t devices = getPylonDeviceList();
    if(devices.size() < nr + 1){
      DEBUG_LOG("Demanded device Nr. " << nr << " but only "
                << devices.size() << " available.")
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
      DEBUG_LOG("Camera not found." << argvec.at(0).c_str())
      throw ICLException("Pylon device with specific IP not found.");
    }
    return devices.at(0);
  }
  DEBUG_LOG("Wrong parameters: " << args)
  throw ICLException("PylonDevice not found");
}

int icl::pylon::channelFromArgs(std::string args){
  std::vector<std::string> 	argvec = icl::tok(args, ",");
  if(argvec.size() < 2){
      return 0;
  } else {
    return icl::parse<int>(argvec.at(1));
  }
}

Pylon::DeviceInfoList_t
icl::pylon::getPylonDeviceList(Pylon::DeviceInfoList_t* filter){
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
