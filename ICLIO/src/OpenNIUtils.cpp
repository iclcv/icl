/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/OpenNIUtils.cpp                              **
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
#include <ICLUtils/Macros.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/OpenNIUtils.h>
#include <limits>

#include <sstream>

using namespace xn;
using namespace icl;
using namespace icl_openni;

//##############################################################################
//############################# OpenNIImageGenerator ###########################
//##############################################################################

//  Creates the corresponding Generator.
OpenNIMapGenerator* OpenNIMapGenerator::createGenerator(
    xn::Context* context, std::string id)
{
  if(id == "depth"){
    return new OpenNIDepthGenerator(context, 0);
  } else if(id == "rgb"){
    return new OpenNIRgbGenerator(context, 0);
  } else if(id == "ir") {
    return new OpenNIIRGenerator(context, 0);
  }

  std::string type = id.substr(0,id.size()-1);
  int num = icl::parse<int>(id.substr(id.size()-1, id.npos));
  if(type == "depth"){
    return new OpenNIDepthGenerator(context, num);
  } else if(type == "rgb"){
    return new OpenNIRgbGenerator(context, num);
  } else if(type == "ir") {
    return new OpenNIIRGenerator(context, num);
  } else {
    std::ostringstream s;
    s << "Generator type '" << id << "' not supported.";
    std::cout << "Unknown generator type '" << id << "'." << std::endl;
    std::cout << "Supported generator types are: rgb, depth and ir" << std::endl;
    throw new ICLException(s.str());
  }
}

//##############################################################################
//############################# OpenNIDepthGenerator ###########################
//##############################################################################

