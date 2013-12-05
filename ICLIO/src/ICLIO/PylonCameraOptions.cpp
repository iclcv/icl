/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PylonCameraOptions.cpp                 **
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
#include <ICLIO/PylonCameraOptions.h>
#include <ICLUtils/StringUtils.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::io::pylon;

// This is a list of dafault sizes strings for
// the 'size' parameter of the camera
const std::string default_sizes =
    "128x96,176x144,160x120,320x200,320x240,352x288,360x240,480x320,"
    "640x350,640x480,768x576,800x480,800x600,960x540,960x640,1024x768,"
    "1152x864,1200x800,1280x720,1280x800,1440x900,1280x960,1280x1024,"
    "1600x900,1400x1050,1600x1050,1600x1200,1920x1080,3840x2160";

PylonCameraOptions::PylonCameraOptions(
    Pylon::IPylonDevice* camera, Interruptable* grabber){
  m_Interu = grabber;
  m_Camera = camera;
  m_OmitDoubleFrames = true;

  // Configurable
  addProperty("size", "menu", default_sizes, getValue("Width") + "x" + getValue("Height"), 0, "");
  addProperty("format", getType("PixelFormat"), getInfo("PixelFormat"), getValue("PixelFormat"), 0, "");
  addProperty("OmitDoubleFrames", "flag", "", m_OmitDoubleFrames, 0, "");

  // Cameras default options
  std::vector<std::string> ps;
  std::vector<std::string>::iterator it;
  addToPropertyList(ps, getNode("Root"));
  for(it = ps.begin(); it != ps.end(); ++it){
    addProperty(*it, getType(*it), getInfo(*it), getValue(*it), isVolatile(*it), ""); //TODO getTooltip
  }

  Configurable::registerCallback(utils::function(this,&PylonCameraOptions::processPropertyChange));
}

PylonCameraOptions::~PylonCameraOptions(){
  // nothing to do
}

// callback for changed configurable properties
void PylonCameraOptions::processPropertyChange(const utils::Configurable::Property &prop){
  //DEBUG_LOG(prop.name << " = " << prop.value);
  if(prop.name == "size"){
    Size size(prop.value);
    setPropertyValue("Width", size.width);
    setPropertyValue("Height", size.height);
  } else if(prop.name == "format"){
    setPropertyValue("PixelFormat", prop.value);
  } else if(prop.name == "OmitDoubleFrames"){
    m_OmitDoubleFrames = parse<bool>(prop.value);
  } else {
    GenApi::CValuePtr node = getNode(prop.name);
    if (!node) return;
    AcquisitionInterruptor a(m_Interu, GenApi::IsWritable(node));
    GrabbingInterruptor g(m_Interu, GenApi::IsWritable(node));
    if(!GenApi::IsWritable(node)){
      DEBUG_LOG2("The parameter '" << prop.name << "' is not writable");
      return;
    }
    try {
      node -> FromString(prop.value.c_str(), true);
    } catch (GenICam::GenericException &e) {
      DEBUG_LOG2("catched exception: " << e.what());
    }
  }
}

// helper function for getPropertyList, recursive
void PylonCameraOptions::addToPropertyList(
    std::vector<std::string> &ps, const GenApi::CNodePtr& node)
{
  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) {
    case GenApi::intfIInteger:
    case GenApi::intfIFloat:
    case GenApi::intfIBoolean:
    case GenApi::intfIEnumeration:
      if (GenApi::IsReadable(node) || GenApi::IsWritable(node)){
        // If the feature is implemented, add name of node
        ps.push_back(node->GetName().c_str());
      }
      return;

    case GenApi::intfICategory:
    {
      GenApi::CCategoryPtr ptrCategory (node);
      if (ptrCategory) {
        // add name of category
        GenApi::FeatureList_t features;
        ptrCategory -> GetFeatures(features);
        GenApi::FeatureList_t::const_iterator it;
        for (it = features.begin(); it != features.end(); ++it){
          addToPropertyList(ps, *it);
        }
      }
      return;
    }
      // for now we ignore these nodes
    case GenApi::intfICommand:
    case GenApi::intfIEnumEntry:
    case GenApi::intfIString:
    case GenApi::intfIRegister:
    case GenApi::intfIPort:
    case GenApi::intfIBase:
    default:
      return;
  }
}

// get type of property
std::string PylonCameraOptions::getType(const std::string &name){
  FUNCTION_LOG(name)
      GenApi::INode* node = getNode(name);
  if (!node) {
    DEBUG_LOG("There is no parameter called '" << name << "'")
        return "info";
  }

  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) {
    case GenApi::intfIInteger:
    case GenApi::intfIFloat:
      return "range";

    case GenApi::intfIBoolean:
      return "flag";

    case GenApi::intfIEnumeration:
      return "menu";

    case GenApi::intfICommand:
      return "command";

      // for now these stay info nodes
    case GenApi::intfIEnumEntry:
    case GenApi::intfIString:
    case GenApi::intfIRegister:
    case GenApi::intfICategory:
    case GenApi::intfIPort:
    case GenApi::intfIBase:
    default:
      return "info";
  }
}

// creates an info string for integer-nodes when possible
std::string intInfo(GenApi::INode* node){
  GenApi::IInteger* inode = NULL;
  if((inode = dynamic_cast<GenApi::IInteger*>(node))) {
    std::ostringstream ret;
    ret << "[" << inode -> GetMin() << "," << inode -> GetMax()
        //<< "]:" << inode -> GetInc(); // gui does not support != 1
        << "]:1";
    return ret.str();
  } else {
    return "";
  }
}

