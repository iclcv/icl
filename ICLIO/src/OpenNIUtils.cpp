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
#include <ICLUtils/SteppingRange.h>
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
//############################# OpenNIMapGenerator #############################
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

// creates an info string for MapOutputModes of MapGenerator gen.
std::string OpenNIMapGenerator::getMapOutputModeInfo(MapGenerator* gen){
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


// creates a string describing the current MapOutputMode
std::string OpenNIMapGenerator::getCurrentMapOutputMode(MapGenerator* gen){
  XnMapOutputMode mode;
  gen -> GetMapOutputMode(mode);
  std::ostringstream ret;
  ret << mode.nXRes << "x" << mode.nYRes << "@" << mode.nFPS << "fps";
  DEBUG_LOG2("Map output mode: " << ret.str());
  return ret.str();
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
  m_Options = new ImageGeneratorOptions(m_RgbGenerator);
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

  // somehow my kinect did not create the ir images before setting it to
  // this MapOutputMode.
  XnMapOutputMode mo;
  mo.nFPS = 30;
  mo.nXRes = 640;
  mo.nYRes = 480;
  m_IrGenerator -> SetMapOutputMode(mo);

  // create generator options
  m_Options = new MapGeneratorOptions(m_IrGenerator);
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
  ret << "self,";
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
  fillProductionNodeMap(m_Generator -> GetContext(), m_ProductionNodeMap);
  //Configurable
  addProperty("map output mode", "menu", OpenNIMapGenerator::getMapOutputModeInfo(m_Generator),
              OpenNIMapGenerator::getCurrentMapOutputMode(m_Generator), 0,
              "The map output mode of the used MapGenerator");
  //fillProductionNodeMap(m_Generator -> GetContext(), m_ProductionNodeMap);
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_CROPPING)){
    // get max map output in every for x and y
    unsigned int x = 0; unsigned int y = 0;
    XnUInt32 count = m_Generator -> GetSupportedMapOutputModesCount();
    XnMapOutputMode* modes = new XnMapOutputMode[count];
    m_Generator -> GetSupportedMapOutputModes(modes, count);
    for(unsigned int i = 0; i < count; ++i){
      x = (modes[i].nXRes > x) ? modes[i].nXRes : x;
      y = (modes[i].nYRes > y) ? modes[i].nYRes : y;
    }
    // cropping info
    XnCropping crop;
    m_Generator -> GetCroppingCap().GetCropping(crop);

    addProperty("Cropping Enabled", "flag", "", crop.bEnabled, 0,
                "Whether cropping should be used."
                );
    addProperty("Cropping offset X", "range", str(SteppingRange<int>(0, x, 1)),
                crop.nXOffset, 0, "The X offset of the cropping from (0,0)."
                "Needs to be set regarding cropping size and image size."
                );
    addProperty("Cropping offset Y", "range", str(SteppingRange<int>(0, y, 1)),
                crop.nYOffset, 0, "The Y offset of the cropping from (0,0). "
                "Needs to be set regarding cropping size and image size."
                );
    addProperty("Cropping size X", "range", str(SteppingRange<int>(0, x, 1)),
                crop.nXSize, 0, "The X size cropped image. Needs to be set "
                "regarding cropping size and image size."
                );
    addProperty("Cropping size Y", "range", str(SteppingRange<int>(0, y, 1)),
                crop.nYSize, 0, "The Y size cropped image. Needs to be set "
                "regarding cropping size and image size."
                );
  }
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_ANTI_FLICKER)){
    addProperty(XN_CAPABILITY_ANTI_FLICKER, "menu", "Power line frequency OFF,"
                "Power line frequency 50Hz,Power line frequency 60Hz",
                antiFlickerCapabilityValue(m_Generator), 0, ""
                );
  }
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)){
    addProperty(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT, "menu",
                alternativeViewPiontCapabilityInfo(m_Generator, m_ProductionNodeMap),
                alternativeViewPiontCapabilityValue(m_Generator, m_ProductionNodeMap),
                0, ""
                );
  }
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_MIRROR)){
    addProperty(XN_CAPABILITY_MIRROR, "flag", "",
                m_Generator -> GetMirrorCap().IsMirrored(), 0,
                "Flips the image vertically."
                );
  }
  addGeneralIntProperty(XN_CAPABILITY_BRIGHTNESS);
  addGeneralIntProperty(XN_CAPABILITY_CONTRAST);
  addGeneralIntProperty(XN_CAPABILITY_HUE);
  addGeneralIntProperty(XN_CAPABILITY_SATURATION);
  addGeneralIntProperty(XN_CAPABILITY_SHARPNESS);
  addGeneralIntProperty(XN_CAPABILITY_GAMMA);
  addGeneralIntProperty(XN_CAPABILITY_COLOR_TEMPERATURE);
  addGeneralIntProperty(XN_CAPABILITY_BACKLIGHT_COMPENSATION);
  addGeneralIntProperty(XN_CAPABILITY_GAIN);
  addGeneralIntProperty(XN_CAPABILITY_PAN);
  addGeneralIntProperty(XN_CAPABILITY_TILT);
  addGeneralIntProperty(XN_CAPABILITY_ROLL);
  addGeneralIntProperty(XN_CAPABILITY_ZOOM);
  addGeneralIntProperty(XN_CAPABILITY_EXPOSURE);
  addGeneralIntProperty(XN_CAPABILITY_IRIS);
  addGeneralIntProperty(XN_CAPABILITY_FOCUS);
  addGeneralIntProperty(XN_CAPABILITY_LOW_LIGHT_COMPENSATION);

  Configurable::registerCallback(
        utils::function(this,&MapGeneratorOptions::processPropertyChange));
}

