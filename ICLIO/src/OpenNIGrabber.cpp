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

void printNodeInfo(Context cont){
    NodeInfoList l;
    XnStatus rc = cont.EnumerateExistingNodes(l);
    DEBUG_LOG("XnStatus: " << xnGetStatusString(rc))
    XnProductionNodeDescription pi;
    DEBUG_LOG("NodeInfoList")
    for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it){
        pi = (*it).GetDescription();
        DEBUG_LOG(pi.strName)
        DEBUG_LOG(pi.strVendor)
        DEBUG_LOG(pi.Type)
                DEBUG_LOG(pi.Version.nMajor << " : "
                          << pi.Version.nMinor << " : "
                          << pi.Version.nMaintenance << " : ")
    }
    if(l.Begin() == l.End()){
        DEBUG_LOG("The nodelist is empty \n")
    } else {
        std::cout << std::endl;
    }
}

void printProductionTrees(Context cont){
  EnumerationErrors errors;
  NodeInfoList l;
  XnStatus rc = cont.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL , l, &errors);
  DEBUG_LOG("XnStatus: " << xnGetStatusString(rc))
  DEBUG_LOG("ProductionTree - NodeInfoList")
  int i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it){
      NodeInfo deviceNodeInfo = *it;
      Device deviceNode;

      deviceNodeInfo.GetInstance(deviceNode);
      XnBool bExists = deviceNode.IsValid();

      if (!bExists) {
        DEBUG_LOG1("Node not valid. CreateProductionTree")
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
        DEBUG_LOG(" [" << i << "]" << deviceName << "(" << deviceSN << ")")
                DEBUG_LOG(deviceNodeInfo.GetAdditionalData())
                DEBUG_LOG(deviceNodeInfo.GetCreationInfo())
                DEBUG_LOG(deviceNodeInfo.GetDescription().strName)
                DEBUG_LOG(deviceNodeInfo.GetDescription().strVendor)
                DEBUG_LOG(deviceNodeInfo.GetDescription().Type)
                DEBUG_LOG(deviceNodeInfo.GetDescription().Version.nMajor)
                DEBUG_LOG(deviceNodeInfo.GetInstanceName())
                DEBUG_LOG("CAM: " << treeStrRep)
      }
      i++;
  }
}



void printSupportedMapOutputModes(MapGenerator gen){
  XnUInt32 count = gen.GetSupportedMapOutputModesCount();
  DEBUG_LOG("count: " << count)
  XnMapOutputMode* modes = new XnMapOutputMode[count];
  XnStatus rc = gen.GetSupportedMapOutputModes(modes, count);
  DEBUG_LOG("XnStatus: " << xnGetStatusString(rc))
  DEBUG_LOG("SupportedMapOutputModes: " << count)
  for(int i = 0; i < count; ++i){
      DEBUG_LOG("fps: " << modes[i].nFPS)
      DEBUG_LOG("xRes: " << modes[i].nXRes)
      DEBUG_LOG("yRes: " << modes[i].nYRes)
  }
  std::cout << std::endl;
  delete[] modes;
}

void assertStatus(XnStatus &status){
  if (status != XN_STATUS_OK){
    throw new ICLException("Status " + *xnGetStatusString(status));
  }
}

//##############################################################################
//############################# OpenNIGrabberImpl ##############################
//##############################################################################

// Constructor of OpenNIGrabberImpl
OpenNIGrabberImpl::OpenNIGrabberImpl(std::string name, std::string args)
    : m_AutoContext(), m_Context(m_AutoContext.getContextPtr()), m_Device(NULL),
      m_GeneratorLock(), m_Generator(NULL),
      m_SetToGenerator(OpenNIImageGenerator::RGB)
{
  icl::Mutex::Locker lock(m_GeneratorLock);
  XnStatus rc;
  rc = m_Context -> Init();
  assertStatus(rc);

  m_Device = createDeviceFromName(name);

//  m_Generator = OpenNIImageGenerator::
//                  createGenerator(m_Context, OpenNIImageGenerator::RGB);

  // Set it to VGA maps at 30 FPS
  // XnMapOutputMode mapMode;
  // mapMode.nXRes = 1280;
  // mapMode.nYRes = 1024;
  // mapMode.nFPS = 30;

  // printSupportedMapOutputModes(m_Image);
  // rc = m_Image.SetMapOutputMode(mapMode);

  DEBUG_LOG("init done")
}

OpenNIGrabberImpl::~OpenNIGrabberImpl(){
    DEBUG_LOG("");
    m_Context -> Release();
}

const icl::ImgBase* OpenNIGrabberImpl::acquireImage(){
  Mutex::Locker l(m_GeneratorLock);
  if(m_SetToGenerator != OpenNIImageGenerator::NOT_SPECIFIED){
      ICL_DELETE(m_Generator)
      m_Generator = OpenNIImageGenerator::
                  createGenerator(m_Context, m_SetToGenerator);
      m_SetToGenerator = OpenNIImageGenerator::NOT_SPECIFIED;
  }
  return m_Generator -> acquireImage();
}

