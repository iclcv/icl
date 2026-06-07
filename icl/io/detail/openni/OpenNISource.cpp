// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#include <icl/utils/Exception.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/ImgBase.h>
#include <icl/core/Img.h>
#include <icl/io/detail/openni/OpenNISource.h>
#include <icl/utils/Macros.h>
#include <mutex>

using namespace xn;
using namespace icl;
using namespace core;
using namespace utils;
using namespace io;
using namespace icl_openni;

//##############################################################################
//############################# OpenNISourceThread ############################
//##############################################################################

// a singleton instance of the source thread
static OpenNISourceThread oniSourceThread;

// Constructor sets used source
OpenNISourceThread::OpenNISourceThread() { /* nothing to do */ }

OpenNISourceThread::~OpenNISourceThread(){
  if(oniSourceThread.running()) oniSourceThread.stop();
}

void OpenNISourceThread::addSource(OpenNISource* source){
  std::scoped_lock lk(oniSourceThread.m_mutex);
  oniSourceThread.m_Source.insert(source);
}

void OpenNISourceThread::removeSource(OpenNISource* source){
  std::scoped_lock lk(oniSourceThread.m_mutex);
  oniSourceThread.m_Source.erase(source);
}

// constantly calls grabNextImage.
void OpenNISourceThread::run(){
  // run as long as source list is not empty
  while(!m_Source.empty()){
    msleep(1);
    // locking thread
    if(!m_mutex.try_lock()) {
      DEBUG_LOG("threadlock returned error. sleep and retry.");
      continue;
    }
    // thread update buffers
    XnStatus rc = OpenNIContext::waitAndUpdate();
    if (rc != XN_STATUS_OK)
    {
      DEBUG_LOG("Read failed: " << xnGetStatusString(rc));
    } else {
      for(std::set<OpenNISource*>::iterator it = m_Source.begin(); it != m_Source.end(); ++it){
        (*it) -> grabNextDisplay();
      }
    }
    // allow thread-stop.
    m_mutex.unlock();
  }
}

//##############################################################################
//############################# OpenNISource ##################################
//##############################################################################

// Constructor of OpenNISourceImpl
OpenNISource::OpenNISource(std::string args)
  : m_Id(args), m_OmitDoubleFrames(true)
{
  std::scoped_lock lock(m_Mutex);
  oniSourceThread.stop();

  DEBUG_LOG("init " << m_Id);

  // create ImageGenerator and Buffer
  m_Generator = OpenNIMapGenerator::createGenerator(m_Id);
  m_Buffer = new ReadWriteBuffer<ImgBase>(m_Generator);
  m_Generator -> getMapGenerator()->StartGenerating();

  addProperty("omit double frames", utils::prop::Flag{}, m_OmitDoubleFrames, "");
  addProperty("format",
              utils::prop::menuFromCsv(m_Generator->getMapOutputModeInfo(m_Generator->getMapGenerator())),
              m_Generator->getCurrentMapOutputMode(m_Generator->getMapGenerator()), "The image format.");
  addProperty("size", utils::prop::Info{}, "adjusted by format", "This is set by the format-property.");
  addChildConfigurable(m_Generator -> getMapGeneratorOptions());
  registerCallback([this](const utils::Configurable::Property &p){ processPropertyChange(p); });

  // register to source thread
  oniSourceThread.addSource(this);
  oniSourceThread.start();
  DEBUG_LOG("init done");
}

OpenNISource::~OpenNISource(){
  DEBUG_LOG("");
  // stop grabbing
  oniSourceThread.stop();
  oniSourceThread.removeSource(this);
  oniSourceThread.start();

  std::scoped_lock lock(m_Mutex);
  // free all
  ICL_DELETE(m_Generator);
  ICL_DELETE(m_Buffer);
}

