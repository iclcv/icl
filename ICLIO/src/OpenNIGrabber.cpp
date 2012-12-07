/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/OpenNIGrabber.cpp                            **
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
    DEBUG_LOG(st.str());
    throw new ICLException(st.str());
  }
}

//##############################################################################
//############################# OpenNIGrabberThread ############################
//##############################################################################

// Constructor sets used grabber
OpenNIGrabberThread::OpenNIGrabberThread(OpenNIGrabberImpl* grabber)
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
//############################# OpenNIGrabberImpl ##############################
//##############################################################################

// Constructor of OpenNIGrabberImpl
OpenNIGrabberImpl::OpenNIGrabberImpl(std::string args)
  : m_Id(args), m_OmitDoubleFrames(true)
{
  Mutex::Locker lock(m_Mutex);

  XnStatus rc;
  DEBUG_LOG2("init " << m_Id);
  // init OpenNI Context
  rc = m_Context.Init();
  assertStatus(rc);

  // create ImageGenerator and Buffer
  m_Generator = OpenNIMapGenerator::createGenerator(&m_Context, m_Id);
  m_Buffer = new ReadWriteBuffer<ImgBase>(m_Generator);
  m_Generator -> getMapGenerator()->StartGenerating();

  addProperty("omit double frames", "flag", "", m_OmitDoubleFrames, 0, "");
  addProperty("format", "menu", m_Generator -> getMapOutputModeInfo(m_Generator->getMapGenerator()),
              m_Generator -> getCurrentMapOutputMode(m_Generator->getMapGenerator()),
              0, "The image format.");
  addProperty("size", "info", "", "adjusted by format", 0, "This is set by the format-property.");
  addChildConfigurable(m_Generator -> getMapGeneratorOptions());
  Configurable::registerCallback(utils::function(this,&OpenNIGrabberImpl::processPropertyChange));

  // create grabber-thread
  m_GrabberThread = new OpenNIGrabberThread(this);
  m_GrabberThread -> start();
  DEBUG_LOG2("init done");
}

OpenNIGrabberImpl::~OpenNIGrabberImpl(){
  DEBUG_LOG2("");
  // stop grabbing
  m_GrabberThread -> stop();

  Mutex::Locker lock(m_Mutex);
  // free all
  ICL_DELETE(m_Generator);
  ICL_DELETE(m_Buffer);
  ICL_DELETE(m_GrabberThread);
  m_Context.Release();
}

const ImgBase* OpenNIGrabberImpl::acquireImage(){
  // get image from buffer
  ImgBase* img = m_Buffer -> getNextReadBuffer(m_OmitDoubleFrames);
  if(img && !(img -> getDim())){ // catch empty images
    return NULL;
  }
  return img;
}

// returns the underlying handle of the grabber. In this case the corresponding MapGenerator.
void* OpenNIGrabberImpl::getHandle(){
  return m_Generator -> getMapGenerator();
}

// grabs an image from ImageGenerator
void OpenNIGrabberImpl::grabNextImage(){
  Mutex::Locker l(m_Mutex);
  // make ImageGenerator grab an image.
  m_Generator -> acquireImage(m_Buffer -> getNextWriteBuffer());
}

// Returns the string representation of the currently used device.
std::string OpenNIGrabberImpl::getName(){
  return m_Id;
}

// callback for changed configurable properties
void OpenNIGrabberImpl::processPropertyChange(const utils::Configurable::Property &prop){
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
