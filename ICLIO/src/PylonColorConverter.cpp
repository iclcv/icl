/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/PylonColorConverter.cpp                      **
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

#include <ICLIO/PylonColorConverter.h>
#include <ICLCC/CCFunctions.h>

using namespace icl;
using namespace icl::pylon;

// returns whether the passed PixelType needs to be handled as a color type
bool isColor(Pylon::PixelType pixelType){
  if(Pylon::IsBayer(pixelType)){
    return true;
  }
  if(pixelType == Pylon::PixelType_YUV422packed){
    return true;
  }
  if(pixelType == Pylon::PixelType_YUV422_YUYV_Packed){
    return true;
  }
  return false;
}

// Constructor
PylonColorConverter::PylonColorConverter()
  : m_Height(0), m_Width(0), m_PixelType(Pylon::PixelType_Undefined),
  m_Mutex(), m_Convert(undefined), m_Reset(true)

{
  Mutex::Locker l(m_Mutex);
  m_Image = NULL;
  m_ImageBuff = NULL;
  m_ImageBuff16 = NULL;
  m_ImageRGBA = NULL;
  m_Channels = NULL;
  m_Channels16 = NULL;
  m_ColorConverter = NULL;
  m_InputFormat = NULL;
  m_OutputFormat = NULL;
}

// Destructor
PylonColorConverter::~PylonColorConverter(){
  Mutex::Locker l(m_Mutex);
  freeAll();
}

// used to free all allocated resources
void PylonColorConverter::freeAll(){
  ICL_DELETE(m_Image)
  ICL_DELETE_ARRAY(m_ImageBuff)
  ICL_DELETE_ARRAY(m_ImageBuff16)
  ICL_DELETE(m_ImageRGBA)
  ICL_DELETE(m_Channels)
  ICL_DELETE(m_Channels16)
  ICL_DELETE(m_ColorConverter)
  ICL_DELETE(m_InputFormat)
  ICL_DELETE(m_OutputFormat)
}

// makes the conversion adapt to the passed properties on next convert.
void PylonColorConverter::resetConversion(int width, int height,
                                          Pylon::PixelType pixel_type,
                                          int pixel_size_bits,
                                          long buffer_size){
  //locking mutex
  Mutex::Locker l(m_Mutex);
  FUNCTION_LOG("w=" << width << " h=" << height << " t=" << pixel_type
                    << " sb=" << pixel_size_bits << " bs=" << buffer_size)
  // set image information
  m_Height = height;
  m_Width = width;
  m_PixelType = pixel_type;
  m_PixelSize = pixel_size_bits;
  m_BufferSize = buffer_size;
  m_Reset = true;
}

void PylonColorConverter::initConversion(){
  // free all resources
  freeAll();
  // create pylon color format converter
  m_InputFormat = new Pylon::SImageFormat();
  m_InputFormat -> Width = m_Width;
  m_InputFormat -> Height = m_Height;
  m_InputFormat -> PixelFormat = m_PixelType;
  m_OutputFormat = new Pylon::SOutputImageFormat();
  // Line pitch default settings for input image
  if(Pylon::IsPacked(m_PixelType)){
      DEBUG_LOG("isPacked")
    m_InputFormat -> LinePitch = 0;
  } else {
    m_InputFormat -> LinePitch =
      (int) (m_Width * (m_PixelSize/8.0) + 0.5);
  }
  if (isColor(m_PixelType)){
      DEBUG_LOG("isColor")
    // make settings for rgb-conversion
    setConversionRGB();
  } else if (Pylon::IsMono(m_PixelType)){
      DEBUG_LOG("isMono")
    // make settings for mono-conversion
    setConversionMono();
  } else if (m_PixelType == 35127316){
    // make settings for RGBPacked-conversion
    DEBUG_LOG("isRGBPacked")
    setConversionRGBPacked();
  } else {
    DEBUG_LOG("\n\nATTENTION: ColorFormat " << m_PixelType
              << " is currently not supported.\n"
              << "Stopping Application to prevend from undefined behavior.\n")
    exit(-1);
  }
}

// makes all settings for the conv. of color images.
void PylonColorConverter::setConversionRGB(){
  // set conversion to rgba
  m_Convert = yes_rgba;

  // output image settings
  m_OutputFormat -> LinePitch = m_Width*4;
  m_OutputFormat -> PixelFormat = Pylon::PixelType_RGBA8packed;

  // this is used for the color actual conversion
  m_ImageBuff = new icl8u[m_Width*m_Height*4];
  // this will be used for the interlieved-to-planar conversion
  m_ImageRGBA = new Img8u(Size(m_Width, m_Height), 4);

  // collect RGB channels from m_ImageRGBA for the m_Image
  m_Channels = new std::vector<icl8u*>();
  m_Channels -> push_back((icl8u*) (m_ImageRGBA->getDataPtr(2)));
  m_Channels -> push_back((icl8u*) (m_ImageRGBA->getDataPtr(1)));
  m_Channels -> push_back((icl8u*) (m_ImageRGBA->getDataPtr(0)));

  // m_Image gets the RGB channels from m_ImageRGBA
  m_Image = new Img8u(Size(m_Width, m_Height), icl::formatRGB, *m_Channels);

  // create/init correct converter
  if (Pylon::IsBayer(m_PixelType)){
    // Bayer color Converter
    m_ColorConverter = new Pylon::CPixelFormatConverterBayer();
  } else if (m_PixelType == Pylon::PixelType_YUV422packed){
    // Yuv422-UYVY color Converter
    m_ColorConverter = new Pylon::CPixelFormatConverterYUV422;
  } else {
    // Yuv422-YUYV color Converter
    m_ColorConverter = new Pylon::CPixelFormatConverterYUV422YUYV;
  }
  m_ColorConverter->Init(*m_InputFormat);
}

