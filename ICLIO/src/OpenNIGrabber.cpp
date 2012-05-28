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

Context g_context;
ScriptNode g_scriptNode;
DepthGenerator g_depth;
ImageGenerator g_image;
DepthMetaData g_depthMD;
ImageMetaData g_imageMD;

Img8u* rgbImage;
Img8u* depthImage;

int img = 0;

using namespace icl;

// Constructor of OpenNIGrabberImpl
OpenNIGrabberImpl::OpenNIGrabberImpl(const std::string args)
    : m_ImgMutex()
{
  DEBUG_LOG("args: " << args)
  icl::Mutex::Locker lock(m_ImgMutex);

  XnStatus rc;

  EnumerationErrors errors;
  g_context.Init();

  NodeInfoList l;
  g_context.EnumerateExistingNodes(l);

  g_depth.Create(g_context);
  g_image.Create(g_context);

  // Set it to VGA maps at 30 FPS
  XnMapOutputMode mapMode;
  mapMode.nXRes = 640;
  mapMode.nYRes = 480;
  mapMode.nFPS = 30;
  g_depth.SetMapOutputMode(mapMode);
  g_image.SetMapOutputMode(mapMode);

  g_context.StartGeneratingAll();

  g_depth.GetMetaData(g_depthMD);
  g_image.GetMetaData(g_imageMD);

  rgbImage = new Img8u(Size(g_imageMD.FullXRes(), g_imageMD.FullYRes()), formatRGB);
  depthImage = new Img8u(Size(g_depthMD.FullXRes(), g_depthMD.FullYRes()), formatGray);
}

OpenNIGrabberImpl::~OpenNIGrabberImpl(){
    DEBUG_LOG("");
}

const icl::ImgBase* OpenNIGrabberImpl::acquireImage(){
  XnStatus rc = XN_STATUS_OK;

  // Read a new frame
  rc = g_context.WaitAnyUpdateAll();
  if (rc != XN_STATUS_OK)
  {
    printf("Read failed: %s\n", xnGetStatusString(rc));
    return NULL;
  }

  g_depth.GetMetaData(g_depthMD);
  g_image.GetMetaData(g_imageMD);
  // draw RGB image
  const XnRGB24Pixel* rgbPixel = g_imageMD.RGB24Data();

  for (int y = 0; y < g_imageMD.YRes(); ++y){
    for (int x = 0; x < g_imageMD.XRes(); ++x, ++rgbPixel){
      (*rgbImage)(x, y, 0) = rgbPixel -> nRed;
      (*rgbImage)(x, y, 1) = rgbPixel -> nGreen;
      (*rgbImage)(x, y, 2) = rgbPixel -> nBlue;
    }
  }
  // draw DEPTH image
  const XnDepthPixel* pDepthRow = g_depthMD.Data();
  float depth = 0;
  for (int y = 0; y < g_depthMD.YRes(); ++y){
    for (int x = 0; x < g_depthMD.XRes(); ++x){
      (*depthImage)(x, y, 0) = (((float) *(pDepthRow + (g_depthMD.XRes() * y) + x)) / g_depthMD.ZRes() * 256);
    }
  }

  return rgbImage;
}

// default name
std::string OpenNIGrabberImpl::getName()
{
    return "default";
}

// setter function for video device properties
void OpenNIGrabberImpl::setProperty(
  const std::string &property, const std::string &value)
{
    DEBUG_LOG(property << " := " << value)
}

// returns a list of properties, that can be set using setProperty
std::vector<std::string> OpenNIGrabberImpl::getPropertyList(){
    DEBUG_LOG("")
  std::vector<std::string> ps;
  ps.push_back("size");
  ps.push_back("format");
  ps.push_back("DummyProp");
  return ps;
}

bool OpenNIGrabberImpl::supportsProperty(const std::string &property){
    DEBUG_LOG("");
  if (property.compare("size") == 0) return true;
  if (property.compare("format") == 0) return true;
  if (property.compare("DummyProp") == 0) return true;
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
  if(name.compare("DummyProp") == 0){
      return "range";
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
      if(name.compare("DummyProp") == 0){
      return "[10,100]:1.3";;
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
  if(name.compare("DummyProp") == 0){
    return "50";
  }
}

// Returns whether this property may be changed internally.
int OpenNIGrabberImpl::isVolatile(const std::string &propertyName){
  return true;
}