// callback for changed configurable properties
void MapGeneratorOptions::processPropertyChange(
    const utils::Configurable::Property &prop)
{
  if(isCropping(prop.name)){
    setCropping(m_Generator, prop.name, prop.value);
  } else if (prop.name == XN_CAPABILITY_ANTI_FLICKER){
    antiFlickerCapabilitySet(m_Generator, prop.value);
  } else if (prop.name == XN_CAPABILITY_ALTERNATIVE_VIEW_POINT){
    alternativeViewPiontCapabilitySet(m_Generator, prop.value, m_ProductionNodeMap);
  } else if (prop.name == XN_CAPABILITY_MIRROR){
    m_Generator -> GetMirrorCap().SetMirror(parse<bool>(prop.value));
  } else if (setGeneralIntCapability(m_Generator, prop.name, prop.value)){
    // nothing to do setting is done in condition
  } else if (prop.name == "map output mode"){
    setCurrentMapOutputmode(m_Generator, prop.value);
  }
}

// adds a GeneralIntCapability as property
void MapGeneratorOptions::addGeneralIntProperty(const std::string name) {
  // only add if supported
  if(!m_Generator -> IsCapabilitySupported(name.c_str())){
    return;
  }
  GeneralIntCapability cap = getGeneralIntCapability(m_Generator, name);
  // when auto is supported add the auto-version too.
  if(isGeneralIntAutoSupported(cap)){
    std::ostringstream tmp;
    tmp << "Auto" << name;
    addProperty(tmp.str(),"flag", "", generalIntCapabilityValue(cap),
                0, "Automaticly set corresponding porperty.");
  }
  // get info
  XnInt32 min, max, step, def;
  XnBool sauto;
  cap.GetRange(min, max, step, def, sauto);
  std::ostringstream inf;
  inf << "[" << min << "," << max << "]:" << step;
  addProperty(name, "range", inf.str(), cap.Get(), 0, "");
}

//##############################################################################
//############################# DepthGeneratorOptions ##########################
//##############################################################################

// constructor
DepthGeneratorOptions::DepthGeneratorOptions(xn::DepthGenerator* generator)
  : MapGeneratorOptions(generator), m_DepthGenerator(generator)
{
  addProperty("max depth", "info", "", m_DepthGenerator -> GetDeviceMaxDepth(),
              0, "The maximum depth value of this grabber.");
  // field of view
  XnFieldOfView fov;
  m_DepthGenerator -> GetFieldOfView(fov);
  addProperty("field of view X", "info", "", fov.fHFOV, 0,
              "Horizontal field of view.");
  addProperty("field of view Y", "info", "", fov.fVFOV, 0,
              "Vertical field of view.");
}

//##############################################################################
//############################# ImageGeneratorOptions ##########################
//##############################################################################

// constructor
ImageGeneratorOptions::ImageGeneratorOptions(xn::ImageGenerator* generator)
  : MapGeneratorOptions(generator), m_ImageGenerator(generator)
{
  // construnct Pixel Format - format string
  std::ostringstream format;
  if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_RGB24)){
    format << "rgb24,";
  }
  if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_YUV422)){
    format << "yuv422,";
  }
  if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_8_BIT)){
    format << "grayscale8,";
  }
  if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_16_BIT)){
    format << "grayscale16,";
  }
  if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_MJPEG)){
    format << "mjpeg,";
  }

  std::string value = "";
  switch(m_ImageGenerator -> GetPixelFormat()){
    case XN_PIXEL_FORMAT_RGB24:
      value = "rgb24";
    case XN_PIXEL_FORMAT_YUV422:
      value = "yuv422";
    case XN_PIXEL_FORMAT_GRAYSCALE_8_BIT:
      value = "grayscale8";
    case XN_PIXEL_FORMAT_GRAYSCALE_16_BIT:
      value = "grayscale16";
    case XN_PIXEL_FORMAT_MJPEG:
      value = "mjpeg";
    default:
      DEBUG_LOG("Unknown Pixel Format " << m_ImageGenerator -> GetPixelFormat());
      value = "rgb24";
  }

  addProperty("Pixel Format", "menu", format.str(), value,
              0, "The pixel format of the aquired image.");

  Configurable::registerCallback(
        utils::function(this,&ImageGeneratorOptions::processPropertyChange));
}

// callback for changed configurable properties
void ImageGeneratorOptions::processPropertyChange(const utils::Configurable::Property &prop){
      if(prop.name == "Pixel Format"){
    if (prop.value == "rgb24"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_RGB24)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_RGB24);
      }
    }
    if (prop.value == "yuv422"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_YUV422)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_YUV422);
      }
    }
    if (prop.value == "grayscale8"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_8_BIT)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_GRAYSCALE_8_BIT);
      }
    }
    if (prop.value == "grayscale16"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_GRAYSCALE_16_BIT)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_GRAYSCALE_16_BIT);
      }
    }
    if (prop.value == "mjpeg"){
      if (m_ImageGenerator -> IsPixelFormatSupported(XN_PIXEL_FORMAT_MJPEG)){
        m_ImageGenerator -> SetPixelFormat(XN_PIXEL_FORMAT_MJPEG);
      }
    }
  }
}