// Returns the string representation of the currently used device.
std::string OpenNIGrabberImpl::getName(){
    return getTreeStringRepresentation(*m_Device);
}

// default name
std::string OpenNIGrabberImpl::getTreeStringRepresentation(NodeInfo info){
  //XnChar treeRep[200];
  //info.GetTreeStringRepresentation(treeRep, 200);
  DEBUG_LOG("info:")
  std::ostringstream str;
  str << info.GetDescription().strName;
  str << info.GetDescription().strVendor;
  str << info.GetDescription().Type;
  return str.str();
}

// gets progargs and finds the corresponding device
std::string OpenNIGrabberImpl::getDeviceNodeNameFromArgs(std::string args){
  OpenNIAutoContext c;
  unsigned int nr = icl::parse<int>(args);
  xn::NodeInfoList devices;
  c.getContextPtr() ->
          EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL , devices, NULL);
  if(devices.IsEmpty()){
    DEBUG_LOG("No devices available.")
    throw ICLException("No devices available.");
  }
  DEBUG_LOG("search device")
  int i = 0;
  for(NodeInfoList::Iterator it = devices.Begin();
      it != devices.End(); ++i, ++it){
    if(i != nr){
      continue;
    }
    return getTreeStringRepresentation(*it);
  }
  std::cout << "Demanded device Nr. " << nr
            << " but only " << i << "devices available.";
  throw ICLException("Could not find demanded device.");
}

NodeInfo* OpenNIGrabberImpl::createDeviceFromName(std::string name){
    xn::NodeInfoList devices;
    m_Context ->
            EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL , devices, NULL);
    if(devices.IsEmpty()){
      DEBUG_LOG("No devices available.")
      throw ICLException("No devices available.");
    }
    DEBUG_LOG("search device")
    int i = 0;
    for(NodeInfoList::Iterator it = devices.Begin();
        it != devices.End(); ++i, ++it){
      if(!getTreeStringRepresentation(*it).compare(name)){
        return new NodeInfo(*it);
      }
    }
    DEBUG_LOG("could not find device: " << name)
    throw ICLException("Could not find demanded device.");
}

// setter function for video device properties
void OpenNIGrabberImpl::setProperty(
  const std::string &property, const std::string &value)
{
  DEBUG_LOG(property << " := " << value)
  if (property.compare("size") == 0){
      DEBUG_LOG("not implemented")
  }
  if (property.compare("format") == 0){
      DEBUG_LOG("not implemented")
  }
  if (property.compare("generator") == 0){
      setGeneratorTo(value);
  }
}

// returns a list of properties, that can be set using setProperty
std::vector<std::string> OpenNIGrabberImpl::getPropertyList(){
    DEBUG_LOG("")
  std::vector<std::string> ps;
  ps.push_back("size");
  ps.push_back("format");
  ps.push_back("generator");
  return ps;
}

bool OpenNIGrabberImpl::supportsProperty(const std::string &property){
    DEBUG_LOG("");
  if (property.compare("size") == 0) return true;
  if (property.compare("format") == 0) return true;
  if (property.compare("generator") == 0) return true;
  return false;
}

// get type of property
std::string OpenNIGrabberImpl::getType(const std::string &name){
    DEBUG_LOG("")
  if(name.compare("size") == 0){
      return "menu";
  }
  if(name.compare("format") == 0){
      return "menu";
  }
  if(name.compare("generator") == 0){
      return "menu";
  }
}

// get information of a properties valid values
std::string OpenNIGrabberImpl::getInfo(const std::string &name){
    DEBUG_LOG("")
    if(name.compare("size") == 0){
        return "{320x240,640x480}";
    }
    if(name.compare("format") == 0){
        return "{depth,rgb}";
    }
    if(name.compare("generator") == 0){
      return "{depth,rgb}";
    }
}

// returns the current value of a property or a parameter
std::string OpenNIGrabberImpl::getValue(const std::string &name){
  DEBUG_LOG(name)
  if(name.compare("size") == 0){
    return "640x480";
  }
  if(name.compare("format") == 0){
    return "depth";
  }
  if(name.compare("generator") == 0){
      switch(m_Generator->getType()){
        case OpenNIImageGenerator::DEPTH:
          return "depth";
        case OpenNIImageGenerator::RGB:
          return "rgb";
      }
  }
  return "error: property not found";
}

// Returns whether this property may be changed internally.
int OpenNIGrabberImpl::isVolatile(const std::string &propertyName){
  return true;
}

// switches generator.
void OpenNIGrabberImpl::setGeneratorTo(std::string value){
    if(!value.compare("depth")){
      if(m_Generator->getType() == OpenNIImageGenerator::DEPTH){
        return;
      }
      Mutex::Locker l(m_GeneratorLock);
      m_SetToGenerator = OpenNIImageGenerator::DEPTH;
      return;
    }
    if(!value.compare("rgb")){
        if(m_Generator->getType() == OpenNIImageGenerator::RGB){
          return;
        }
        Mutex::Locker l(m_GeneratorLock);
        m_SetToGenerator = OpenNIImageGenerator::RGB;
        return;
    }
    DEBUG_LOG("value " << value << " not found.")
}
