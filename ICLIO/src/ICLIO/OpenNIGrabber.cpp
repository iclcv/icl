// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#include <ICLUtils/Exception.h>
#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLIO/OpenNIGrabber.h>
#include <ICLUtils/Macros.h>
#include <mutex>

using namespace xn;
using namespace icl;
using namespace core;
using namespace utils;
using namespace io;
using namespace icl_openni;

//##############################################################################
//############################# OpenNIGrabberThread ############################
//##############################################################################

// a singleton instance of the grabber thread
static OpenNIGrabberThread oniGrabberThread;

// Constructor sets used grabber
OpenNIGrabberThread::OpenNIGrabberThread() { /* nothing to do */ }

OpenNIGrabberThread::~OpenNIGrabberThread(){
  if(oniGrabberThread.running()) oniGrabberThread.stop();
}

void OpenNIGrabberThread::addGrabber(OpenNIGrabber* grabber){
  oniGrabberThread.lock();
  oniGrabberThread.m_Grabber.insert(grabber);
  oniGrabberThread.unlock();
}

void OpenNIGrabberThread::removeGrabber(OpenNIGrabber* grabber){
  oniGrabberThread.lock();
  oniGrabberThread.m_Grabber.erase(grabber);
  oniGrabberThread.unlock();
}

// constantly calls grabNextImage.
void OpenNIGrabberThread::run(){
  // run as long as grabber list is not empty
  while(!m_Grabber.empty()){
    msleep(1);
    // locking thread
    if(trylock()) {
      DEBUG_LOG("threadlock returned error. sleep and retry.");
      continue;
    }
    // thread update buffers
    XnStatus rc = OpenNIContext::waitAndUpdate();
    if (rc != XN_STATUS_OK)
    {
      DEBUG_LOG("Read failed: " << xnGetStatusString(rc));
    } else {
      for(std::set<OpenNIGrabber*>::iterator it = m_Grabber.begin(); it != m_Grabber.end(); ++it){
        (*it) -> grabNextDisplay();
      }
    }
    // allow thread-stop.
    unlock();
  }
}

//##############################################################################
//############################# OpenNIGrabber ##################################
//##############################################################################

// Constructor of OpenNIGrabberImpl
OpenNIGrabber::OpenNIGrabber(std::string args)
  : m_Id(args), m_OmitDoubleFrames(true)
{
  std::scoped_lock<std::recursive_mutex> lock(m_Mutex);
  oniGrabberThread.stop();

  DEBUG_LOG("init " << m_Id);

  // create ImageGenerator and Buffer
  m_Generator = OpenNIMapGenerator::createGenerator(m_Id);
  m_Buffer = new ReadWriteBuffer<ImgBase>(m_Generator);
  m_Generator -> getMapGenerator()->StartGenerating();

  addProperty("omit double frames", "flag", "", m_OmitDoubleFrames, 0, "");
  addProperty("format", "menu", m_Generator -> getMapOutputModeInfo(m_Generator->getMapGenerator()),
              m_Generator -> getCurrentMapOutputMode(m_Generator->getMapGenerator()),
              0, "The image format.");
  addProperty("size", "info", "", "adjusted by format", 0, "This is set by the format-property.");
  addChildConfigurable(m_Generator -> getMapGeneratorOptions());
  Configurable::registerCallback([this](const utils::Configurable::Property &p){ processPropertyChange(p); });

  // register to grabber thread
  oniGrabberThread.addGrabber(this);
  oniGrabberThread.start();
  DEBUG_LOG("init done");
}

OpenNIGrabber::~OpenNIGrabber(){
  DEBUG_LOG("");
  // stop grabbing
  oniGrabberThread.stop();
  oniGrabberThread.removeGrabber(this);
  oniGrabberThread.start();

  std::scoped_lock<std::recursive_mutex> lock(m_Mutex);
  // free all
  ICL_DELETE(m_Generator);
  ICL_DELETE(m_Buffer);
}

