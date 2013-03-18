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
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

// checks whether status is OK. else throws an Exception.
void assertStatus(XnStatus &status){
  if (status != XN_STATUS_OK){
    std::ostringstream st;
    st << "XnStatus != XN_STATUS_OK. Got '" << xnGetStatusString(status) << "'";
    ERROR_LOG(st.str());
    throw new ICLException(st.str());
  }
}

/// the OpenNI context
class OniContext{
  private:
    Mutex lock;
    xn::Context context;
    bool initialized;

    OniContext() : initialized(false){}

    static OniContext& inst(){
      static OniContext inst;
      return inst;
    }

  public:
    static xn::Context* get(){
      OniContext& inst = OniContext::inst();
      Mutex::Locker l(inst.lock);
      if(!inst.initialized){
        XnStatus xn = inst.context.Init();
        assertStatus(xn);
        inst.initialized = true;
      }
      return &inst.context;
    }

    static void release(){
      OniContext& inst = OniContext::inst();
      Mutex::Locker l(inst.lock);
      if(inst.initialized){
        //inst.context.Release();
        //inst.initialized = false;
        // not releasing. releasing leads to more problems than not releasing
      }
    }

    ~OniContext(){
      Mutex::Locker l(lock);
      if(initialized){
        context.Release();
        initialized = false;
      }
    }
};

//##############################################################################
//############################# OpenNIGrabberThread ############################
//##############################################################################

// Constructor sets used grabber
OpenNIGrabberThread::OpenNIGrabberThread(OpenNIGrabber* grabber)
  : m_Grabber(grabber)
{ /* nothing to do */ }


// constantly calls grabNextImage.
void OpenNIGrabberThread::run(){
  // non-stop-loop
  while(1){
    msleep(1);
    // locking thread
    if(trylock()) {
      DEBUG_LOG2("threadlock returned error. sleep and retry.");
      continue;
    }
    // thread locked grab image.
    m_Grabber -> grabNextImage();
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

  DEBUG_LOG2("init " << m_Id);

  // create ImageGenerator and Buffer
  m_Generator = OpenNIMapGenerator::createGenerator(OniContext::get(), m_Id);
  m_Buffer = new ReadWriteBuffer<ImgBase>(m_Generator);
  m_Generator -> getMapGenerator()->StartGenerating();

  addProperty("omit double frames", "flag", "", m_OmitDoubleFrames, 0, "");
  addProperty("format", "menu", m_Generator -> getMapOutputModeInfo(m_Generator->getMapGenerator()),
              m_Generator -> getCurrentMapOutputMode(m_Generator->getMapGenerator()),
              0, "The image format.");
  addProperty("size", "info", "", "adjusted by format", 0, "This is set by the format-property.");
  addChildConfigurable(m_Generator -> getMapGeneratorOptions());
  Configurable::registerCallback(utils::function(this,&OpenNIGrabber::processPropertyChange));

  // create grabber-thread
  m_GrabberThread = new OpenNIGrabberThread(this);
  m_GrabberThread -> start();
  DEBUG_LOG2("init done");
}

OpenNIGrabber::~OpenNIGrabber(){
  DEBUG_LOG2("");
  // stop grabbing
  m_GrabberThread -> stop();

  Mutex::Locker lock(m_Mutex);
  // free all
  ICL_DELETE(m_Generator);
  ICL_DELETE(m_Buffer);
  ICL_DELETE(m_GrabberThread);
  OniContext::release();
}

const ImgBase* OpenNIGrabber::acquireImage(){
  // get image from buffer
  ImgBase* img = m_Buffer -> getNextReadBuffer(m_OmitDoubleFrames);
  if(img && !(img -> getDim())){ // catch empty images
    return NULL;
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
  // make ImageGenerator grab an image.
  m_Generator -> acquireImage(m_Buffer -> getNextWriteBuffer());
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
  xn::Context* context = OniContext::get();
  xn::NodeInfoList nodes;
  context -> EnumerateProductionTrees(type, NULL , nodes, NULL);
  int i = 0;
  for (xn::NodeInfoList::Iterator it = nodes.Begin(); it != nodes.End(); ++it, ++i){
    deviceList.push_back(
          GrabberDeviceDescription("oni"+postfix, utils::str(i), desc)
          );
  }
  OniContext::release();
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
