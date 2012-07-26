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

#include <sstream>

using namespace xn;
using namespace icl;

ImageGenerator* ig = NULL;
DepthGenerator* dg = NULL;

//##############################################################################
//############################# OpenNIAutoContext ##############################
//##############################################################################

int openni_inits = 0;
Mutex openni_lock;
Context openni_context;

// Initializes OpenNI context if not already done.
OpenNIAutoContext::OpenNIAutoContext(){
  initOpenNIContext();
}

// Releases OpenNI context when (calls to term) == (calls to init).
OpenNIAutoContext::~OpenNIAutoContext(){
  releaseOpenNIContext();
}

// Initializes the OpenNI context.
bool OpenNIAutoContext::initOpenNIContext(){
  icl::Mutex::Locker l(openni_lock);
  ICLASSERT(openni_inits >= 0)
      openni_inits++;
  if(openni_inits == 1){
    DEBUG_LOG("Initializing OpenNI context")
        openni_context.Init();
    return true;
  } else {
    return false;
  }
}

// releases the OpenNI context.
bool OpenNIAutoContext::releaseOpenNIContext(){
  icl::Mutex::Locker l(openni_lock);
  ICLASSERT(openni_inits > 0)
      openni_inits--;
  if(openni_inits == 0){
    DEBUG_LOG("Releasing OpenNI context")
        openni_context.Release();
    return true;
  } else {
    return false;
  }
}

// returns a pointer to the OpenNI context.
xn::Context* OpenNIAutoContext::getContextPtr(){
  return &openni_context;
}

//##############################################################################
//############################# OpenNIImageGenerator ###########################
//##############################################################################

//  Creates the corresponding Generator.
OpenNIImageGenerator* OpenNIImageGenerator::createGenerator(
    xn::Context* context, Generators type, int num)
{
  switch(type){
    case DEPTH:
      return new OpenNIDepthGenerator(context, num);
    case RGB:
      return new OpenNIRgbGenerator(context, num);
    default:
      throw new ICLException("Generator not supported.");
  }
}

//##############################################################################
//############################# OpenNIDepthGenerator ###########################
//##############################################################################

// Creates a DepthGenerator from Context
/*OpenNIDepthGenerator::OpenNIDepthGenerator(Context* context, int num)
  : m_Context(context), m_DepthGenerator(NULL)
{
  XnStatus status;
  NodeInfoList nodeInfoList;
  status = m_Context -> EnumerateProductionTrees(
             XN_NODE_TYPE_DEPTH , NULL , nodeInfoList);
  DEBUG_LOG("enumerate: "<< xnGetStatusString(status));

  if(nodeInfoList.IsEmpty()){
    throw ICLException("empty nodeinfo list");
  }
  ProductionNode pn;
  NodeInfoList::Iterator it = nodeInfoList.Begin();
  for(int i = 0; it != nodeInfoList.End(); it++, i++){
    if(i == num){
      NodeInfo ni = *it;
      status = ni.GetInstance(pn);
      DEBUG_LOG("getinstance: "<< xnGetStatusString(status));

      DEBUG_LOG("Creating OpenNIDepthGenerator");
      m_DepthGenerator = new DepthGenerator(pn);
      status = m_DepthGenerator -> Create(*m_Context);
      if (status != XN_STATUS_OK){
        std::ostringstream s;
        s << "Generator init error " << xnGetStatusString(status);
        throw ICLException(s.str());
      }
      m_DepthGenerator -> StartGenerating();
      return;
    }
  }
  std::ostringstream s;
  s << "Could not find enough depth generators to create nr. " << num;
  throw ICLException(s.str());
}*/

OpenNIDepthGenerator::OpenNIDepthGenerator(Context* context, int num)
  : m_Context(context), m_DepthGenerator(dg)
{
  XnStatus status;
  ICL_DELETE(m_DepthGenerator);
  dg = new DepthGenerator();
  m_DepthGenerator = dg;
  status = m_DepthGenerator -> Create(*m_Context);
  if (status != XN_STATUS_OK){
    std::ostringstream s;
    s << "Generator init error " << xnGetStatusString(status);
    throw ICLException(s.str());
  }
  m_DepthGenerator -> StartGenerating();
}

// Destructor frees all resouurces
OpenNIDepthGenerator::~OpenNIDepthGenerator(){
  m_DepthGenerator -> StopGenerating();
  DEBUG_LOG("deleting depth gen")
      ICL_DELETE(m_DepthGenerator)
}