// Creates a DepthGenerator from Context
OpenNIDepthGenerator::OpenNIDepthGenerator(Context* context, int num)
  : m_Context(context), m_DepthGenerator(NULL), m_Options(NULL)
{
  XnStatus status;
  m_DepthGenerator = new DepthGenerator();
  NodeInfoList l;
  m_Context -> EnumerateProductionTrees(XN_NODE_TYPE_DEPTH, NULL , l);
  int i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    if (i == num){
      NodeInfo ni = *it;
      status = m_Context -> CreateProductionTree(ni, *m_DepthGenerator);
    }
  }
  if(i <= num){
    std::ostringstream s;
    s << "Demanded depth generator nr " << num
      << " but only " << i << " available.";
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  if (status != XN_STATUS_OK){
    std::ostringstream s;
    s << "Generator init error " << xnGetStatusString(status);
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  m_Options = new MapGeneratorOptions(m_DepthGenerator);
  m_DepthGenerator -> StartGenerating();
}

// Destructor frees all resouurces
OpenNIDepthGenerator::~OpenNIDepthGenerator(){
  m_DepthGenerator -> StopGenerating();
  DEBUG_LOG("deleting depth gen");
  ICL_DELETE(m_DepthGenerator);
  ICL_DELETE(m_Options);
}

// setter function for video device properties
void OpenNIDepthGenerator::setProperty(const std::string &property, const std::string &value){
  m_Options -> setProperty(property, value);
}

// adds properties to propertylist
void OpenNIDepthGenerator::addPropertiesToList(std::vector<std::string> &properties){
  m_Options -> addPropertiesToList(properties);
}

// checks if property is supported
bool OpenNIDepthGenerator::supportsProperty(const std::string &property){
  return m_Options -> supportsProperty(property);
}

// get type of property
std::string OpenNIDepthGenerator::getType(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getType(name);
}

// get information of a properties valid values
std::string OpenNIDepthGenerator::getInfo(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getInfo(name);
}

// returns the current value of a property or a parameter
std::string OpenNIDepthGenerator::getValue(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getValue(name);
}

// Returns whether this property may be changed internally.
int OpenNIDepthGenerator::isVolatile(const std::string &propertyName){
  if(m_Options -> supportsProperty(propertyName)) return m_Options -> isVolatile(propertyName);
}

// grab function grabs an image
bool OpenNIDepthGenerator::acquireImage(ImgBase* dest){
  //Time t = Time::now();
  XnStatus rc = XN_STATUS_OK;

  // Read a new frame
  rc = m_Context -> WaitOneUpdateAll(*m_DepthGenerator);
  if (rc != XN_STATUS_OK)
  {
    DEBUG_LOG("Read failed: " << xnGetStatusString(rc));
    return false;
  }
  m_DepthGenerator -> GetMetaData(m_DepthMD);

  convertDepthImg(&m_DepthMD, dest -> as16s());
  //DEBUG_LOG("grabbed in " << t.age());
  return true;
}

// tells the type of the Generator
OpenNIMapGenerator::Generators OpenNIDepthGenerator::getGeneratorType(){
  return OpenNIMapGenerator::DEPTH;
}

// returns underlying xn::MapGenerator instance
MapGenerator* OpenNIDepthGenerator::getMapGenerator(){
  return m_DepthGenerator;
}

ImgBase* OpenNIDepthGenerator::initBuffer(){
  return new Img16s(Size(0,0), formatGray);
}

//##############################################################################
//############################# OpenNIRgbGenerator #############################
//##############################################################################

// Creates a RgbGenerator from Context
OpenNIRgbGenerator::OpenNIRgbGenerator(Context* context, int num)
  : m_Context(context), m_RgbGenerator(NULL), m_Options(NULL)
{
  XnStatus status;
  m_DepthGenerator = new DepthGenerator();
  if (XN_STATUS_OK != m_DepthGenerator -> Create(*m_Context)){
    std::string error =  xnGetStatusString(status);
    DEBUG_LOG("DepthGenerator init error '" << error << "'");
    throw new ICLException(error);
  }
  m_RgbGenerator = new ImageGenerator();
  NodeInfoList l;
  m_Context -> EnumerateProductionTrees(XN_NODE_TYPE_IMAGE, NULL , l);
  int i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    if (i == num){
      NodeInfo ni = *it;
      status = m_Context -> CreateProductionTree(ni, *m_RgbGenerator);
    }
  }
  if(i <= num){
    std::ostringstream s;
    s << "Demanded rgb generator nr " << num
      << " but only " << i << " available.";
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  if (status != XN_STATUS_OK){
    std::string error =  xnGetStatusString(status);
    DEBUG_LOG("ImageGenerator init error '" << error << "'");
    throw new ICLException(error);
  }

  m_Options = new MapGeneratorOptions(m_RgbGenerator);
  m_RgbGenerator -> StartGenerating();
  DEBUG_LOG("done creating OpenNIRgbGenerator");
}

// Destructor frees all resouurces
OpenNIRgbGenerator::~OpenNIRgbGenerator(){
  m_RgbGenerator -> StopGenerating();
  ICL_DELETE(m_RgbGenerator);
  ICL_DELETE(m_IrGenerator);
  ICL_DELETE(m_DepthGenerator);
  ICL_DELETE(m_Options);
}

// setter function for video device properties
void OpenNIRgbGenerator::setProperty(const std::string &property, const std::string &value){
  m_Options -> setProperty(property, value);
}

// adds properties to propertylist
void OpenNIRgbGenerator::addPropertiesToList(std::vector<std::string> &properties){
  m_Options -> addPropertiesToList(properties);
}

// checks if property is supported
bool OpenNIRgbGenerator::supportsProperty(const std::string &property){
  return m_Options -> supportsProperty(property);
}

// get type of property
std::string OpenNIRgbGenerator::getType(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getType(name);
}

// get information of a properties valid values
std::string OpenNIRgbGenerator::getInfo(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getInfo(name);
}

// returns the current value of a property or a parameter
std::string OpenNIRgbGenerator::getValue(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getValue(name);
}

// Returns whether this property may be changed internally.
int OpenNIRgbGenerator::isVolatile(const std::string &propertyName){
  if(m_Options -> supportsProperty(propertyName)) return m_Options -> isVolatile(propertyName);
}

// grab function grabs an image
bool OpenNIRgbGenerator::acquireImage(ImgBase* dest){
  //Time t = Time::now();
  XnStatus rc = XN_STATUS_OK;

  // Read a new frame
  //DEBUG_LOG("wait context")
  //Time t3 = Time::now();
  rc = m_Context -> WaitOneUpdateAll(*m_RgbGenerator);
  //DEBUG_LOG("wait context: " << t3.age());
  if (rc != XN_STATUS_OK)
  {
    //DEBUG_LOG("Read failed: " << xnGetStatusString(rc))
    return false;
  }
  //DEBUG_LOG("getmeta")
  m_RgbGenerator -> GetMetaData(m_RgbMD);
  //DEBUG_LOG("getmeta_")
  convertRGBImg(&m_RgbMD, dest->as8u());
  //DEBUG_LOG("copy in " << t2.age());
  //DEBUG_LOG("grabbed in " << t.age());
  return true;
}

// tells the type of the Generator
OpenNIMapGenerator::Generators OpenNIRgbGenerator::getGeneratorType(){
  return OpenNIMapGenerator::RGB;
}

// returns underlying xn::MapGenerator instance
MapGenerator* OpenNIRgbGenerator::getMapGenerator(){
  return m_RgbGenerator;
}

Img8u* OpenNIRgbGenerator::initBuffer(){
  return new Img8u(Size(0,0), formatRGB);
}

//##############################################################################
//############################# OpenNIIRGenerator #############################
//##############################################################################

// Creates a IrGenerator from Context
OpenNIIRGenerator::OpenNIIRGenerator(Context* context, int num)
  : m_Context(context), m_IrGenerator(NULL), m_Options(NULL)
{
  XnStatus status;
  m_IrGenerator = new IRGenerator();
  NodeInfoList l;
  m_Context -> EnumerateProductionTrees(XN_NODE_TYPE_IR, NULL , l);
  int i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    if (i == num){
      NodeInfo ni = *it;
      status = m_Context -> CreateProductionTree(ni, *m_IrGenerator);
    }
  }
  if(i <= num){
    std::ostringstream s;
    s << "Demanded ir generator nr " << num
      << " but only " << i << " available.";
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  if (XN_STATUS_OK != status){
    std::string error =  xnGetStatusString(status);
    DEBUG_LOG("IRGenerator init error '" << error << "'");
    throw new ICLException(error);
  }

  m_Options = new MapGeneratorOptions(m_IrGenerator);
  // somehow my kinect did not create the ir generator before setting it to
  // this MapOutputMode.
  XnMapOutputMode mo;
  mo.nFPS = 30;
  mo.nXRes = 640;
  mo.nYRes = 480;
  m_IrGenerator -> SetMapOutputMode(mo);
  status = m_IrGenerator -> StartGenerating();
  DEBUG_LOG("startgenerating: " << xnGetStatusString(status));
  DEBUG_LOG("done creating OpenNIIRGenerator");
}

// Destructor frees all resouurces
OpenNIIRGenerator::~OpenNIIRGenerator(){
  m_IrGenerator -> StopGenerating();
  ICL_DELETE(m_IrGenerator);
  ICL_DELETE(m_Options);
}

// setter function for video device properties
void OpenNIIRGenerator::setProperty(const std::string &property, const std::string &value){
  m_Options -> setProperty(property, value);
}

// adds properties to propertylist
void OpenNIIRGenerator::addPropertiesToList(std::vector<std::string> &properties){
  m_Options -> addPropertiesToList(properties);
}

// checks if property is supported
bool OpenNIIRGenerator::supportsProperty(const std::string &property){
  return m_Options -> supportsProperty(property);
}

// get type of property
std::string OpenNIIRGenerator::getType(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getType(name);
}

// get information of a properties valid values
std::string OpenNIIRGenerator::getInfo(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getInfo(name);
}

// returns the current value of a property or a parameter
std::string OpenNIIRGenerator::getValue(const std::string &name){
  if(m_Options -> supportsProperty(name)) return m_Options -> getValue(name);
}

// Returns whether this property may be changed internally.
int OpenNIIRGenerator::isVolatile(const std::string &propertyName){
  if(m_Options -> supportsProperty(propertyName)) return m_Options -> isVolatile(propertyName);
}

// grab function grabs an image
bool OpenNIIRGenerator::acquireImage(ImgBase* dest){
  XnStatus rc = XN_STATUS_OK;
  Time t = Time::now();
  // Read a new frame
  rc = m_Context -> WaitOneUpdateAll(*m_IrGenerator);
  t = t.now();
  if (rc != XN_STATUS_OK)
  {
    return false;
  }
  m_IrGenerator -> GetMetaData(m_IrMD);
  t = t.now();
  convertIRImg(&m_IrMD, dest -> as16s());
  t = t.now();
  return true;
}

// tells the type of the Generator
OpenNIMapGenerator::Generators OpenNIIRGenerator::getGeneratorType(){
  return OpenNIMapGenerator::IR;
}

// returns underlying xn::MapGenerator instance
MapGenerator* OpenNIIRGenerator::getMapGenerator(){
  return m_IrGenerator;
}

Img16s* OpenNIIRGenerator::initBuffer(){
  return new Img16s(Size(0,0), formatGray);
}

//##############################################################################
//############################# MapGeneratorOptions ############################
//##############################################################################

std::string getMapOutputModeInfo(MapGenerator* gen){
  XnUInt32 count = gen -> GetSupportedMapOutputModesCount();
  XnMapOutputMode* modes = new XnMapOutputMode[count];
  gen -> GetSupportedMapOutputModes(modes, count);
  // remove double entries
  std::vector<XnMapOutputMode*> cleaned;
  for(unsigned int i = 0; i < count; ++i){
    bool added = false;
    for (unsigned int j = 0; j < cleaned.size(); ++j){
      if(modes[i].nXRes == cleaned.at(j) -> nXRes &&
         modes[i].nYRes == cleaned.at(j) -> nYRes &&
         modes[i].nFPS == cleaned.at(j) -> nFPS){
        added = true;
        break;
      }
    }
    if(!added) cleaned.push_back(modes + i);
  }
  // create info-string
  std::ostringstream ret;
  ret << "{";
  for(unsigned int i = 0; i < cleaned.size(); ++i){
    ret << cleaned.at(i) -> nXRes << "x";
    ret << cleaned.at(i) -> nYRes << "@";
    ret << cleaned.at(i) -> nFPS << "fps";
    if(i+1 < cleaned.size()){
      ret << ",";
    }
  }
  delete[] modes;
  DEBUG_LOG("supported map output modes: " << ret.str());
  return ret.str();
}

bool isSupportedMapOutputMode(MapGenerator* gen, XnMapOutputMode* mode){
  XnUInt32 count = gen -> GetSupportedMapOutputModesCount();
  XnMapOutputMode* modes = new XnMapOutputMode[count];
  gen -> GetSupportedMapOutputModes(modes, count);
  std::ostringstream ret;
  ret << "{";
  for(unsigned int i = 0; i < count; ++i){
    if(modes[i].nXRes == mode->nXRes
       && modes[i].nYRes == mode->nYRes
       && modes[i].nFPS == mode->nFPS ){
      return true;
    }
  }
  return false;
}

std::string getCurrentMapOutputMode(MapGenerator* gen){
  XnMapOutputMode mode;
  gen -> GetMapOutputMode(mode);
  std::ostringstream ret;
  ret << mode.nXRes << "x" << mode.nYRes << "@" << mode.nFPS << "fps";
  DEBUG_LOG("Map output mode: " << ret.str());
  return ret.str();
}

void setCurrentMapOutputmode(MapGenerator* gen, const std::string &value){
  char delimiter;
  int val = 0;
  XnMapOutputMode mode;
  std::stringstream t;
  t << value;
  t >> mode.nXRes;
  t >> delimiter;
  t >> mode.nYRes;
  t >> delimiter;
  t >> mode.nFPS;
  if(isSupportedMapOutputMode(gen, &mode)){
    XnStatus st;
    st = gen -> SetMapOutputMode(mode);
    DEBUG_LOG(xnGetStatusString(st));
  } else {
    DEBUG_LOG("mode " << value << " not supported.");
  }
}

bool isGeneralIntCapability(const std::string &name){
  if (name == XN_CAPABILITY_BRIGHTNESS
      || name == XN_CAPABILITY_CONTRAST
      || name == XN_CAPABILITY_HUE
      || name == XN_CAPABILITY_SATURATION
      || name == XN_CAPABILITY_SHARPNESS
      || name == XN_CAPABILITY_GAMMA
      || name == XN_CAPABILITY_COLOR_TEMPERATURE
      || name == XN_CAPABILITY_BACKLIGHT_COMPENSATION
      || name == XN_CAPABILITY_GAIN
      || name == XN_CAPABILITY_PAN
      || name == XN_CAPABILITY_TILT
      || name == XN_CAPABILITY_ROLL
      || name == XN_CAPABILITY_ZOOM
      || name == XN_CAPABILITY_EXPOSURE
      || name == XN_CAPABILITY_IRIS
      || name == XN_CAPABILITY_FOCUS
      || name == XN_CAPABILITY_LOW_LIGHT_COMPENSATION){
    return true;
  }
  return false;
}

GeneralIntCapability
getGeneralIntCapability(xn::MapGenerator* gen, const std::string &name){
  if (name == XN_CAPABILITY_BRIGHTNESS){
    return gen -> GetBrightnessCap();
  } else if (name == XN_CAPABILITY_CONTRAST){
    return gen -> GetContrastCap();
  } else if (name == XN_CAPABILITY_HUE){
    return gen -> GetHueCap();
  } else if (name == XN_CAPABILITY_SATURATION){
    return gen -> GetSaturationCap();
  } else if (name == XN_CAPABILITY_SHARPNESS){
    return gen -> GetSharpnessCap();
  } else if (name == XN_CAPABILITY_GAMMA){
    return gen -> GetGammaCap();
  } else if (name == XN_CAPABILITY_COLOR_TEMPERATURE){
    return gen -> GetWhiteBalanceCap();
  } else if (name == XN_CAPABILITY_BACKLIGHT_COMPENSATION){
    return gen -> GetBacklightCompensationCap();
  } else if (name == XN_CAPABILITY_GAIN){
    return gen -> GetGainCap();
  } else if (name == XN_CAPABILITY_PAN){
    return gen -> GetPanCap();
  } else if (name == XN_CAPABILITY_TILT){
    return gen -> GetTiltCap();
  } else if (name == XN_CAPABILITY_ROLL){
    return gen -> GetRollCap();
  } else if (name == XN_CAPABILITY_ZOOM){
    return gen -> GetZoomCap();
  } else if (name == XN_CAPABILITY_EXPOSURE){
    return gen -> GetExposureCap();
  } else if (name == XN_CAPABILITY_IRIS){
    return gen -> GetIrisCap();
  } else if (name == XN_CAPABILITY_FOCUS){
    return gen -> GetFocusCap();
  } else if (name == XN_CAPABILITY_LOW_LIGHT_COMPENSATION){
    return gen -> GetLowLightCompensationCap();
  }
  throw ICLException("unknown int capability");
}

bool isGeneralIntAutoSupported(GeneralIntCapability &cap){
  XnInt32 min, max, step, def;
  XnBool sauto;
  cap.GetRange(min, max, step, def, sauto);
  return sauto;
}

void addGeneralIntCapability(xn::MapGenerator* gen,
                             std::vector<std::string> &properties,
                             const std::string &name)
{
  if(!gen -> IsCapabilitySupported(name.c_str())){
    return;
  }
  GeneralIntCapability cap = getGeneralIntCapability(gen, name);
  if(isGeneralIntAutoSupported(cap)){
    std::ostringstream tmp;
    tmp << "Auto" << name;
    properties.push_back(tmp.str());
  }
  properties.push_back(name);
}

std::string generalIntCapabilityInfo(GeneralIntCapability cap){
  XnInt32 min, max, step, def;
  XnBool sauto;
  cap.GetRange(min, max, step, def, sauto);
  std::ostringstream ret;
  ret << "[" << min << "," << max << "]:" << step;
  return ret.str();
}

void setGeneralIntCapabilityDefault(GeneralIntCapability cap){
  XnInt32 min, max, step, def;
  XnBool sauto;
  cap.GetRange(min, max, step, def, sauto);
  cap.Set(def);
}

std::string generalIntCapabilityValue(GeneralIntCapability cap){
  std::ostringstream ret;
  ret << cap.Get();
  return ret.str();
}

std::string antiFlickerCapabilityValue(xn::MapGenerator* gen){
  XnPowerLineFrequency curr = gen -> GetAntiFlickerCap().GetPowerLineFrequency();
  if(curr == XN_POWER_LINE_FREQUENCY_OFF){
    return "Power line frequency OFF";
  } else if (curr == XN_POWER_LINE_FREQUENCY_50_HZ){
    return "Power line frequency 50Hz";
  } else if (curr == XN_POWER_LINE_FREQUENCY_60_HZ){
    return "Power line frequency 60Hz";
  } else {
    return "error";
  }
}

void antiFlickerCapabilitySet(xn::MapGenerator* gen, std::string value){
  AntiFlickerCapability cap = gen -> GetAntiFlickerCap();
  if(value == "Power line frequency OFF"){
    cap.SetPowerLineFrequency(XN_POWER_LINE_FREQUENCY_OFF);
  } else if (value == "Power line frequency 50Hz"){
    cap.SetPowerLineFrequency(XN_POWER_LINE_FREQUENCY_50_HZ);
  } else if (value == "Power line frequency 60Hz"){
    cap.SetPowerLineFrequency(XN_POWER_LINE_FREQUENCY_60_HZ);
  }
}

bool isGeneralIntAutoCapability(const std::string &property){
  if (property.size() <= 4){ // Auto capabilities have 'Auto' prepended.
    return false;
  }
  std::string prop = property.substr(4);
  if (!isGeneralIntCapability(prop)){
    return false;
  }
  return true;
}

bool setGeneralIntCapability(xn::MapGenerator* gen,
                             const std::string &property,
                             const std::string &value){
  if(isGeneralIntCapability(property)){
    getGeneralIntCapability(gen, property).Set(icl::parse<int>(value));
    return true;
  }
  if(isGeneralIntAutoCapability(property)){
    if(value == "On"){
      getGeneralIntCapability(gen, property).Set(XN_AUTO_CONTROL);
    } else {
      GeneralIntCapability cap = getGeneralIntCapability(gen, property);
      setGeneralIntCapabilityDefault(cap);
    }
    return true;
  }
  return false;
}

void addCroppingCapability(std::vector<std::string> &capabilities){
  capabilities.push_back("Cropping Enabled");
  capabilities.push_back("Cropping offset X");
  capabilities.push_back("Cropping offset Y");
  capabilities.push_back("Cropping size X");
  capabilities.push_back("Cropping size Y");
}

bool isCropping(const std::string &property){
  if (property == "Cropping Enabled"
      || property == "Cropping offset X"
      || property == "Cropping offset Y"
      || property == "Cropping size X"
      || property == "Cropping size Y"){
    return true;
  }
  return false;
}

void setCropping(xn::MapGenerator* gen,
                 const std::string &property,
                 const std::string &value)
{
  XnCropping crop;
  gen -> GetCroppingCap().GetCropping(crop);
  if (property == "Cropping Enabled"){
    if(value == "On"){
      crop.bEnabled = true;
    } else {
      crop.bEnabled = false;
    }
  } else {
    unsigned int val = icl::parse<int>(value);
    XnMapOutputMode mode;
    gen -> GetMapOutputMode(mode);
    if (property == "Cropping offset X"){
      if(val + crop.nXSize <= mode.nXRes){
        crop.nXOffset = val;
      }
    } else if (property == "Cropping offset Y"){
      if(val + crop.nYSize <= mode.nYRes){
        crop.nYOffset = val;
      }
    } else if (property == "Cropping size X"){
      if(val + crop.nXOffset <= mode.nXRes){
        crop.nXSize = val;
      }
    } else if (property == "Cropping size Y"){
      if(val + crop.nXOffset <= mode.nYRes){
        crop.nYSize = val;
      }
    }
  }
  gen -> GetCroppingCap().SetCropping(crop);
}

std::string getCroppingValue(xn::MapGenerator* gen, const std::string &name){
  XnCropping crop;
  gen -> GetCroppingCap().GetCropping(crop);
  if (name == "Cropping Enabled"){
    if(crop.bEnabled){
      return "On";
    } else {
      return "Off";
    }
  } else  if (name == "Cropping offset X"){
    return icl::toStr(crop.nXOffset);
  } else if (name == "Cropping offset Y"){
    return icl::toStr(crop.nYOffset);
  } else if (name == "Cropping size X"){
    return icl::toStr(crop.nXSize);
  } else if (name == "Cropping size Y"){
    return icl::toStr(crop.nYSize);
  }
  DEBUG_LOG("unknown cropping type " << name);
  throw ICLException("unknown cropping type");
}

std::string getCroppingType(const std::string &property){
  if (property == "Cropping Enabled"){
    return "menu";
  } else  if (property == "Cropping offset X"){
    return "range";
  } else if (property == "Cropping offset Y"){
    return "range";
  } else if (property == "Cropping size X"){
    return "range";
  } else if (property == "Cropping size Y"){
    return "range";
  }
  DEBUG_LOG("unknown cropping type " << property);
  throw ICLException("unknown cropping type");
}

std::string getCroppingInfo(xn::MapGenerator* gen, const std::string &property){
  XnCropping crop;
  gen -> GetCroppingCap().GetCropping(crop);
  if (property == "Cropping Enabled"){
    return "{On,Off}";
  } else {
    // get max map output in every for x and y
    int x = 0; int y = 0;
    XnUInt32 count = gen -> GetSupportedMapOutputModesCount();
    XnMapOutputMode* modes = new XnMapOutputMode[count];
    gen -> GetSupportedMapOutputModes(modes, count);
    for(unsigned int i = 0; i < count; ++i){
      x = (modes[i].nXRes > x) ? modes[i].nXRes : x;
      y = (modes[i].nYRes > y) ? modes[i].nYRes : y;
    }
    // write info
    std::ostringstream tmp;
    if (property == "Cropping offset X"){
      tmp << "[0," << x << "]:1";
    } else if (property == "Cropping offset Y"){
      tmp << "[0," << y << "]:1";
    } else if (property == "Cropping size X"){
      tmp << "[0," << x << "]:1";
    } else if (property == "Cropping size Y"){
      tmp << "[0," << y << "]:1";
    }
    return tmp.str();
  }
}

void alternativeViewPiontCapabilitySet(xn::MapGenerator* gen,
                                       const std::string &value,
                                       std::map<std::string, xn::ProductionNode> &pn_map)
{
  AlternativeViewPointCapability avc = gen -> GetAlternativeViewPointCap();
  ProductionNode n;
  XnStatus status;
  if(value == "self"){
    status = avc.ResetViewPoint();
  } else {
    std::map<std::string, xn::ProductionNode>::iterator it = pn_map.find(value);
    if (it != pn_map.end()) {
      if(avc.IsViewPointSupported(pn_map[value])){
        status = avc.SetViewPoint(pn_map[value]);
      }
      if(status != XN_STATUS_OK){
        DEBUG_LOG("Setting Viewpoint returned error: " << xnGetStatusString(status));
      }
    } else {
      DEBUG_LOG("ProductinNode " << value << " for alt. Viewpoint not found.");
    }
  }
}

std::string alternativeViewPiontCapabilityValue(xn::MapGenerator* gen,
                                                std::map<std::string, xn::ProductionNode> &pn_map)
{
  AlternativeViewPointCapability avc = gen -> GetAlternativeViewPointCap();
  std::map<std::string, xn::ProductionNode>::iterator it;
  for(it = pn_map.begin(); it != pn_map.end(); ++it){
    if(avc.IsViewPointAs((*it).second)){
      return (*it).first;
    }
  }
  return "self";
}

std::string alternativeViewPiontCapabilityInfo(xn::MapGenerator* gen,
                                               std::map<std::string, xn::ProductionNode> &pn_map)
{
  AlternativeViewPointCapability avc = gen -> GetAlternativeViewPointCap();
  std::ostringstream ret;
  ret << "{self,";
  std::map<std::string, xn::ProductionNode>::iterator it;
  for(it = pn_map.begin(); it != pn_map.end(); ++it){
    if(avc.IsViewPointSupported((*it).second)){
      ret << (*it).first << ",";
    }
  }
  return ret.str();
}

void fillProductionNodeMap(Context context,
                           std::map<std::string, xn::ProductionNode> &pn_map)
{
  ProductionNode n;
  XnStatus status = XN_STATUS_OK;
  NodeInfoList l;
  // RGB
  context.EnumerateProductionTrees(XN_NODE_TYPE_IMAGE, NULL , l);
  int i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    std::ostringstream tmp;
    tmp << "rgb";
    if(i) tmp << i;
    NodeInfo ni = *it;
    status = context.CreateProductionTree(ni, n);
    if(status == XN_STATUS_OK){
      pn_map[tmp.str()] = n;
    } else {
      DEBUG_LOG("error while creating Production tree: " << xnGetStatusString(status));
    }
  }
  // DEPTH
  context.EnumerateProductionTrees(XN_NODE_TYPE_DEPTH, NULL , l);
  i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    std::ostringstream tmp;
    tmp << "depth";
    if(i) tmp << i;
    NodeInfo ni = *it;
    status = context.CreateProductionTree(ni, n);
    if(status == XN_STATUS_OK){
      pn_map[tmp.str()] = n;
    } else {
      DEBUG_LOG("error while creating Production tree: " << xnGetStatusString(status));
    }
  }
  // IR
  context.EnumerateProductionTrees(XN_NODE_TYPE_IR, NULL , l);
  i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    std::ostringstream tmp;
    tmp << "ir";
    if(i) tmp << i;
    NodeInfo ni = *it;
    status = context.CreateProductionTree(ni, n);
    if(status == XN_STATUS_OK){
      pn_map[tmp.str()] = n;
    } else {
      DEBUG_LOG("error while creating Production tree: " << xnGetStatusString(status));
    }
  }
  // AUDIO
  context.EnumerateProductionTrees(XN_NODE_TYPE_AUDIO, NULL , l);
  i = 0;
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    std::ostringstream tmp;
    tmp << "audio";
    if(i) tmp << i;
    NodeInfo ni = *it;
    status = context.CreateProductionTree(ni, n);
    if(status == XN_STATUS_OK){
      pn_map[tmp.str()] = n;
    } else {
      DEBUG_LOG("error while creating Production tree: " << xnGetStatusString(status));
    }
  }
}

