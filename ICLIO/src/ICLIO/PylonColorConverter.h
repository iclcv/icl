// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#pragma once

#include <ICLIO/PylonIncludes.h>

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLIO/PylonUtils.h>
#include <ICLCore/BayerConverter.h>
#include <mutex>

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
      class ICLIO_API PylonColorConverter {
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
        std::recursive_mutex m_Mutex;
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
      class ICLIO_API Mono16sToMono16s : public ColorConverter{
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
      class ICLIO_API MonoToMono8u : public ColorConverter{
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
      class ICLIO_API Rgb8uToRgb8u : public ColorConverter{
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
      class ICLIO_API PylonColorToRgb : public ColorConverter{
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
      class ICLIO_API BayerToRgb8Icl : public ColorConverter{
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

      // TODO: add IPP-accelerated Yuv422ToRgb8 / Yuv422YUYVToRgb8 converters
      //       via ippiYUV422ToRGB_8u_C2C3R / ippiCbYCr422ToRGB_8u_C2C3R

    } //namespace
  } // namespace io
} //namespace