// grab function grabs an image
bool OpenNIDepthGenerator::acquireImage(ImgBase* dest){
  //Time t = Time::now();
  XnStatus rc = XN_STATUS_OK;

  // Read a new frame
  rc = m_Context -> WaitAnyUpdateAll();
  if (rc != XN_STATUS_OK)
  {
    DEBUG_LOG("Read failed: " << xnGetStatusString(rc));
    return false;
  }

  m_DepthGenerator -> GetMetaData(m_DepthMD);
  Img16s* image = dest->as16s();
  image -> setSize(Size(m_DepthMD.XRes(), m_DepthMD.YRes()));
  // draw DEPTH image
  const XnDepthPixel* pDepthRow = m_DepthMD.Data();
  icl16s* data = image -> getData(0);
  for (unsigned int y = 0; y < m_DepthMD.YRes(); ++y){
    for (unsigned int x = 0; x < m_DepthMD.XRes(); ++x, ++pDepthRow, ++data){
      *data = *pDepthRow;
    }
  }
  //DEBUG_LOG("grabbed in " << t.age());
  return true;
}

// tells the type of the Generator
OpenNIImageGenerator::Generators OpenNIDepthGenerator::getType(){
  return OpenNIImageGenerator::DEPTH;
}

// returns underlying xn::MapGenerator instance
MapGenerator* OpenNIDepthGenerator::getMapGenerator(){
  return m_DepthGenerator;
}

Img16s* OpenNIDepthGenerator::initBuffer(){
  return new Img16s(Size(0,0), formatGray);
}

//##############################################################################
//############################# OpenNIRgbGenerator #############################
//##############################################################################

// Creates a RgbGenerator from Context
/*OpenNIRgbGenerator::OpenNIRgbGenerator(Context* context, int num)
  : m_Context(context), m_RgbGenerator(NULL)
{
  XnStatus status;
  NodeInfoList nodeInfoList;
  status = m_Context -> EnumerateProductionTrees(
             XN_NODE_TYPE_IMAGE , NULL , nodeInfoList);
  DEBUG_LOG("enumerate: "<< xnGetStatusString(status));

      if(nodeInfoList.IsEmpty()){
    throw ICLException("empty nodeinfo list");
  }
  ProductionNode pn;
  NodeInfoList::Iterator it = nodeInfoList.Begin();
  for(int i = 0; it != nodeInfoList.End(); it++, i++){
    if(i == num){
      NodeInfo ni = *it;
      status = ni.GetInstance(pn);
      DEBUG_LOG("getinstance: "<< xnGetStatusString(status));


      DEBUG_LOG("Creating OpenNIRgbGenerator");
      m_RgbGenerator = new ImageGenerator(pn);
      status = m_RgbGenerator -> Create(*m_Context);

      if (status != XN_STATUS_OK){
        std::string error(xnGetStatusString(status));
        DEBUG_LOG("Generator init error '" << error << "'");
            throw new ICLException(error);
      }
      m_RgbGenerator -> StartGenerating();
      DEBUG_LOG("done creating OpenNIRgbGenerator");
      return;
    }
  }
  std::ostringstream s;
  s << "Could not find enough depth generators to create nr. " << num;
  throw ICLException(s.str());
}*/

OpenNIRgbGenerator::OpenNIRgbGenerator(Context* context, int num)
  : m_Context(context), m_RgbGenerator(ig)
{
  XnStatus status;

  ICL_DELETE(ig);
  ICL_DELETE(dg);

  dg = new DepthGenerator();
  status = dg -> Create(*m_Context);

  m_RgbGenerator = new ImageGenerator();
  ig = m_RgbGenerator;
  status = m_RgbGenerator -> Create(*m_Context);

  if (status != XN_STATUS_OK){
    std::string error(xnGetStatusString(status));
    DEBUG_LOG("Generator init error '" << error << "'");
    throw new ICLException(error);
  }
  m_RgbGenerator -> StartGenerating();
  DEBUG_LOG("done creating OpenNIRgbGenerator");
}

// Destructor frees all resouurces
OpenNIRgbGenerator::~OpenNIRgbGenerator(){
  m_RgbGenerator -> StopGenerating();
  ICL_DELETE(m_RgbGenerator)
}

