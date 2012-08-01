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
using namespace icl_openni;

ScriptNode g_scriptNode;

void printNodeInfo(Context cont){
  NodeInfoList l;
  XnStatus rc = cont.EnumerateExistingNodes(l);
  DEBUG_LOG("XnStatus: " << xnGetStatusString(rc))
      XnProductionNodeDescription pi;
  DEBUG_LOG("NodeInfoList")
      for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it){
    pi = (*it).GetDescription();
    DEBUG_LOG(pi.strName);
    DEBUG_LOG(pi.strVendor);
    DEBUG_LOG(pi.Type);
    DEBUG_LOG(pi.Version.nMajor << " : "
              << pi.Version.nMinor << " : "
              << pi.Version.nMaintenance << " : ");
  }
  if(l.Begin() == l.End()){
    DEBUG_LOG("The nodelist is empty \n");
  } else {
    std::cout << std::endl;
  }
}

void printProductionTrees(Context cont){
  EnumerationErrors errors;
  NodeInfoList l;
  XnStatus rc = cont.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL , l, &errors);
  DEBUG_LOG("XnStatus: " << xnGetStatusString(rc));
  DEBUG_LOG("ProductionTree - NodeInfoList");
  int i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it){
    NodeInfo deviceNodeInfo = *it;
    Device deviceNode;

    deviceNodeInfo.GetInstance(deviceNode);
    XnBool bExists = deviceNode.IsValid();

    if (!bExists) {
      DEBUG_LOG1("Node not valid. CreateProductionTree");
      cont.CreateProductionTree(deviceNodeInfo, deviceNode);
    }

    if (deviceNode.IsValid() && deviceNode.IsCapabilitySupported(XN_CAPABILITY_DEVICE_IDENTIFICATION)) {
      const XnUInt32 bSize = 200;
      XnChar deviceName[bSize];
      XnChar deviceSN[bSize];
      XnChar treeStrRep[bSize];

      deviceNode.GetIdentificationCap().GetDeviceName(deviceName, bSize);
      deviceNode.GetIdentificationCap().GetSerialNumber(deviceSN, bSize);
      deviceNodeInfo.GetTreeStringRepresentation(treeStrRep, bSize);
      DEBUG_LOG(" [" << i << "]" << deviceName << "(" << deviceSN << ")");
      /*DEBUG_LOG(deviceNodeInfo.GetAdditionalData());
      DEBUG_LOG(deviceNodeInfo.GetCreationInfo());
      DEBUG_LOG(deviceNodeInfo.GetDescription().strName);
      DEBUG_LOG(deviceNodeInfo.GetDescription().strVendor);
      DEBUG_LOG(deviceNodeInfo.GetDescription().Type);
      DEBUG_LOG(deviceNodeInfo.GetDescription().Version.nMajor);
      DEBUG_LOG(deviceNodeInfo.GetInstanceName());*/
      DEBUG_LOG("CAM: " << treeStrRep);
    } else {
      DEBUG_LOG("No device identification cap:" << i);
    }
    i++;
  }
}

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
      DEBUG_LOG("threadlock returned error. sleep and retry.");
      continue;
    }
    //thread locked grab image.
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
  : m_Id(parse<int>(args)), m_OmitDoubleFrames(true)
{
  icl::Mutex::Locker lock(m_Mutex);

  XnStatus rc;
  DEBUG_LOG("init " << m_Id);
  rc = m_Context.Init();
  assertStatus(rc);

  printProductionTrees(m_Context);

  m_Generator = OpenNIMapGenerator::createGenerator(
                  &m_Context, OpenNIMapGenerator::DEPTH, m_Id);
  m_Buffer = new ReadWriteBuffer<ImgBase>(m_Generator);
  m_Generator -> getMapGenerator()->StartGenerating();
  m_GrabberThread = new OpenNIGrabberThread(this);
  m_GrabberThread -> start();
  DEBUG_LOG("init done");
}

OpenNIGrabberImpl::~OpenNIGrabberImpl(){
  DEBUG_LOG("");
  m_GrabberThread -> stop();

  icl::Mutex::Locker lock(m_Mutex);
  ICL_DELETE(m_Generator);
  ICL_DELETE(m_Buffer);
  ICL_DELETE(m_GrabberThread);
  m_Context.Release();
}

const icl::ImgBase* OpenNIGrabberImpl::acquireImage(){
  ImgBase* img = m_Buffer -> getNextReadBuffer(m_OmitDoubleFrames);
  if(!img){
    DEBUG_LOG("img is null");
  } else if(!(img -> getDim())){
    DEBUG_LOG("img is empty");
    return NULL;
  }
  return img;
}

// grabs an image from Imagegenerator
void OpenNIGrabberImpl::grabNextImage(){
  Mutex::Locker l(m_Mutex);
  //DEBUG_LOG("");
  m_Generator -> acquireImage(m_Buffer -> getNextWriteBuffer());
  //DEBUG_LOG("");
}