// creates an info string for float-nodes when possible
std::string floatInfo(GenApi::INode* node){
  GenApi::IFloat* fnode;
  if((fnode = dynamic_cast<GenApi::IFloat*>(node))) {
    std::ostringstream ret;
    ret << "[" << fnode -> GetMin() << "," << fnode -> GetMax() << "]";
    /*if(fnode -> HasInc()){
      ret << ":" << fnode -> GetInc();
    }*/ // gui does not support != 1
    return ret.str();
  } else {
    return "";
  }
}

// creates an info string for enum-nodes when possible
std::string enumInfo(GenApi::INode* node){
  GenApi::IEnumeration* enode;
  if((enode = dynamic_cast<GenApi::IEnumeration*>(node))) {
    std::ostringstream ret;
    Pylon::StringList_t symbols;
    enode -> GetSymbolics(symbols);
    Pylon::StringList_t::iterator it;
    for (it = symbols.begin(); it != symbols.end(); ++it){
      ret << it -> c_str();
      if(it+1 != symbols.end()){
        ret << ",";
      }
    }
    return ret.str();
  } else {
    return "";
  }
}

// get information of a properties valid values
std::string PylonCameraOptions::getInfo(const std::string &name){
  FUNCTION_LOG(name)
      GenApi::INode* node = getNode(name);
  if (!node) {
    DEBUG_LOG("There is no parameter called '" << name << "'")
        return "";
  }
  std::string ret = "";
  GenApi::EInterfaceType type = node -> GetPrincipalInterfaceType();
  switch (type) {
    case GenApi::intfIInteger:
      ret = intInfo(node);
      break;
    case GenApi::intfIFloat:
      ret = floatInfo(node);
      break;
    case GenApi::intfIEnumeration:
      ret = enumInfo(node);
      break;
      // no info needed
    case GenApi::intfIBoolean:
    case GenApi::intfICommand:
    case GenApi::intfIEnumEntry:
    case GenApi::intfIString:
    case GenApi::intfIRegister:
    case GenApi::intfICategory:
    case GenApi::intfIPort:
    case GenApi::intfIBase:
    default:
      break;
  }
  return ret;
}

// returns the current value of a property or a parameter
std::string PylonCameraOptions::getValue(const std::string &name){
  return getParameterValueString(m_Camera, name);
}

// Returns whether this property may be changed internally.
int PylonCameraOptions::isVolatile(const std::string &propertyName){
  // can't guarantee anything, sorry.
  return 0;
}

// whether double frames should be omitted.
bool PylonCameraOptions::omitDoubleFrames(){
  return m_OmitDoubleFrames;
}

// getter for the current expected framerate
double PylonCameraOptions::getResultingFrameRateAbs(){
  return parse<double>(getValue("ResultingFrameRateAbs"));
}

GenApi::INode *PylonCameraOptions::getNode(std::string name){
  return m_Camera -> GetNodeMap() -> GetNode(name.c_str());
}

// returns the Pylon::PixelType enum describing the cameras current pixel type
Pylon::PixelType PylonCameraOptions::getCameraPixelType(){
  std::string format = getFormatString();
  Pylon::PixelType type = Pylon::CPixelTypeMapper::GetPylonPixelTypeByName(
        GenICam::gcstring(format.c_str()));
  if(type == Pylon::PixelType_Undefined){
    DEBUG_LOG("PixelType " << format << "=" << type << "' is not defined.");
  }
  return type;
}

// returns the expected pixel size.
int PylonCameraOptions::getCameraPixelSize(){
  return Pylon::BitPerPixel(getCameraPixelType());
}

// TODO: check size...somehow looks awkward. remove division? what about 16bit?
// returns the expected image size. used to get the needed size of buffers.
long PylonCameraOptions::getNeededBufferSize(){
  try {
    return getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(
          m_Camera, "PayloadSize");
  } catch (ICLException &e){
    DEBUG_LOG("The camera does not have a parameter called PayloadSize.")
        DEBUG_LOG("Assuming that the image size is width * height * pixelsize.")
        return (long) ((getWidth() * getHeight() * getCameraPixelSize() / 8) + 0.5);
  }
}

// Executes the ICommand 'AcquisitionStart'
void PylonCameraOptions::acquisitionStart(){
  GenApi::INode* node = getNode("AcquisitionStart");
  GenApi::ICommand* node2 = dynamic_cast<GenApi::ICommand*>(node);
  node2 -> Execute();
}

// Executes the ICommand 'AcquisitionStop'
void PylonCameraOptions::acquisitionStop(){
  GenApi::INode* node = getNode("AcquisitionStop");
  GenApi::ICommand* node2 = dynamic_cast<GenApi::ICommand*>(node);
  node2 -> Execute();
}

// getter for the camera image height.
int PylonCameraOptions::getHeight(){
  return getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(
        m_Camera, "Height");
}

// getter for the camera image width.
int PylonCameraOptions::getWidth(){
  return getParameterValueOf<Pylon::IPylonDevice, GenApi::IInteger, int>(
        m_Camera, "Width");
}

// returns the cameras PixelFormat as string
std::string PylonCameraOptions::getFormatString(){
  return getParameterValueString(m_Camera, "PixelFormat");
}

// returns a string representation of the value of a parameter of the camera.
std::string icl::io::pylon::getParameterValueString(
    Pylon::IPylonDevice* device, std::string parameter)
{
  GenApi::INode* node = device -> GetNodeMap() -> GetNode(parameter.c_str());
  if (!node) {
    DEBUG_LOG("There is no parameter called '" << parameter << "'")
        return "";
  }
  GenApi::IValue* value = dynamic_cast<GenApi::IValue*>(node);
  if (!value) {
    DEBUG_LOG("Could not cast '" << parameter << "' node to IValue")
        return "";
  }
  return (value -> ToString()).c_str();
}
