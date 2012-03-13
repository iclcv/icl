/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
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

#ifndef ICL_PYLON_COLOR_CONVERTER_H
#define ICL_PYLON_COLOR_CONVERTER_H

#include <pylon/PylonIncludes.h>

#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Mutex.h>

namespace icl {
  namespace pylon {

    /// This is the color-conversion-class for Pylon images \ingroup GIGE_G
    class PylonColorConverter {
      public:
      /// Constructor
      PylonColorConverter();
      /// Destructor
      ~PylonColorConverter();

      /// makes the conversion adapt to the passed properties on next convert.
      void resetConversion(int width, int height, Pylon::PixelType pixel_type,
                           int pixel_size_bits, long buffer_size);
      /// Converts pImageBuffer into an internal ImageBase.
      /**
      * @return a pointer to the converted image.
      */
      icl::ImgBase* convert(const void *pImageBuffer);

      private:
      /// image height
      int m_Height;
      /// image width
      int m_Width;
      /// input image pixel type
      Pylon::PixelType m_PixelType;
      /// input image pixel size (bit)
      int m_PixelSize;
      /// input image buffer size
      int m_BufferSize;
      /// A mutex lock for concurrency.
      Mutex m_Mutex;

      /// To this ImageBase the converted image is written.
      icl::ImgBase* m_Image;
      /// Buffer für color conversion
      icl8u* m_ImageBuff;
      /// Buffer für 16 bit mono copy conversion
      icl16s* m_ImageBuff16;
      /// Buffer for interlieved-to-planar conversion
      icl::Img8u* m_ImageRGBA;
      /// Vector for channels
      std::vector<icl8u*>* m_Channels;
      /// Vector for 16bit mono-channel
      std::vector<icl16s*>* m_Channels16;

      /// Pylon color format converter
      Pylon::CPixelFormatConverter* m_ColorConverter;
      Pylon::SImageFormat* m_InputFormat;
      Pylon::SOutputImageFormat* m_OutputFormat;

      /// Used to determine wether (and to what) to convert an image.
      enum convert_to {
        yes_rgba,
        no_rgb_packed,
        yes_mono8u,
        no_mono8u,
        no_mono16,
        undefined = -1
      };
      /// indicates whether the current colorformat needs conversion.
      convert_to m_Convert;
      /// indicates whether conversion should be reinitialized.
      bool m_Reset;

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

  } //namespace
} //namespace

#endif