MapGeneratorOptions::MapGeneratorOptions(xn::MapGenerator* generator)
  : m_Generator(generator)
{
  m_Capabilities.push_back("map output mode");
  fillProductionNodeMap(m_Generator -> GetContext(), m_ProductionNodeMap);
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_CROPPING)){
    addCroppingCapability(m_Capabilities);
  }
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_ANTI_FLICKER)){
    m_Capabilities.push_back(XN_CAPABILITY_ANTI_FLICKER);
  }
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)){
    m_Capabilities.push_back(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT);
  }
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_MIRROR)){
    m_Capabilities.push_back(XN_CAPABILITY_MIRROR);
  }
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_BRIGHTNESS);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_CONTRAST);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_HUE);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_SATURATION);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_SHARPNESS);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_GAMMA);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_COLOR_TEMPERATURE);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_BACKLIGHT_COMPENSATION);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_GAIN);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_PAN);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_TILT);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_ROLL);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_ZOOM);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_EXPOSURE);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_IRIS);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_FOCUS);
  addGeneralIntCapability(m_Generator, m_Capabilities, XN_CAPABILITY_LOW_LIGHT_COMPENSATION);
}

// adds properties to propertylist
void MapGeneratorOptions::addPropertiesToList(
    std::vector<std::string> &properties){
  properties.insert(properties.end(),
                    m_Capabilities.begin(),
                    m_Capabilities.end());
}