const ImgBase* OpenNIGrabber::acquireDisplay(){
  Time t = Time::now();
  // get image from buffer
  ImgBase* img = nullptr;
  while(!img || !(img -> getDim())){ // catch null and empty images
    img = m_Buffer -> getNextReadBuffer(m_OmitDoubleFrames);
    if((Time::now() - t).toSecondsDouble() >= 1.){
      ERROR_LOG("OpenNiGrabber could not grab an image for more than 1 Second");
      return nullptr;
    }
  }
  return img;
}

// returns the underlying handle of the grabber. In this case the corresponding MapGenerator.
void* OpenNIGrabber::getHandle(){
  return m_Generator -> getMapGenerator();
}

// grabs an image from ImageGenerator
void OpenNIGrabber::grabNextDisplay(){
  std::scoped_lock<std::recursive_mutex> l(m_Mutex);
  // check whether a new frame is available
  if(m_Generator->newFrameAvailable()){
    // make ImageGenerator grab an image.
    m_Generator -> acquireImage(m_Buffer -> getNextWriteBuffer());
  }
}

// Returns the string representation of the currently used device.
std::string OpenNIGrabber::getName(){
  return m_Id;
}

// callback for changed configurable properties
void OpenNIGrabber::processPropertyChange(const utils::Configurable::Property &prop){
  DEBUG_LOG(prop.name << " := " << prop.value);
  if (prop.name == "format"){
    setPropertyValue("map output mode", prop.value);
  }
  if (prop.name == "omit double frames"){
    m_OmitDoubleFrames = parse<bool>(prop.value);
    return;
  }
  // "size" is ignored
}

REGISTER_CONFIGURABLE(OpenNIGrabber, return new OpenNIGrabber(""));


static Grabber* createNIGrabberDepth(const std::string &param){
  return new OpenNIGrabber("depth" + param);
}

static Grabber* createNIGrabberColor(const std::string &param){
  return new OpenNIGrabber("rgb" + param);
}

static Grabber* createNIGrabberIr(const std::string &param){
  return new OpenNIGrabber("ir" + param);
}

static void getNIDeviceList(std::vector<GrabberDeviceDescription>& deviceList,
                            XnPredefinedProductionNodeType type,
                            std::string postfix, std::string desc)
{
  xn::NodeInfoList nodes;
  OpenNIContext::EnumerateProductionTrees(type, nullptr , nodes, nullptr);
  int i = 0;
  for (xn::NodeInfoList::Iterator it = nodes.Begin(); it != nodes.End(); ++it, ++i){
    deviceList.push_back(
          GrabberDeviceDescription("oni"+postfix, utils::str(i), desc)
          );
  }
}

static const std::vector<GrabberDeviceDescription>& getNIDeviceListDepth(std::string hint, bool rescan){
  static std::vector<GrabberDeviceDescription> deviceList;
  if(rescan){
    deviceList.clear();
    getNIDeviceList(deviceList, XN_NODE_TYPE_DEPTH, "d", "An OpenNI Depth data generator.");
  }
  return deviceList;
}

static const std::vector<GrabberDeviceDescription>& getNIDeviceListColor(std::string hint, bool rescan){
  static std::vector<GrabberDeviceDescription> deviceList;
  if(rescan){
    deviceList.clear();
    getNIDeviceList(deviceList, XN_NODE_TYPE_IMAGE, "c", "An OpenNI rgb-image data generator.");
  }
  return deviceList;
}

static const std::vector<GrabberDeviceDescription>& getNIDeviceListIr(std::string hint, bool rescan){
  static std::vector<GrabberDeviceDescription> deviceList;
  if(rescan){
    deviceList.clear();
    getNIDeviceList(deviceList, XN_NODE_TYPE_IR, "i", "An OpenNI IR data generator.");
  }
  return deviceList;
}

REGISTER_GRABBER(onid,createNIGrabberDepth, getNIDeviceListDepth, "onid:index 0 opens the first depth source:OpenNI based image source.");
REGISTER_GRABBER(onic,createNIGrabberColor, getNIDeviceListColor, "onic:index 0 opens the first color source:OpenNI based image source.");
REGISTER_GRABBER(onii,createNIGrabberIr, getNIDeviceListIr, "onii:index 0 opens the first ir source:OpenNI based image source.");
