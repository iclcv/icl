/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PylonColorConverter.cpp                **
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

#include <ICLIO/PylonColorConverter.h>
#include <ICLCore/CCFunctions.h>
#include <ICLUtils/Time.h>

#define CONCAT(x) (std::ostringstream << x).str()

//#define SPEED_TEST

using namespace icl::core;
using namespace icl::utils;
using namespace icl::io::pylon;
using namespace Pylon;

#ifdef SPEED_TEST
  int count = 0;
  Time conv = Time();
  Time copyt = Time();
  std::string type_name = "not defined";
  int cccount = 0;
#endif

// Constructor
PylonColorConverter::PylonColorConverter() : m_Mutex() {
  Mutex::Locker l(m_Mutex);
  m_Converter = NULL;
}

// Destructor
PylonColorConverter::~PylonColorConverter(){
  Mutex::Locker l(m_Mutex);
  ICL_DELETE(m_Converter)
}

// makes the conversion adapt to the passed properties on next convert.
void PylonColorConverter::resetConversion(
    int width, int height, int pixel_size_bits, long buffer_size,
    Pylon::PixelType pixel_type, std::string pixel_type_name){

  //locking mutex
  Mutex::Locker l(m_Mutex);
  DEBUG_LOG2("w=" << width << " h=" << height << " t=" << pixel_type
                    << " sb=" << pixel_size_bits << " bs=" << buffer_size)
  #ifdef SPEED_TEST
    type_name = pixel_type_name;
  #endif

  ICL_DELETE(m_Converter)
  // reset color conversion
  switch(pixel_type){
    case PixelType_Mono8:
    case PixelType_Mono8signed:
      m_Converter = new Mono8uToMono8u(width, height);
      break;

    case PixelType_Mono10:
    case PixelType_Mono10packed:
    case PixelType_Mono12:
    case PixelType_Mono12packed:
      m_Converter = new MonoToMono8u(width, height, pixel_type,
                                       pixel_size_bits, buffer_size);
      break;
    case PixelType_Mono16:
      m_Converter = new Mono16sToMono16s(width, height);
      break;

    case PixelType_BayerGR8:
      m_Converter = new BayerToRgb8Icl(BayerConverter::nearestNeighbor,
                                       BayerConverter::bayerPattern_GRBG,
                                       Size(width, height));
      break;

    case PixelType_BayerRG8:
      m_Converter = new BayerToRgb8Icl(BayerConverter::nearestNeighbor,
                                       BayerConverter::bayerPattern_RGGB,
                                       Size(width, height));
      break;

    case PixelType_BayerGB8:
      m_Converter = new BayerToRgb8Icl(BayerConverter::nearestNeighbor,
                                       BayerConverter::bayerPattern_GBRG,
                                       Size(width, height));
      break;

    case PixelType_BayerBG8:
      m_Converter = new BayerToRgb8Icl(BayerConverter::nearestNeighbor,
                                       BayerConverter::bayerPattern_BGGR,
                                       Size(width, height));
      break;

    case PixelType_BayerGR10:
    case PixelType_BayerRG10:
    case PixelType_BayerGB10:
    case PixelType_BayerBG10:

    case PixelType_BayerGR12:
    case PixelType_BayerRG12:
    case PixelType_BayerGB12:
    case PixelType_BayerBG12:

    case PixelType_BayerGR12Packed:
    case PixelType_BayerRG12Packed:
    case PixelType_BayerGB12Packed:
    case PixelType_BayerBG12Packed:

    case PixelType_BayerGR16:
    case PixelType_BayerRG16:
    case PixelType_BayerGB16:
    case PixelType_BayerBG16:

    case PixelType_YUV411packed:
    case PixelType_YUV444packed:
      m_Converter = new PylonColorToRgb(width, height, pixel_type,
                                          pixel_size_bits, buffer_size);
      break;


    case PixelType_YUV422packed:
    #ifdef ICL_HAVE_IPP
      m_Converter = new Yuv422ToRgb8Icl(width, height);
    #else
      m_Converter = new PylonColorToRgb(width, height, pixel_type,
                                          pixel_size_bits, buffer_size);
    #endif
      break;

    case PixelType_YUV422_YUYV_Packed:
    #ifdef ICL_HAVE_IPP
      m_Converter = new Yuv422YUYVToRgb8Icl(width, height);
    #else
      m_Converter = new PylonColorToRgb(width, height, pixel_type,
                                          pixel_size_bits, buffer_size);
    #endif
      break;

    case PixelType_RGB8packed:
      m_Converter = new Rgb8uToRgb8u(width, height);
      break;
    case PixelType_BGR8packed:
    case PixelType_RGBA8packed:
    case PixelType_BGRA8packed:
    case PixelType_RGB10packed:
    case PixelType_BGR10packed:
    case PixelType_RGB12packed:
    case PixelType_BGR12packed:
    case PixelType_BGR10V1packed:
    case PixelType_BGR10V2packed:
    case PixelType_RGB8planar:
    case PixelType_RGB10planar:
    case PixelType_RGB12planar:
    case PixelType_RGB16planar:
    case PixelType_RGB12V1packed:
    case PixelType_Double:
    case PixelType_Undefined:
    default:
      std::ostringstream m;
      m << "color conversion for '" << pixel_type_name
      << "' (" << pixel_type << ") is not implemented.";
      m_ErrorMessage = m.str();
  }
}