// checks if property is supported
bool MapGeneratorOptions::supportsProperty(const std::string &property){
  for (unsigned int i = 0; i < m_Capabilities.size(); ++i){
    if(m_Capabilities.at(i) == property) return true;
  }
  return false;
}

// interface for the setter function for video device properties
void MapGeneratorOptions::setProperty(
    const std::string &property, const std::string &value){
  if(isCropping(property)){
    setCropping(m_Generator, property, value);
  } else if (property == XN_CAPABILITY_ANTI_FLICKER){
    antiFlickerCapabilitySet(m_Generator, value);
  } else if (property == XN_CAPABILITY_ALTERNATIVE_VIEW_POINT){
    alternativeViewPiontCapabilitySet(m_Generator, value, m_ProductionNodeMap);
  } else if (property == XN_CAPABILITY_MIRROR){
    m_Generator -> GetMirrorCap().SetMirror(value == "On");
  } else if (setGeneralIntCapability(m_Generator, property, value)){
    // nothing to do setting is done in condition
  } else if (property == "map output mode"){
    setCurrentMapOutputmode(m_Generator, value);
  }
}

// get type of property
std::string MapGeneratorOptions::getType(const std::string &name){
  if(isCropping(name)){
    return getCroppingType(name);
  } else if (name == XN_CAPABILITY_ANTI_FLICKER){
    return "menu";
  } else if (name == XN_CAPABILITY_ALTERNATIVE_VIEW_POINT){
    return "menu";
  } else if (name == XN_CAPABILITY_MIRROR){
    return "menu";
  } else if (isGeneralIntCapability(name)){
    return "range";
  } else if(name == "map output mode"){
    return "menu";
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// get information of a properties valid values
std::string MapGeneratorOptions::getInfo(const std::string &name){
  if(isCropping(name)){
    return getCroppingInfo(m_Generator, name);
  } else if (name == XN_CAPABILITY_ANTI_FLICKER){
    return "{Power line frequency OFF,"
        "Power line frequency 50Hz,"
        "Power line frequency 60Hz}";
  } else if (name == XN_CAPABILITY_ALTERNATIVE_VIEW_POINT){
    return alternativeViewPiontCapabilityInfo(m_Generator, m_ProductionNodeMap);
  } else if (name == XN_CAPABILITY_MIRROR){
    return "{On,Off}";
  } else if (isGeneralIntCapability(name)){
    return generalIntCapabilityInfo(getGeneralIntCapability(m_Generator, name));
  } else if (isGeneralIntAutoCapability(name)){
    return "{On,Off}";
  } else if(name == "map output mode"){
    return getMapOutputModeInfo(m_Generator);
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// returns the current value of a property or a parameter
std::string MapGeneratorOptions::getValue(const std::string &name){
  if(isCropping(name)){
    return getCroppingValue(m_Generator, name);
  } else if (name == XN_CAPABILITY_ANTI_FLICKER){
    return antiFlickerCapabilityValue(m_Generator);
  } else if (name == XN_CAPABILITY_ALTERNATIVE_VIEW_POINT){
    return alternativeViewPiontCapabilityValue(m_Generator, m_ProductionNodeMap);
  } else if (name == XN_CAPABILITY_MIRROR){
     return (m_Generator -> GetMirrorCap().IsMirrored()) ? "On" : "Off";
  } else if (isGeneralIntCapability(name)){
    return generalIntCapabilityValue(getGeneralIntCapability(m_Generator, name));
  } else if (isGeneralIntAutoCapability(name)){
    std::string aname = name.substr(4);
    return generalIntCapabilityValue(getGeneralIntCapability(m_Generator, aname));
  } else if(name == "map output mode"){
    return getCurrentMapOutputMode(m_Generator);
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// Returns whether this property may be changed internally.
int MapGeneratorOptions::isVolatile(const std::string &propertyName){
  if(isCropping(propertyName)){
    return 100;
  }
  return 0;
}