// switches the current generator to desired
void
OpenNIGrabberImpl::setGeneratorTo(OpenNIMapGenerator::Generators desired){
  DEBUG_LOG("set Generator to " << desired);
  if(desired != m_Generator -> getGeneratorType()){
    DEBUG_LOG("switching generator");
    Mutex::Locker l(m_Mutex);
    DEBUG_LOG("got lock");
    ICL_DELETE(m_Generator);
    m_Generator = OpenNIMapGenerator::createGenerator(
                    &m_Context, desired, m_Id
                    );
    m_Buffer -> switchHandler(m_Generator);
    DEBUG_LOG("done");
  }
}

// Returns the string representation of the currently used device.
std::string OpenNIGrabberImpl::getName(){
  return toStr(m_Id);
}

std::string stringFromGenerator(const OpenNIMapGenerator::Generators &gen){
  switch (gen){
    case OpenNIMapGenerator::RGB:
      return "rgb";
    case OpenNIMapGenerator::DEPTH:
      return "depth";
    case OpenNIMapGenerator::NOT_SPECIFIED:
      return "not_specified";
    default:
      return "error";
  }
}

OpenNIMapGenerator::Generators generatorFromString(const std::string &value){
  if(!value.compare("rgb")){
    return OpenNIMapGenerator::RGB;
  }
  if(!value.compare("depth")){
    return OpenNIMapGenerator::DEPTH;
  }
  return OpenNIMapGenerator::NOT_SPECIFIED;
}

// interface for the setter function for video device properties
void OpenNIGrabberImpl::setProperty(const std::string &property, const std::string &value){
  DEBUG_LOG(property << " := " << value);
      if (property.compare("size") == 0){
    return;
  }
  if (property.compare("format") == 0){
    m_Generator -> setProperty("map output mode", value);
  }
  if (property.compare("omit double frames") == 0){
    m_OmitDoubleFrames = (value == "On");
    return;
  }
  if (property.compare("generator") == 0){
    setGeneratorTo(generatorFromString(value));
    return;
  }
  if (m_Generator -> supportsProperty(property)){
    m_Generator -> setProperty(property, value);
    return;
  }
}

// returns a list of properties, that can be set using setProperty
std::vector<std::string> OpenNIGrabberImpl::getPropertyList(){
  DEBUG_LOG("");
      std::vector<std::string> ps;
  ps.push_back("size");
  ps.push_back("format");
  ps.push_back("omit double frames");
  ps.push_back("generator");
  m_Generator -> addPropertiesToList(ps);
  return ps;
}

// checks if property is returned, implemented, available and of processable GenApi::EInterfaceType
bool OpenNIGrabberImpl::supportsProperty(const std::string &property){
  DEBUG_LOG("");
  if (property.compare("size") == 0) return true;
  if (property.compare("format") == 0) return true;
  if (property.compare("omit double frames") == 0) return true;
  if (property.compare("generator") == 0) return true;
  if (m_Generator -> supportsProperty(property)) return true;
  return false;
}

// get type of property
std::string OpenNIGrabberImpl::getType(const std::string &name){
  DEBUG_LOG("");
      if(name.compare("size") == 0){
    return "info";
  }
  if(name.compare("format") == 0){
    return "menu";
  }
  if(name.compare("omit double frames") == 0){
    return "menu";
  }
  if(name.compare("generator") == 0){
    return "menu";
  }
  if (m_Generator -> supportsProperty(name)){
    return m_Generator -> getType(name);
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unkown property");
}

// get information of a properties valid values
std::string OpenNIGrabberImpl::getInfo(const std::string &name){
  DEBUG_LOG("");
      if(name.compare("size") == 0){
    return "";
  }
  if(name.compare("format") == 0){
    return m_Generator -> getInfo("map output mode");
  }
  if(name.compare("omit double frames") == 0){
    return "{On,Off}";
  }
  if(name.compare("generator") == 0){
    return "{depth,rgb}";
  }
  if (m_Generator -> supportsProperty(name)){
    return m_Generator -> getInfo(name);
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unkown property");
}

// returns the current value of a property or a parameter
std::string OpenNIGrabberImpl::getValue(const std::string &name){
  DEBUG_LOG(name);
      if(name.compare("size") == 0){
    return "set by format";
  }
  if(name.compare("format") == 0){
    return "depth";
  }
  if(name.compare("omit double frames") == 0){
    return (m_OmitDoubleFrames) ? "On" : "Off";
  }
  if(name.compare("generator") == 0){
    return stringFromGenerator(m_Generator -> getGeneratorType());
  }
  if (m_Generator -> supportsProperty(name)){
    return m_Generator -> getValue(name);
  }
  return "error: property not found";
}

// Returns whether this property may be changed internally.
int OpenNIGrabberImpl::isVolatile(const std::string &propertyName){
  if (m_Generator -> supportsProperty(propertyName)){
    return m_Generator -> isVolatile(propertyName);
  }
  return true;
}