const ImgBase* OpenNISource::acquireImage(){
  Time t = Time::now();
  // get image from buffer
  ImgBase* img = nullptr;
  while(!img || !(img -> getDim())){ // catch null and empty images
    img = m_Buffer -> getNextReadBuffer(m_OmitDoubleFrames);
    if((Time::now() - t).toSecondsDouble() >= 1.){
      ERROR_LOG("OpenNiSource could not grab an image for more than 1 Second");
      return nullptr;
    }
  }
  return img;
}

// returns the underlying handle of the source. In this case the corresponding MapGenerator.
void* OpenNISource::getHandle(){
  return m_Generator -> getMapGenerator();
}

// grabs an image from ImageGenerator
void OpenNISource::grabNextDisplay(){
  std::scoped_lock l(m_Mutex);
  // check whether a new frame is available
  if(m_Generator->newFrameAvailable()){
    // make ImageGenerator grab an image.
    m_Generator -> acquireImage(m_Buffer -> getNextWriteBuffer());
  }
}

// Returns the string representation of the currently used device.
std::string OpenNISource::getName(){
  return m_Id;
}

// callback for changed configurable properties
void OpenNISource::processPropertyChange(const utils::Configurable::Property &prop){
  DEBUG_LOG(prop.name << " := " << prop.as<std::string>());
  if (prop.name == "format"){
    // `prop` parameter shadows Configurable::prop — spell out `this->`.
    this->prop("map output mode").value = prop.as<std::string>();
  }
  if (prop.name == "omit double frames"){
    m_OmitDoubleFrames = prop.as<bool>();
    return;
  }
  // "size" is ignored
}

REGISTER_CONFIGURABLE(OpenNISource, return new OpenNISource(""));


static SourceBackend* createNISourceDepth(const std::string &param){
  return new OpenNISource("depth" + param);
}

static SourceBackend* createNISourceColor(const std::string &param){
  return new OpenNISource("rgb" + param);
}

static SourceBackend* createNISourceIr(const std::string &param){
  return new OpenNISource("ir" + param);
}

static void getNIDeviceList(std::vector<DeviceDescription>& deviceList,
                            XnPredefinedProductionNodeType type,
                            std::string postfix, std::string desc)
{
  xn::NodeInfoList nodes;
  OpenNIContext::EnumerateProductionTrees(type, nullptr , nodes, nullptr);
  int i = 0;
  for (xn::NodeInfoList::Iterator it = nodes.Begin(); it != nodes.End(); ++it, ++i){
    deviceList.push_back(
          DeviceDescription("oni"+postfix, utils::str(i), desc)
          );
  }
}

static const std::vector<DeviceDescription>& getNIDeviceListDepth(std::string hint, bool rescan){
  static std::vector<DeviceDescription> deviceList;
  if(rescan){
    deviceList.clear();
    getNIDeviceList(deviceList, XN_NODE_TYPE_DEPTH, "d", "An OpenNI Depth data generator.");
  }
  return deviceList;
}

static const std::vector<DeviceDescription>& getNIDeviceListColor(std::string hint, bool rescan){
  static std::vector<DeviceDescription> deviceList;
  if(rescan){
    deviceList.clear();
    getNIDeviceList(deviceList, XN_NODE_TYPE_IMAGE, "c", "An OpenNI rgb-image data generator.");
  }
  return deviceList;
}

static const std::vector<DeviceDescription>& getNIDeviceListIr(std::string hint, bool rescan){
  static std::vector<DeviceDescription> deviceList;
  if(rescan){
    deviceList.clear();
    getNIDeviceList(deviceList, XN_NODE_TYPE_IR, "i", "An OpenNI IR data generator.");
  }
  return deviceList;
}

REGISTER_SOURCE_BACKEND(onid,createNISourceDepth, getNIDeviceListDepth, "index 0 opens the first depth source~OpenNI based image source.");
REGISTER_SOURCE_BACKEND(onic,createNISourceColor, getNIDeviceListColor, "index 0 opens the first color source~OpenNI based image source.");
REGISTER_SOURCE_BACKEND(onii,createNISourceIr, getNIDeviceListIr, "index 0 opens the first ir source~OpenNI based image source.");
