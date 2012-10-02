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
using namespace utils;
using namespace core;
using namespace io;
using namespace icl_openni;

//##############################################################################
//############################# OpenNIImageGenerator ###########################
//##############################################################################

//  Creates the corresponding Generator.
OpenNIMapGenerator* OpenNIMapGenerator::createGenerator(
    xn::Context* context, std::string id)
{
  // create generator from string
  if(id == "depth"){
    return new OpenNIDepthGenerator(context, 0);
  } else if(id == "rgb"){
    return new OpenNIRgbGenerator(context, 0);
  } else if(id == "ir") {
    return new OpenNIIRGenerator(context, 0);
  }

  // create generator from string with index
  std::string type = id.substr(0,id.size()-1);
  int num = utils::parse<int>(id.substr(id.size()-1, id.npos));
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
  // enumerate depth generators
  m_Context -> EnumerateProductionTrees(XN_NODE_TYPE_DEPTH, NULL , l);
  int i = 0;
  // look for generator according to number num
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    if (i == num){
      NodeInfo ni = *it;
      status = m_Context -> CreateProductionTree(ni, *m_DepthGenerator);
    }
  }
  if(i <= num){ // not enough generators
    std::ostringstream s;
    s << "Demanded depth generator nr " << num
      << " but only " << i << " available.";
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  if (status != XN_STATUS_OK){ // error while creating depth generator
    std::ostringstream s;
    s << "Generator init error " << xnGetStatusString(status);
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  // create GeneratorOptions for DepthGenerator
  m_Options = new DepthGeneratorOptions(m_DepthGenerator);
  m_DepthGenerator -> StartGenerating();
}

// Destructor frees all resouurces
OpenNIDepthGenerator::~OpenNIDepthGenerator(){
  m_DepthGenerator -> StopGenerating();
  DEBUG_LOG2("deleting depth gen");
  ICL_DELETE(m_DepthGenerator);
  ICL_DELETE(m_Options);
}

// grab function grabs an image
bool OpenNIDepthGenerator::acquireImage(ImgBase* dest){
  XnStatus rc = XN_STATUS_OK;
  // Read a new frame
  rc = m_Context -> WaitOneUpdateAll(*m_DepthGenerator);
  if (rc != XN_STATUS_OK)
  {
    DEBUG_LOG2("Read failed: " << xnGetStatusString(rc));
    return false;
  }

  // get data and write to image
  m_DepthGenerator -> GetMetaData(m_DepthMD);
  convertDepthImg(&m_DepthMD, dest -> as16s());
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

// getter for MapGeneratorOptions
MapGeneratorOptions* OpenNIDepthGenerator::getMapGeneratorOptions(){
  return m_Options;
}

//##############################################################################
//############################# OpenNIRgbGenerator #############################
//##############################################################################

// Creates a RgbGenerator from Context
OpenNIRgbGenerator::OpenNIRgbGenerator(Context* context, int num)
  : m_Context(context), m_RgbGenerator(NULL), m_Options(NULL)
{
  XnStatus status;
  // create DepthGenerator. The Kinect rgb-generator did not work without
  // a depth generator being initialized beforehand.
  m_DepthGenerator = new DepthGenerator();
  if (XN_STATUS_OK != (status = m_DepthGenerator -> Create(*m_Context))){
    std::string error =  xnGetStatusString(status);
    DEBUG_LOG("DepthGenerator init error '" << error << "'");
    throw new ICLException(error);
  }
  m_RgbGenerator = new ImageGenerator();
  NodeInfoList l;
  // get all rgb-image generators
  m_Context -> EnumerateProductionTrees(XN_NODE_TYPE_IMAGE, NULL , l);
  int i = 0;
  // create generator according to num
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    if (i == num){
      NodeInfo ni = *it;
      status = m_Context -> CreateProductionTree(ni, *m_RgbGenerator);
    }
  }
  if(i <= num){ // not enough generators found
    std::ostringstream s;
    s << "Demanded rgb generator nr " << num
      << " but only " << i << " available.";
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  if (status != XN_STATUS_OK){ // error while creating
    std::string error =  xnGetStatusString(status);
    DEBUG_LOG("ImageGenerator init error '" << error << "'");
    throw new ICLException(error);
  }

  // create generator options
  m_Options = new MapGeneratorOptions(m_RgbGenerator);
  m_RgbGenerator -> StartGenerating();
  DEBUG_LOG2("done creating OpenNIRgbGenerator");
}

// Destructor frees all resouurces
OpenNIRgbGenerator::~OpenNIRgbGenerator(){
  m_RgbGenerator -> StopGenerating();
  ICL_DELETE(m_RgbGenerator);
  ICL_DELETE(m_IrGenerator);
  ICL_DELETE(m_DepthGenerator);
  ICL_DELETE(m_Options);
}

// grab function grabs an image
bool OpenNIRgbGenerator::acquireImage(ImgBase* dest){
  XnStatus rc = XN_STATUS_OK;

  // Read a new frame
  rc = m_Context -> WaitOneUpdateAll(*m_RgbGenerator);
  if (rc != XN_STATUS_OK)
  {
    DEBUG_LOG2("Read failed: " << xnGetStatusString(rc))
        return false;
  }
  // get data and write to image
  m_RgbGenerator -> GetMetaData(m_RgbMD);
  convertRGBImg(&m_RgbMD, dest->as8u());
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

// getter for MapGeneratorOptions
MapGeneratorOptions* OpenNIRgbGenerator::getMapGeneratorOptions(){
  return m_Options;
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
  // enumerate ir generators
  m_Context -> EnumerateProductionTrees(XN_NODE_TYPE_IR, NULL , l);
  int i = 0;
  // create generator according to num
  for (NodeInfoList::Iterator it = l.Begin(); it != l.End(); ++it, ++i){
    if (i == num){
      NodeInfo ni = *it;
      status = m_Context -> CreateProductionTree(ni, *m_IrGenerator);
    }
  }
  if(i <= num){ // not enough generators
    std::ostringstream s;
    s << "Demanded ir generator nr " << num
      << " but only " << i << " available.";
    DEBUG_LOG(s.str());
    throw ICLException(s.str());
  }
  if (XN_STATUS_OK != status){ // error while creating generator
    std::string error =  xnGetStatusString(status);
    DEBUG_LOG("IRGenerator init error '" << error << "'");
    throw new ICLException(error);
  }

  // create generator options
  m_Options = new MapGeneratorOptions(m_IrGenerator);
  // somehow my kinect did not create the ir images before setting it to
  // this MapOutputMode.
  XnMapOutputMode mo;
  mo.nFPS = 30;
  mo.nXRes = 640;
  mo.nYRes = 480;
  m_IrGenerator -> SetMapOutputMode(mo);
  status = m_IrGenerator -> StartGenerating();
  DEBUG_LOG2("startgenerating: " << xnGetStatusString(status));
}

// Destructor frees all resouurces
OpenNIIRGenerator::~OpenNIIRGenerator(){
  m_IrGenerator -> StopGenerating();
  ICL_DELETE(m_IrGenerator);
  ICL_DELETE(m_Options);
}

// grab function grabs an image
bool OpenNIIRGenerator::acquireImage(ImgBase* dest){
  XnStatus rc = XN_STATUS_OK;
  // Read a new frame
  rc = m_Context -> WaitOneUpdateAll(*m_IrGenerator);
  if (rc != XN_STATUS_OK)
  {
    return false;
  }
  // get data wnd write image
  m_IrGenerator -> GetMetaData(m_IrMD);
  convertIRImg(&m_IrMD, dest -> as16s());
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


// getter for MapGeneratorOptions
MapGeneratorOptions* OpenNIIRGenerator::getMapGeneratorOptions(){
  return m_Options;
}

//##############################################################################
//############################# MapGeneratorOptions ############################
//##############################################################################

// creates an info string for MapOutputModes of MapGenerator gen.
std::string getMapOutputModeInfo(MapGenerator* gen){
  // list all MapOutputModes
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
  // free allocated memory
  delete[] modes;
  DEBUG_LOG2("supported map output modes: " << ret.str());
  return ret.str();
}

// tells whether a MapOutputMode is supported by a MapGenerator
bool isSupportedMapOutputMode(MapGenerator* gen, XnMapOutputMode* mode){
  // list modes
  XnUInt32 count = gen -> GetSupportedMapOutputModesCount();
  XnMapOutputMode* modes = new XnMapOutputMode[count];
  gen -> GetSupportedMapOutputModes(modes, count);
  // check whether one is equivalent
  for(unsigned int i = 0; i < count; ++i){
    if(modes[i].nXRes == mode->nXRes
       && modes[i].nYRes == mode->nYRes
       && modes[i].nFPS == mode->nFPS ){
      return true;
    }
  }
  return false;
}

// creates a string describing the current MapOutputMode
std::string getCurrentMapOutputMode(MapGenerator* gen){
  XnMapOutputMode mode;
  gen -> GetMapOutputMode(mode);
  std::ostringstream ret;
  ret << mode.nXRes << "x" << mode.nYRes << "@" << mode.nFPS << "fps";
  DEBUG_LOG2("Map output mode: " << ret.str());
  return ret.str();
}

// sets the current MapOutputMode from string.
void setCurrentMapOutputmode(MapGenerator* gen, const std::string &value){
  // fill XnMapOutputMode instance from string
  char delimiter;
  XnMapOutputMode mode;
  std::stringstream t;
  t << value;
  t >> mode.nXRes;
  t >> delimiter;
  t >> mode.nYRes;
  t >> delimiter;
  t >> mode.nFPS;
  // when supported, set to new mode.
  if(isSupportedMapOutputMode(gen, &mode)){
    //XnStatus st = gen -> SetMapOutputMode(mode);
    //DEBUG_LOG2(xnGetStatusString(st));
    gen -> SetMapOutputMode(mode);
  } else {
    DEBUG_LOG2("mode " << value << " not supported.");
  }
}

// checks whether name is a GeneralIntCapability
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

// gets the GeneralIntCapability corresponding to name.
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

// checks whether the GeneralIntCapability supports auto-mode
bool isGeneralIntAutoSupported(GeneralIntCapability &cap){
  XnInt32 min, max, step, def;
  XnBool sauto;
  cap.GetRange(min, max, step, def, sauto);
  return sauto;
}

// adds a GeneralIntCapability to properties vector
void addGeneralIntCapability(xn::MapGenerator* gen,
                             std::vector<std::string> &properties,
                             const std::string &name)
{
  // only add iff supported
  if(!gen -> IsCapabilitySupported(name.c_str())){
    return;
  }
  GeneralIntCapability cap = getGeneralIntCapability(gen, name);
  // when auto is supported add the auto-version too.
  if(isGeneralIntAutoSupported(cap)){
    std::ostringstream tmp;
    tmp << "Auto" << name;
    properties.push_back(tmp.str());
  }
  properties.push_back(name);
}

// creates an info string for a GeneralIntCapability
std::string generalIntCapabilityInfo(GeneralIntCapability cap){
  XnInt32 min, max, step, def;
  XnBool sauto;
  cap.GetRange(min, max, step, def, sauto);
  std::ostringstream ret;
  ret << "[" << min << "," << max << "]:" << step;
  return ret.str();
}

// sets a GeneralIntCapability to Default.
void setGeneralIntCapabilityDefault(GeneralIntCapability cap){
  XnInt32 min, max, step, def;
  XnBool sauto;
  cap.GetRange(min, max, step, def, sauto);
  cap.Set(def);
}

// returns the string-value of a GeneralIntCapability
std::string generalIntCapabilityValue(GeneralIntCapability cap){
  std::ostringstream ret;
  ret << cap.Get();
  return ret.str();
}

// checks whether property is an auto-GeneralIntCapability
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

// sets a GeneralIntCapability from name and value string.
bool setGeneralIntCapability(xn::MapGenerator* gen,
                             const std::string &property,
                             const std::string &value){
  // set when its a GeneralIntCapability
  if(isGeneralIntCapability(property)){
    getGeneralIntCapability(gen, property).Set(parse<int>(value));
    return true;
  }
  // when its an auto-capability set accordingly
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

// returns the string-value of the andi-flicker capability of MapGenerator
std::string antiFlickerCapabilityValue(xn::MapGenerator* gen){
  // yeah anti-flicker is power-line-freq...
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

// sets anti flicker capability from string
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

// adds cropping properties to capability vector
void addCroppingCapability(std::vector<std::string> &capabilities){
  capabilities.push_back("Cropping Enabled");
  capabilities.push_back("Cropping offset X");
  capabilities.push_back("Cropping offset Y");
  capabilities.push_back("Cropping size X");
  capabilities.push_back("Cropping size Y");
}

// checks whether property is a cropping property
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

// sets a cropping property from name and value string
void setCropping(xn::MapGenerator* gen,
                 const std::string &property,
                 const std::string &value)
{
  XnCropping crop;
  gen -> GetCroppingCap().GetCropping(crop);
  // go through all cropping properties and set the one named by property-string
  if (property == "Cropping Enabled"){
    if(value == "On"){
      crop.bEnabled = true;
    } else {
      crop.bEnabled = false;
    }
  } else {
    unsigned int val = parse<int>(value);
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

// creates a value-string for cropping property named by name.
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
    return toStr(crop.nXOffset);
  } else if (name == "Cropping offset Y"){
    return toStr(crop.nYOffset);
  } else if (name == "Cropping size X"){
    return toStr(crop.nXSize);
  } else if (name == "Cropping size Y"){
    return toStr(crop.nYSize);
  }
  DEBUG_LOG("unknown cropping type " << name);
  throw ICLException("unknown cropping type");
}

// creates a type-string for cropping properties
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

// creates info strings for cropping properties.
std::string getCroppingInfo(xn::MapGenerator* gen, const std::string &property){
  XnCropping crop;
  gen -> GetCroppingCap().GetCropping(crop);
  if (property == "Cropping Enabled"){
    return "{On,Off}";
  } else {
    // get max map output in every for x and y
    unsigned int x = 0; unsigned int y = 0;
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

// sets alternative viewpoit
void alternativeViewPiontCapabilitySet(xn::MapGenerator* gen,
                                       const std::string &value,
                                       std::map<std::string, xn::ProductionNode> &pn_map)
{
  // get alt. viewpoint capapility
  AlternativeViewPointCapability avc = gen -> GetAlternativeViewPointCap();
  ProductionNode n;
  XnStatus status;
  // if self, reset viewpoint to default.
  if(value == "self"){
    status = avc.ResetViewPoint();
  } else {
    // get the ProductionNode named by value from Map
    std::map<std::string, xn::ProductionNode>::iterator it = pn_map.find(value);
    if (it != pn_map.end()) {
      if(avc.IsViewPointSupported(pn_map[value])){
        // set alt viewpoint.
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

// creates string representation of alt. viewpoint capability value
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

// creates info string for alt viewpoint capability
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

// fills a Map with available ProductionNodes. used for altern. viewpoint.
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

// fills capabilities list.
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

//##############################################################################
//############################# DepthGeneratorOptions ##########################
//##############################################################################

// constructor
DepthGeneratorOptions::DepthGeneratorOptions(xn::DepthGenerator* generator)
  : MapGeneratorOptions(generator), m_DepthGenerator(generator)
{/* nothing to do */}

// interface for the setter function for video device properties
void DepthGeneratorOptions::setProperty(const std::string &property, const std::string &value){
  MapGeneratorOptions::setProperty(property, value);
  if(property == "max depth"){
    return;
  } else if (property == "field of view X"){
    return;
  } else if (property == "field of view Y"){
    return;
  }
}

// adds properties to propertylist
void DepthGeneratorOptions::addPropertiesToList(std::vector<std::string> &properties){
  MapGeneratorOptions::addPropertiesToList(properties);
  properties.push_back("max depth");
  properties.push_back("field of view X");
  properties.push_back("field of view Y");
}

// checks if property is supported
bool DepthGeneratorOptions::supportsProperty(const std::string &property){
  if(MapGeneratorOptions::supportsProperty(property)){
    return true;
  }
  if(property == "max depth"){
    return true;
  } else if (property == "field of view X"){
    return true;
  } else if (property == "field of view Y"){
    return true;
  }
  return false;
}

// get type of property
std::string DepthGeneratorOptions::getType(const std::string &name){
  if(MapGeneratorOptions::supportsProperty(name)){
    return MapGeneratorOptions::getType(name);
  }
  if(name == "max depth"){
    return "info";
  } else if (name == "field of view X"){
    return "info";
  } else if (name == "field of view Y"){
    return "info";
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// get information of a properties valid values
std::string DepthGeneratorOptions::getInfo(const std::string &name){
  if(MapGeneratorOptions::supportsProperty(name)){
    return MapGeneratorOptions::getInfo(name);
  }
  if(name == "max depth"){
    return "";
  } else if (name == "field of view X"){
    return "";
  } else if (name == "field of view Y"){
    return "";
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// returns the current value of a property or a parameter
std::string DepthGeneratorOptions::getValue(const std::string &name){
  if(MapGeneratorOptions::supportsProperty(name)){
    return MapGeneratorOptions::getValue(name);
  }
  std::ostringstream s;
  if(name == "max depth"){
    s << m_DepthGenerator -> GetDeviceMaxDepth();
    return s.str();
  } else if (name == "field of view X"){
    XnFieldOfView fov;
    m_DepthGenerator -> GetFieldOfView(fov);
    s << fov.fHFOV;
    return s.str();
  } else if (name == "field of view Y"){
    XnFieldOfView fov;
    m_DepthGenerator -> GetFieldOfView(fov);
    s << fov.fVFOV;
    return s.str();
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// Returns whether this property may be changed internally.
int DepthGeneratorOptions::isVolatile(const std::string &propertyName){
  if(MapGeneratorOptions::supportsProperty(propertyName)){
    return MapGeneratorOptions::isVolatile(propertyName);
  }
  return 0;
}

//##############################################################################
//############################# ImageGeneratorOptions ##########################
//#### abandoned for now because the kinect freezes after changing to yuv422 ###
//##############################################################################

// constructor
ImageGeneratorOptions::ImageGeneratorOptions(xn::ImageGenerator* generator)
  : MapGeneratorOptions(generator), m_ImageGenerator(generator)
{/* nothing to do */}

// interface for the setter function for video device properties
void ImageGeneratorOptions::setProperty(const std::string &property, const std::string &value){
  MapGeneratorOptions::setProperty(property, value);
  if(property == "Pixel Format"){
    if (value == "rgb24"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_RGB24)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_RGB24);
      }
    }
    if (value == "yuv422"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_YUV422)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_YUV422);
      }
    }
    if (value == "grayscale8"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_8_BIT)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_GRAYSCALE_8_BIT);
      }
    }
    if (value == "grayscale16"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_16_BIT)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_GRAYSCALE_16_BIT);
      }
    }
    if (value == "mjpeg"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_MJPEG)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_MJPEG);
      }
    }
  }
}

// adds properties to propertylist
void ImageGeneratorOptions::addPropertiesToList(std::vector<std::string> &properties){
  MapGeneratorOptions::addPropertiesToList(properties);
  properties.push_back("Pixel Format");
}

// checks if property is supported
bool ImageGeneratorOptions::supportsProperty(const std::string &property){
  if(MapGeneratorOptions::supportsProperty(property)){
    return true;
  }
  if(property == "Pixel Format"){
    return true;
  }
  DEBUG_LOG("unknown property " << property);
  throw ICLException("unknown property");
}

// get type of property
std::string ImageGeneratorOptions::getType(const std::string &name){
  if(MapGeneratorOptions::supportsProperty(name)){
    return MapGeneratorOptions::getType(name);
  }
  if(name == "Pixel Format"){
    return "menu";
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// get information of a properties valid values
std::string ImageGeneratorOptions::getInfo(const std::string &name){
  if(MapGeneratorOptions::supportsProperty(name)){
    return MapGeneratorOptions::getInfo(name);
  }
  if(name == "Pixel Format"){
    std::ostringstream s;
    s << "{";
    if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_RGB24)){
      s << "rgb24,";
    }
    if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_YUV422)){
      s << "yuv422,";
    }
    if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_8_BIT)){
      s << "grayscale8,";
    }
    if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_16_BIT)){
      s << "grayscale16,";
    }
    if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_MJPEG)){
      s << "mjpeg,";
    }
    s << "}";
    return s.str();
  }
  DEBUG_LOG("unknown property " << name);
  throw ICLException("unknown property");
}