// grab function grabs an image
bool OpenNIRgbGenerator::acquireImage(ImgBase* dest){
  //Time t = Time::now();
  XnStatus rc = XN_STATUS_OK;

  // Read a new frame
  //DEBUG_LOG("wait context")
  //Time t3 = Time::now();
  rc = m_Context -> WaitAnyUpdateAll();
  //DEBUG_LOG("wait context: " << t3.age());
  if (rc != XN_STATUS_OK)
  {
    //DEBUG_LOG("Read failed: " << xnGetStatusString(rc))
    return false;
  }

  //DEBUG_LOG("getmeta")
  m_RgbGenerator -> GetMetaData(m_RgbMD);
  //DEBUG_LOG("getmeta_")
  Img8u* image = dest->as8u();
  image -> setSize(Size(m_RgbMD.XRes(), m_RgbMD.YRes()));


  // draw RGB image
  //Time t2 = Time::now();
  unsigned char* rChannel = image -> getData(0);
  unsigned char* gChannel = image -> getData(1);
  unsigned char* bChannel = image -> getData(2);
  const XnRGB24Pixel* rgbPixel = m_RgbMD.RGB24Data();
  for (unsigned int y = 0; y < m_RgbMD.YRes(); ++y){
    for (unsigned int x = 0; x < m_RgbMD.XRes(); ++x, ++rgbPixel, ++rChannel,
         ++gChannel, ++bChannel)
    {
      //(*image)(x, y, 0) = rgbPixel -> nRed;
      //(*image)(x, y, 1) = rgbPixel -> nGreen;
      //(*image)(x, y, 2) = rgbPixel -> nBlue;
      *rChannel = rgbPixel -> nRed;
      *gChannel = rgbPixel -> nGreen;
      *bChannel = rgbPixel -> nBlue;
    }
  }
  //DEBUG_LOG("copy in " << t2.age());
  //DEBUG_LOG("grabbed in " << t.age());
  return true;
}

// tells the type of the Generator
OpenNIImageGenerator::Generators OpenNIRgbGenerator::getType(){
  return OpenNIImageGenerator::RGB;
}

// returns underlying xn::MapGenerator instance
MapGenerator* OpenNIRgbGenerator::getMapGenerator(){
  return m_RgbGenerator;
}

Img8u* OpenNIRgbGenerator::initBuffer(){
  return new Img8u(Size(0,0), formatRGB);
}

//##############################################################################
//############################# MapGeneratorOptions ############################
//##############################################################################

std::string getMapOutputModeInfo(MapGenerator* gen){
  XnUInt32 count = gen -> GetSupportedMapOutputModesCount();
  XnMapOutputMode* modes = new XnMapOutputMode[count];
  gen -> GetSupportedMapOutputModes(modes, count);
  std::ostringstream ret;
  ret << "{";
  for(unsigned int i = 0; i < count; ++i){
    ret << modes[i].nXRes << "x";
    ret << modes[i].nYRes << "@";
    ret << modes[i].nFPS << "fps";
    if(i+1 < count){
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
    std::ostringstream tmp;
    XnMapOutputMode mode;
    gen -> GetMapOutputMode(mode);
    if (property == "Cropping offset X"){
      tmp << "[0," << mode.nXRes << "]:1";
    } else if (property == "Cropping offset Y"){
      tmp << "[0," << mode.nYRes << "]:1";
    } else if (property == "Cropping size X"){
      tmp << "[0," << mode.nXRes << "]:1";
    } else if (property == "Cropping size Y"){
      tmp << "[0," << mode.nYRes << "]:1";
    }
    return tmp.str();
  }
}

MapGeneratorOptions::MapGeneratorOptions(xn::MapGenerator* generator)
  : m_Generator(generator)
{
  m_Capabilities.push_back("map output mode");
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_CROPPING)){
    addCroppingCapability(m_Capabilities);
  }
  if(m_Generator -> IsCapabilitySupported(XN_CAPABILITY_ANTI_FLICKER)){
    m_Capabilities.push_back(XN_CAPABILITY_ANTI_FLICKER);
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

// interface for the setter function for video device properties
void MapGeneratorOptions::setProperty(
    const std::string &property, const std::string &value){
  if(isCropping(property)){
    setCropping(m_Generator, property, value);
  } else if (property == XN_CAPABILITY_ANTI_FLICKER){
    antiFlickerCapabilitySet(m_Generator, value);
  } else if (setGeneralIntCapability(m_Generator, property, value)){
    // nothing to do setting is done in condition
  } else if (property == "map output mode"){
    setCurrentMapOutputmode(m_Generator, value);
  }
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

// get type of property
std::string MapGeneratorOptions::getType(const std::string &name){
  if(isCropping(name)){
    return getCroppingType(name);
  } else if (name == XN_CAPABILITY_ANTI_FLICKER){
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
  return true;
}
