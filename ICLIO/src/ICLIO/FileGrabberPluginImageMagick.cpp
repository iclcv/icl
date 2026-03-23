/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileGrabberPluginImageMagick.cpp       **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLIO/FileGrabberPluginImageMagick.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/CCFunctions.h>
#include <ICLIO/FileWriterPluginImageMagick.h>

#ifdef ICL_HAVE_IMAGEMAGICK
// default value which would be set otherwise in Magic++.h as of May 2019
#ifndef MAGICKCORE_QUANTUM_DEPTH
#define MAGICKCORE_QUANTUM_DEPTH 16
#endif
#ifndef MAGICKCORE_HDRI_ENABLE
#define MAGICKCORE_HDRI_ENABLE 0
#endif
#define OMP_NUM_THREADS 1
#include <Magick++.h>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{


  #ifdef ICL_HAVE_IMAGEMAGICK
    struct FileGrabberPluginImageMagick::InternalData{
      std::vector<icl8u> buffer;


    };

    FileGrabberPluginImageMagick::FileGrabberPluginImageMagick():
      m_data(new FileGrabberPluginImageMagick::InternalData){
      // from FileWriterPluginImageMagick.h
    }

    FileGrabberPluginImageMagick::~FileGrabberPluginImageMagick(){
      delete m_data;
    }

    void FileGrabberPluginImageMagick::grab(File &file, ImgBase **dest){
      icl_initialize_image_magick_context();

      Magick::Image image;
      try{
        image.read(file.getName());
      }catch(Magick::Error &err){
        throw ICLException(std::string("ImageMagick::")+err.what());
      }

      Size size(static_cast<int>(image.columns()), static_cast<int>(image.rows()));
      const int dim = size.getDim();

      // Export pixel data as interleaved RGB unsigned chars
      m_data->buffer.resize(dim * 3);
      image.write(0, 0, size.width, size.height, "RGB", Magick::CharPixel, m_data->buffer.data());

      core::ensureCompatible(dest, depth8u, size, formatRGB);
      interleavedToPlanar(m_data->buffer.data(), (*dest)->asImg<icl8u>());
    }

  #else
    struct FileGrabberPluginImageMagick::InternalData{};

    FileGrabberPluginImageMagick::FileGrabberPluginImageMagick():
      m_data(0){}

    FileGrabberPluginImageMagick::~FileGrabberPluginImageMagick(){}

    void FileGrabberPluginImageMagick::grab(File &file, ImgBase **dest){
      ERROR_LOG("grabbing images of this format is not supported without libImageMagic++");
      throw InvalidFileException(file.getName());
    }
  #endif
  } // namespace io
}