// Converts pImageBuffer to correct type and writes it into m_Image
ImgBase* PylonColorConverter::convert(const void *pImageBuffer, ConvBuffers* b){
#ifdef SPEED_TEST
  Time t = Time::now();
#endif
  Mutex::Locker l(m_Mutex);
  if(m_Converter == NULL){
    DEBUG_LOG(m_ErrorMessage)
    return NULL;
  }
  // reinitialize buffer if needed
  if(b -> m_Reset){
    b -> free();
    m_Converter -> initBuffers(b);
    b -> m_Reset = false;
  }
  m_Converter -> convert(pImageBuffer, b);
#ifdef SPEED_TEST
  if(count >= 100){
      std::cout << "ColorConv: " << type_name << " " << copyt / count << std::endl;
      t = Time::now();
      count = 0;
      conv = Time();
      copyt = Time();
  } else {
     copyt += t.age();
     t = Time::now();
     count++;
  }
#endif
  return b -> m_Image;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* ---------------------------- Color converters ---------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

// Constructor initializes conversion
Mono8uToMono8u::Mono8uToMono8u(int width, int height)
: m_Width(width), m_Height(height)
{/* nothing to do*/}

// initializes buffers in b as needed for color conversion.
void Mono8uToMono8u::initBuffers(ConvBuffers* b){
    // settings for input format Mono8u, no conversion
    b -> m_ImageBuff = new icl8u[m_Width*m_Height];
    b -> m_Channels = new std::vector<icl8u*>();
    b -> m_Channels -> push_back(b -> m_ImageBuff);
    b -> m_Image = new Img8u(Size(m_Width, m_Height), core::formatGray, *b -> m_Channels);
}

// writes image from imgBuffer to b using appropriate conversion.
void Mono8uToMono8u::convert(const void *imgBuffer, ConvBuffers* b){
// no conversion just copy values from buffer to m_Image
  icl8u* imgBuffer8 = (icl8u*) imgBuffer;
  // m_ImageBuff is the channel of m_Image
  core::copy(imgBuffer8, imgBuffer8 + (m_Width*m_Height), b -> m_ImageBuff);
}

// Constructor initializes conversion
Mono16sToMono16s::Mono16sToMono16s(int width, int height)
: m_Width(width), m_Height(height)
{/* nothing to do*/}

// initializes buffers in b as needed for color conversion.
void Mono16sToMono16s::initBuffers(ConvBuffers* b){
    // settings for input format Mono16, no conversion
    b -> m_ImageBuff16 = new icl16s[m_Width*m_Height];
    b -> m_Channels16 = new std::vector<icl16s*>();
    b -> m_Channels16 -> push_back(b -> m_ImageBuff16);
    b -> m_Image = new Img16s(
                              Size(m_Width, m_Height),
                              core::formatGray,
                              *b -> m_Channels16
                              );
}

// writes image from imgBuffer to b using appropriate conversion.
void Mono16sToMono16s::convert(const void *imgBuffer, ConvBuffers* b){
  // no conversion just copy values from buffer to m_Image
  int16_t* imgBuffer16 = (int16_t*) imgBuffer;
  // m_ImageBuff16 is the channel of m_Image
  core::copy(imgBuffer16, imgBuffer16 + (m_Width*m_Height), b -> m_ImageBuff16);
}

// Constructor initializes conversion
MonoToMono8u::MonoToMono8u(int width, int height, Pylon::PixelType pixel_type,
                           int pixel_size_bits, long buffer_size)
  : m_Width(width), m_Height(height), m_PixelType(pixel_type),
    m_PixelSize(pixel_size_bits), m_BufferSize(buffer_size)
{
  // create pylon color format converter
  m_InputFormat = new Pylon::SImageFormat();
  m_InputFormat -> Width = m_Width;
  m_InputFormat -> Height = m_Height;
  m_InputFormat -> PixelFormat = m_PixelType;
  m_OutputFormat = new Pylon::SOutputImageFormat();
  m_OutputFormat -> LinePitch = m_Width;
  m_OutputFormat -> PixelFormat = Pylon::PixelType_Mono8;

  if(Pylon::IsPacked(m_PixelType)){
    // no line pitch for packed images
    m_InputFormat -> LinePitch = 0;
    // pylon truncate converter for packed images
    m_ColorConverter = new Pylon::CPixelFormatConverterTruncatePacked();
  } else {
    m_InputFormat -> LinePitch = (int) (m_Width * (m_PixelSize/8.0) + 0.5);
    // pylon truncate converter for non packed images
    m_ColorConverter = new Pylon::CPixelFormatConverterTruncate();
  }
  m_ColorConverter -> Init(*m_InputFormat);
}

// frees allocated ressources
MonoToMono8u::~MonoToMono8u(){
  ICL_DELETE(m_InputFormat)
  ICL_DELETE(m_OutputFormat)
  ICL_DELETE(m_ColorConverter)
}

// initializes buffers in b as needed for color conversion.
void MonoToMono8u::initBuffers(ConvBuffers* b){
  // this is used for the actual truncation
  b -> m_ImageBuff = new icl8u[m_Width*m_Height];

  // use m_ImageBuffer as channel of m_Image
  b -> m_Channels = new std::vector<icl8u*>();
  b -> m_Channels -> push_back(b -> m_ImageBuff);

  // m_Image uses m_ImageBuffer directly as channel
  b -> m_Image = new Img8u(Size(m_Width, m_Height),
                            core::formatGray, *b -> m_Channels, false);
}

// writes image from imgBuffer to b using appropriate conversion.
void MonoToMono8u::convert(const void *imgBuffer, ConvBuffers* b){
  // rgb grayscale conversion, call Pylon-convert function
  // this will change m_Image because its channel is m_ImageBuffer
  m_ColorConverter -> Convert(
              b -> m_ImageBuff,
              m_Width*m_Height*4,
              imgBuffer,
              m_BufferSize,
              *m_InputFormat,
              *m_OutputFormat
              );
}

// Constructor initializes conversion
Rgb8uToRgb8u::Rgb8uToRgb8u(int width, int height)
: m_Width(width), m_Height(height)
{/* nothing to do*/}

// initializes buffers in b as needed for color conversion.
void Rgb8uToRgb8u::initBuffers(ConvBuffers* b){
    b -> m_Image = new Img8u(Size(m_Width, m_Height), core::formatRGB);
}

// writes image from imgBuffer to b using appropriate conversion.
void Rgb8uToRgb8u::convert(const void *imgBuffer, ConvBuffers* b){
    //only need interleaved to planar conversion on original image.
    icl8u* imgBuffer8 = (icl8u*) imgBuffer;
    Img8u* img = dynamic_cast<Img8u*>(b -> m_Image);
    interleavedToPlanar(imgBuffer8, img);
}

// Constructor initializes conversion
PylonColorToRgb::PylonColorToRgb(int width, int height,
                            Pylon::PixelType pixel_type, int pixel_size_bits,
                            long buffer_size)
  : m_Width(width), m_Height(height), m_PixelType(pixel_type),
    m_PixelSize(pixel_size_bits), m_BufferSize(buffer_size)
{
  // create pylon color format converter
  m_InputFormat = new Pylon::SImageFormat();
  m_InputFormat -> Width = m_Width;
  m_InputFormat -> Height = m_Height;
  m_InputFormat -> PixelFormat = m_PixelType;
  m_OutputFormat = new Pylon::SOutputImageFormat();
  m_OutputFormat -> LinePitch = m_Width*4;
  m_OutputFormat -> PixelFormat = Pylon::PixelType_RGBA8packed;

  if(Pylon::IsPacked(m_PixelType)){
    // no line pitch for packed images
    m_InputFormat -> LinePitch = 0;
  } else {
    m_InputFormat -> LinePitch = (int) (m_Width * (m_PixelSize/8.0) + 0.5);
  }
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

// frees allocated ressources
PylonColorToRgb::~PylonColorToRgb(){
  ICL_DELETE(m_InputFormat)
  ICL_DELETE(m_OutputFormat)
  ICL_DELETE(m_ColorConverter)
}

// initializes buffers in b as needed for color conversion.
void PylonColorToRgb::initBuffers(ConvBuffers* b){
  // this is used for the color actual conversion
  b -> m_ImageBuff = new icl8u[m_Width*m_Height*4];
  // this will be used for the interlieved-to-planar conversion
  b -> m_ImageRGBA = new Img8u(Size(m_Width, m_Height), 4);

  // collect RGB channels from m_ImageRGBA for the m_Image
  b -> m_Channels = new std::vector<icl8u*>();
  b -> m_Channels -> push_back((icl8u*) (b -> m_ImageRGBA->getDataPtr(2)));
  b -> m_Channels -> push_back((icl8u*) (b -> m_ImageRGBA->getDataPtr(1)));
  b -> m_Channels -> push_back((icl8u*) (b -> m_ImageRGBA->getDataPtr(0)));

  // m_Image gets the RGB channels from m_ImageRGBA
  b -> m_Image = new Img8u(Size(m_Width, m_Height), core::formatRGB, *b -> m_Channels);
}

// writes image from imgBuffer to b using appropriate conversion.
void PylonColorToRgb::convert(const void *imgBuffer, ConvBuffers* b){
    // rgb color conversion, call Pylon-convert function
    m_ColorConverter -> Convert(
                b -> m_ImageBuff,
                m_Width*m_Height*4,
                imgBuffer,
                m_BufferSize,
                *m_InputFormat,
                *m_OutputFormat
                );

    // convert interleaved m_ImageBuff into m_ImageRGBA.
    // this will change m_Image because its channels are from m_ImageRGBA.
    interleavedToPlanar(b -> m_ImageBuff, b -> m_ImageRGBA);
}

// Constructor initializes conversion
BayerToRgb8Icl::BayerToRgb8Icl(BayerConverter::bayerConverterMethod method,
                               BayerConverter::bayerPattern pattern,
                               Size size)
  : m_Conv(pattern,method, size), m_Channels(1), m_Size(size)
{
  // nothing to do.
}

// frees allocated ressources
BayerToRgb8Icl::~BayerToRgb8Icl(){
  // nothing to do.
}

// initializes buffers in b as needed for color conversion.
void BayerToRgb8Icl::initBuffers(ConvBuffers* b){
  // just an rgb image
  b -> m_Image = new Img8u(m_Size, core::formatRGB);
}

// writes image from imgBuffer to b using appropriate conversion.
void BayerToRgb8Icl::convert(const void *imgBuffer, ConvBuffers* b){
  // set buffer as channels of source image
  m_Channels[0] = (icl8u*) imgBuffer;
  Img8u tmp = Img8u(m_Size, core::formatGray, m_Channels);
  m_Conv.apply(&tmp, &(b -> m_Image));
}

#ifdef ICL_HAVE_IPP
// Constructor initializes conversion
Yuv422ToRgb8Icl::Yuv422ToRgb8Icl(int width, int height)
  : m_Size(width, height)
{
  m_ConvBuffer = new icl8u[m_Size.getDim() * 3];
}

// frees allocated ressources
Yuv422ToRgb8Icl::~Yuv422ToRgb8Icl(){
  ICL_DELETE_ARRAY(m_ConvBuffer);
}

// initializes buffers in b as needed for color conversion.
void Yuv422ToRgb8Icl::initBuffers(ConvBuffers* b){
  // just an rgb image
  b -> m_Image = new Img8u(m_Size, core::formatRGB);
}

// writes image from imgBuffer to b using appropriate conversion.
void Yuv422ToRgb8Icl::convert(const void *imgBuffer, ConvBuffers* b){
  // IPP-colorconversion from yuv to rgb (interleaved)
  ippiCbYCr422ToRGB_8u_C2C3R((icl8u*) imgBuffer,
                           m_Size.width*2,
                           m_ConvBuffer,
                           m_Size.width*3,
                           m_Size
  );
  // conversion writes interleaved image into m_ConvBuffer.
  interleavedToPlanar(m_ConvBuffer, (Img8u*) b -> m_Image);
}

// Constructor initializes conversion
Yuv422YUYVToRgb8Icl::Yuv422YUYVToRgb8Icl(int width, int height)
  : m_Size(width, height)
{
  m_ConvBuffer = new icl8u[m_Size.getDim() * 3];
}

// frees allocated ressources
Yuv422YUYVToRgb8Icl::~Yuv422YUYVToRgb8Icl(){
  ICL_DELETE_ARRAY(m_ConvBuffer);
}

// initializes buffers in b as needed for color conversion.
void Yuv422YUYVToRgb8Icl::initBuffers(ConvBuffers* b){
  // just an rgb image
  b -> m_Image = new Img8u(m_Size, core::formatRGB);
}

// writes image from imgBuffer to b using appropriate conversion.
void Yuv422YUYVToRgb8Icl::convert(const void *imgBuffer, ConvBuffers* b){
  // IPP-colorconversion from yuv to rgb (interleaved)
  ippiYCbCr422ToRGB_8u_C2C3R((icl8u*) imgBuffer,
                           m_Size.width*2,
                           m_ConvBuffer,
                           m_Size.width*3,
                           m_Size
  );
  // conversion writes interleaved image into m_ConvBuffer.
  interleavedToPlanar(m_ConvBuffer, (Img8u*) b -> m_Image);
}
#endif
