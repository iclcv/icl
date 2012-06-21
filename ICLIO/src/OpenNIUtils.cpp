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
#include <ICLIO/OpenNIUtils.h>

using namespace xn;
using namespace icl;

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
        xn::Context* context, Generators type)
{
    switch(type){
      case DEPTH:
        return new OpenNIDepthGenerator(context);
      case RGB:
        return new OpenNIRgbGenerator(context);
      default:
        throw new ICLException("Generator not supported.");
    }
}

//##############################################################################
//############################# OpenNIDepthGenerator ###########################
//##############################################################################

// Creates a DepthGenerator from Context
OpenNIDepthGenerator::OpenNIDepthGenerator(Context* context)
    : m_Context(context), m_DepthGenerator(NULL), m_Image(NULL)
{
  DEBUG_LOG("Creating OpenNIDepthGenerator")
  XnStatus status;
  m_DepthGenerator = new DepthGenerator();
  status = m_DepthGenerator -> Create(*m_Context);
  if (status != XN_STATUS_OK){
    throw new ICLException("Generator init error " + *xnGetStatusString(status));
  }

  m_DepthGenerator -> GetMetaData(m_DepthMD);
  m_Image = new Img16s(Size(m_DepthMD.FullXRes(), m_DepthMD.FullYRes()),
                       formatGray);
  m_DepthGenerator -> StartGenerating();
}

// Destructor frees all resouurces
OpenNIDepthGenerator::~OpenNIDepthGenerator(){
    m_DepthGenerator -> StopGenerating();
    ICL_DELETE(m_DepthGenerator)
    ICL_DELETE(m_Image)
}

// grab function grabs an image
const ImgBase* OpenNIDepthGenerator::acquireImage(){
    XnStatus rc = XN_STATUS_OK;

    // Read a new frame
    rc = m_Context -> WaitAnyUpdateAll();
    if (rc != XN_STATUS_OK)
    {
      printf("Read failed: %s\n", xnGetStatusString(rc));
      return NULL;
    }

    m_DepthGenerator -> GetMetaData(m_DepthMD);

    // draw DEPTH image
    const XnDepthPixel* pDepthRow = m_DepthMD.Data();
    for (int y = 0; y < m_DepthMD.YRes(); ++y){
      for (int x = 0; x < m_DepthMD.XRes(); ++x){
        (*m_Image)(x, y, 0) = *(pDepthRow + (m_DepthMD.XRes() * y) + x);
      }
    }
    return m_Image;
}

// tells the type of the Generator
OpenNIImageGenerator::Generators OpenNIDepthGenerator::getType(){
  return OpenNIImageGenerator::DEPTH;
}

//##############################################################################
//############################# OpenNIRgbGenerator #############################
//##############################################################################s

// Creates a RgbGenerator from Context
OpenNIRgbGenerator::OpenNIRgbGenerator(Context* context)
    : m_Context(context), m_RgbGenerator(NULL), m_Image(NULL)
{
  DEBUG_LOG("Creating OpenNIRgbGenerator")
  XnStatus status;
  m_RgbGenerator = new ImageGenerator();
  status = m_RgbGenerator -> Create(*m_Context);
  if (status != XN_STATUS_OK){
    std::string error(xnGetStatusString(status));
    DEBUG_LOG("Generator init error '" << error << "'")
    throw new ICLException(error);
  }

  m_RgbGenerator -> GetMetaData(m_RgbMD);
  m_Image = new Img8u(Size(m_RgbMD.FullXRes(), m_RgbMD.FullYRes()),
                      formatRGB);
  m_RgbGenerator -> StartGenerating();
}

// Destructor frees all resouurces
OpenNIRgbGenerator::~OpenNIRgbGenerator(){
    m_RgbGenerator -> StopGenerating();
    ICL_DELETE(m_RgbGenerator)
    ICL_DELETE(m_Image)
}

// grab function grabs an image
const ImgBase* OpenNIRgbGenerator::acquireImage(){
    XnStatus rc = XN_STATUS_OK;

    // Read a new frame
    DEBUG_LOG2("wait context")
    rc = m_Context -> WaitAnyUpdateAll();
    DEBUG_LOG2("wait context_")
    if (rc != XN_STATUS_OK)
    {
      DEBUG_LOG2("Read failed: " << xnGetStatusString(rc))
      return NULL;
    }

    DEBUG_LOG2("getmeta")
    m_RgbGenerator -> GetMetaData(m_RgbMD);
    DEBUG_LOG2("getmeta_")

    // draw RGB image
    const XnRGB24Pixel* rgbPixel = m_RgbMD.RGB24Data();
    for (int y = 0; y < m_RgbMD.YRes(); ++y){
      for (int x = 0; x < m_RgbMD.XRes(); ++x, ++rgbPixel){
        (*m_Image)(x, y, 0) = rgbPixel -> nRed;
        (*m_Image)(x, y, 1) = rgbPixel -> nGreen;
        (*m_Image)(x, y, 2) = rgbPixel -> nBlue;
      }
    }
    return m_Image;
}

// tells the type of the Generator
OpenNIImageGenerator::Generators OpenNIRgbGenerator::getType(){
  return OpenNIImageGenerator::RGB;
}
