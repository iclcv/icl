/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/PylonColorConverter.h                    **
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

#pragma once

#include <pylon/PylonIncludes.h>

#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Mutex.h>
#include <ICLIO/PylonUtils.h>
#include <ICLCore/BayerConverter.h>

namespace icl {
  namespace io{
    namespace pylon {
  
      /// Pure virtual interface for color converters  \ingroup GIGE_G
      class ColorConverter{
        public:
          /// Virtual destructor
          virtual ~ColorConverter() {}
          /// initializes buffers in b as needed for color conversion.
          virtual void initBuffers(ConvBuffers* b) = 0;
          /// writes image from imgBuffer to b using appropriate conversion.
          virtual void convert(const void *imgBuffer, ConvBuffers* b) = 0;
      };
  
      /// This is the color-conversion-class for Pylon images \ingroup GIGE_G
      class PylonColorConverter {
        public:
        /// Constructor
        PylonColorConverter();
        /// Destructor
        ~PylonColorConverter();
  
        /// makes the conversion adapt to the passed properties on next convert.
        void resetConversion(int width, int height, int pixel_size_bits,
                             long buffer_size, Pylon::PixelType pixel_type,
                             std::string pixel_type_name);
        /// Converts pImageBuffer into an internal ImageBase.
        /**
        * @return a pointer to the converted image.
        */
        icl::core::ImgBase* convert(const void *imgBuffer, ConvBuffers* b);
  
        private:
        /// A mutex lock for concurrency.
        utils::Mutex m_Mutex;
        /// A pointer to the currently used converter.
        ColorConverter* m_Converter;
        /// field for error message
        /**
           when no appropriate color conversion could be found, an error message
           is saved here and printed on every call to convert.
        **/
        std::string m_ErrorMessage;
  
        /// used to free all allocated resources
        void freeAll();
        /// frees allocated memory and reinitializes thr color conversion.
        void initConversion();
        /// used to get all needed information about the image to convert
        void getInfo();
        /// makes all settings for the conv. of color images.
        void setConversionRGB();
        /// makes all settings for the conv. PackedRGB images.
        void setConversionRGBPacked();
        /// makes all settings for the conv. of grayscale images.
        void setConversionMono();
      };
  
      /// This ColorConverter is used for pylon-mono8u to icl-mono8u conversion.
      class Mono8uToMono8u : public ColorConverter{
        public:
          /// Constructor initializes conversion
          Mono8uToMono8u(int width, int height);
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          int m_Width;
          int m_Height;
      };
  
      /// This ColorConverter is used for pylon-mono16s to icl-mono16s conversion.
      class Mono16sToMono16s : public ColorConverter{
        public:
          /// Constructor initializes conversion
          Mono16sToMono16s(int width, int height);
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          int m_Width;
          int m_Height;
      };
  
      /// This ColorConverter is used for other pylon-mono to icl-mono8u conversion.
      class MonoToMono8u : public ColorConverter{
        public:
          /// Constructor initializes conversion
          MonoToMono8u(int width, int height, Pylon::PixelType pixel_type,
                       int pixel_size_bits, long buffer_size);
          /// frees allocated ressources
          ~MonoToMono8u();
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          int m_Width;
          int m_Height;
          Pylon::PixelType m_PixelType;
          int m_PixelSize;
          long m_BufferSize;
          /// Pylon color core::format converter
          Pylon::CPixelFormatConverter* m_ColorConverter;
          /// Pylon color core::format converter input format
          Pylon::SImageFormat* m_InputFormat;
          /// Pylon color core::format converter output format
          Pylon::SOutputImageFormat* m_OutputFormat;
      };
  
      /// This ColorConverter is used for pylon-rgb to icl-rgb conversion.
      /** This color type is only used by pylon camera-emulation **/
      class Rgb8uToRgb8u : public ColorConverter{
        public:
          /// Constructor initializes conversion
          Rgb8uToRgb8u(int width, int height);
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          int m_Width;
          int m_Height;
      };
  
      /// This ColorConverter is used for pylon-bayer/yuv to icl-rgb conversion.
      class PylonColorToRgb : public ColorConverter{
        public:
          /// Constructor initializes conversion
          PylonColorToRgb(int width, int height, Pylon::PixelType pixel_type,
                       int pixel_size_bits, long buffer_size);
          /// frees allocated ressources
          ~PylonColorToRgb();
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          int m_Width;
          int m_Height;
          Pylon::PixelType m_PixelType;
          int m_PixelSize;
          long m_BufferSize;
          /// Pylon color core::format converter
          Pylon::CPixelFormatConverter* m_ColorConverter;
          /// Pylon color core::format converter input format
          Pylon::SImageFormat* m_InputFormat;
          /// Pylon color core::format converter output format
          Pylon::SOutputImageFormat* m_OutputFormat;
      };
  
      /// This ColorConverter uses the icl Bayer to Rgb conversion.
      class BayerToRgb8Icl : public ColorConverter{
        public:
          /// Constructor initializes conversion
        BayerToRgb8Icl(core::BayerConverter::bayerConverterMethod method,
                       core::BayerConverter::bayerPattern pattern,
                       utils::Size s);
          /// frees allocated ressources
          ~BayerToRgb8Icl();
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          core::BayerConverter m_Conv;
          std::vector<icl8u*> m_Channels;
          utils::Size m_Size;
      };
  
  #ifdef HAVE_IPP
      /// This ColorConverter uses the icl Yuv422 to Rgb conversion.
      class Yuv422ToRgb8Icl : public ColorConverter{
        public:
          /// Constructor initializes conversion
          Yuv422ToRgb8Icl(int width, int height);
          /// frees allocated ressources
          ~Yuv422ToRgb8Icl();
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          utils::Size m_Size;
          icl8u* m_ConvBuffer;
      };
  
      /// This ColorConverter uses the icl Yuv422YUYV to Rgb conversion.
      class Yuv422YUYVToRgb8Icl : public ColorConverter{
        public:
          /// Constructor initializes conversion
          Yuv422YUYVToRgb8Icl(int width, int height);
          /// frees allocated ressources
          ~Yuv422YUYVToRgb8Icl();
          /// initializes buffers in b as needed for color conversion.
          void initBuffers(ConvBuffers* b);
          /// writes image from imgBuffer to b using appropriate conversion.
          void convert(const void *imgBuffer, ConvBuffers* b);
        private:
          utils::Size m_Size;
          icl8u* m_ConvBuffer;
      };
  #endif
  
    } //namespace
  } // namespace io
} //namespace