// makes all settings for the conv. PackedRGB images.
void PylonColorConverter::setConversionRGBPacked(){
  // this format only needs packed to planar conversion.
  m_Convert = no_rgb_packed;
  m_Image = new Img8u(Size(m_Width, m_Height), icl::formatRGB);
}

// makes all settings for the conv. of grayscale images.
void PylonColorConverter::setConversionMono(){
  if (m_PixelType == Pylon::PixelType_Mono8
      || m_PixelType == Pylon::PixelType_Mono8signed){
    // settings for input format Mono8, no conversion
    m_Convert = no_mono8u;
    m_ImageBuff = new icl8u[m_Width*m_Height];
    m_Channels = new std::vector<icl8u*>();
    m_Channels -> push_back(m_ImageBuff);
    m_Image = new Img8u(Size(m_Width, m_Height), icl::formatGray, *m_Channels);
  } else if (m_PixelType == Pylon::PixelType_Mono16){
    // settings for input format Mono16, no conversion
    m_Convert = no_mono16;
    // double sized buffer because of 16bit
    m_ImageBuff16 = new icl16s[m_Width*m_Height];
    m_Channels16 = new std::vector<icl16s*>();
    m_Channels16 -> push_back(m_ImageBuff16);

    m_Image = new Img16s(Size(m_Width, m_Height), icl::formatGray, *m_Channels16);
  } else {
    // Settings for other Mono-input formats (convert using truncation)
    m_Convert = yes_mono8u;

    // output image settings
    m_OutputFormat -> LinePitch = m_Width;
    m_OutputFormat -> PixelFormat = Pylon::PixelType_Mono8;

    // this is used for the actual truncation
    m_ImageBuff = new icl8u[m_Width*m_Height];

    // use m_ImageBuffer as channel of m_Image
    m_Channels = new std::vector<icl8u*>();
    m_Channels -> push_back(m_ImageBuff);

    // m_Image uses m_ImageBuffer directly as channel
    m_Image = new Img8u(Size(m_Width, m_Height),
                            icl::formatGray, *m_Channels, false);

    // create/init correct converter
    if(Pylon::IsPacked(m_PixelType)){
      m_ColorConverter = new Pylon::CPixelFormatConverterTruncatePacked();
    } else {
      m_ColorConverter = new Pylon::CPixelFormatConverterTruncate();
    }
    m_ColorConverter -> Init(*m_InputFormat);
  }
}

// Converts pImageBuffer to correct type and writes it into m_Image
icl::ImgBase* PylonColorConverter::convert(const void *pImageBuffer){
  Mutex::Locker l(m_Mutex);
  // reinitialize conversion if needed
  if(m_Reset) {
    initConversion();
    m_Reset = false;
  }
  // check which conversion to use
  if (m_Convert == yes_rgba){
    // rgb color conversion, call Pylon-convert function
    m_ColorConverter -> Convert(
                m_ImageBuff,
                m_Width*m_Height*4,
                pImageBuffer,
                m_BufferSize,
                *m_InputFormat,
                *m_OutputFormat
                );

    // convert interleaved m_ImageBuff into m_ImageRGBA.
    // this will change m_Image because its channels are from m_ImageRGBA.
    interleavedToPlanar(m_ImageBuff, m_ImageRGBA);

  } else if (m_Convert == no_rgb_packed){
    //only need interleaved toi planar conversion on original image.
    icl8u* pImageBuffer8 = (icl8u*) pImageBuffer;
    Img8u* img = dynamic_cast<Img8u*>(m_Image);
    interleavedToPlanar(pImageBuffer8, img);

  } else if (m_Convert == yes_mono8u){
    // rgb grayscale conversion, call Pylon-convert function
    // this will change m_Image because its channel is m_ImageBuffer
    m_ColorConverter -> Convert(
                m_ImageBuff,
                m_Width*m_Height*4,
                pImageBuffer,
                m_BufferSize,
                *m_InputFormat,
                *m_OutputFormat
                );

  } else if (m_Convert == no_mono8u){
  // no conversion just copy values from buffer to m_Image
  icl8u* pImageBuffer8 = (icl8u*) pImageBuffer;
  // m_ImageBuff is the channel of m_Image
  icl::copy(pImageBuffer8, pImageBuffer8 + (m_Width*m_Height), m_ImageBuff);
  } else if (m_Convert == no_mono16){
  // no conversion just copy values from buffer to m_Image
  int16_t* pImageBuffer16 = (int16_t*) pImageBuffer;
  // m_ImageBuff16 is the channel of m_Image
  icl::copy(pImageBuffer16, pImageBuffer16 + (m_Width*m_Height), m_ImageBuff16);
  } else {
    std::stringstream ex("Conversion to color format convert_to=");
    ex << m_Convert << " not defined";
    throw new ICLException(ex.str());
  }
  return m_Image;
}
