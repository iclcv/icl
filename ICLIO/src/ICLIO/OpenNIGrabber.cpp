/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OpenNIGrabber.cpp                      **
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
#include <ICLIO/OpenNIGrabber.h>
#include <ICLUtils/Macros.h>

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
      DEBUG_LOG2("threadlock returned error. sleep and retry.");
      continue;
    }
    // thread update buffers
    XnStatus rc = OpenNIContext::waitAndUpdate();
    if (rc != XN_STATUS_OK)
    {
      DEBUG_LOG2("Read failed: " << xnGetStatusString(rc));
    } else {
      for(std::set<OpenNIGrabber*>::iterator it = m_Grabber.begin(); it != m_Grabber.end(); ++it){
        (*it) -> grabNextImage();
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
  Mutex::Locker lock(m_Mutex);
  oniGrabberThread.stop();

  DEBUG_LOG2("init " << m_Id);

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
  Configurable::registerCallback(utils::function(this,&OpenNIGrabber::processPropertyChange));

  // register to grabber thread
  oniGrabberThread.addGrabber(this);
  oniGrabberThread.start();
  DEBUG_LOG2("init done");
}

OpenNIGrabber::~OpenNIGrabber(){
  DEBUG_LOG("");
  // stop grabbing
  oniGrabberThread.stop();
  oniGrabberThread.removeGrabber(this);
  oniGrabberThread.start();

  Mutex::Locker lock(m_Mutex);
  // free all
  ICL_DELETE(m_Generator);
  ICL_DELETE(m_Buffer);
}

const ImgBase* OpenNIGrabber::acquireImage(){
  Time t = Time::now();
  // get image from buffer
  ImgBase* img = NULL;
  while(!img || !(img -> getDim())){ // catch null and empty images
    img = m_Buffer -> getNextReadBuffer(m_OmitDoubleFrames);
    if((Time::now() - t).toSecondsDouble() >= 1.){
      ERROR_LOG("OpenNiGrabber could not grab an image for more than 1 Second");
      return NULL;
    }
  }
  return img;
}

// returns the underlying handle of the grabber. In this case the corresponding MapGenerator.
void* OpenNIGrabber::getHandle(){
  return m_Generator -> getMapGenerator();
}

// grabs an image from ImageGenerator
void OpenNIGrabber::grabNextImage(){
  Mutex::Locker l(m_Mutex);
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
  DEBUG_LOG2(prop.name << " := " << prop.value);
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
  OpenNIContext::EnumerateProductionTrees(type, NULL , nodes, NULL);
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

REGISTER_GRABBER(onid,utils::function(createNIGrabberDepth), utils::function(getNIDeviceListDepth), "onid:index 0 opens the first depth source:OpenNI based image source.");
REGISTER_GRABBER(onic,utils::function(createNIGrabberColor), utils::function(getNIDeviceListColor), "onic:index 0 opens the first color source:OpenNI based image source.");
REGISTER_GRABBER(onii,utils::function(createNIGrabberIr), utils::function(getNIDeviceListIr), "onii:index 0 opens the first ir source:OpenNI based image source.");