// returns the current value of a property or a parameter
std::string ImageGeneratorOptions::getValue(const std::string &name){
  if(MapGeneratorOptions::supportsProperty(name)){
    return MapGeneratorOptions::getValue(name);
  }
  if(name == "Pixel Format"){
    XnPixelFormat f = m_ImageGenerator -> GetPixelFormat();
    switch(f){
      case XN_PIXEL_FORMAT_RGB24:
        return "rgb24";
      case XN_PIXEL_FORMAT_YUV422:
        return "yuv422";
      case XN_PIXEL_FORMAT_GRAYSCALE_8_BIT:
        return "grayscale8";
      case XN_PIXEL_FORMAT_GRAYSCALE_16_BIT:
        return "grayscale16";
      case XN_PIXEL_FORMAT_MJPEG:
        return "mjpeg";
      default:
        DEBUG_LOG("Unknown Pixel Format " << f)
            return "rgb24";
    }
  }
  ERROR_LOG("unknown property " << name);
  return "";
}

// Returns whether this property may be changed internally.
int ImageGeneratorOptions::isVolatile(const std::string &propertyName){
  if(MapGeneratorOptions::supportsProperty(propertyName)){
    return MapGeneratorOptions::isVolatile(propertyName);
  }
  if (propertyName == "Pixel Format"){
    return 1000;
  }
  return 0;
}
