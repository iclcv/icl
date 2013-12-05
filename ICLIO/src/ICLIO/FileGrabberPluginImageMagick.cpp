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
#include <ICLCore/CCFunctions.h>
#include <ICLIO/FileWriterPluginImageMagick.h>

#ifdef HAVE_IMAGEMAGICK 
#define OMP_NUM_THREADS 1 
#ifndef ICL_SYSTEM_LINUX
#include <Magick++.h>
#else

//#ifdef ICL_USE_GRAPHICSMAGICK
//#include <GraphicsMagick/Magick++.h>
//#else
#include <ImageMagick/Magick++.h>
//#endif

#endif
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
  
  
  #ifdef HAVE_IMAGEMAGICK  
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
  
      Magick::Image *image = 0;    
      try{
        image = new  Magick::Image(file.getName());
      }catch(Magick::Error &err){
        throw ICLException(std::string("ImageMagick::")+err.what());
      }
      image->modifyImage();
      image->type(Magick::TrueColorType);
      
      Size size(image->columns(),image->rows());
      unsigned int minsize=size.getDim()*3;
      if(m_data->buffer.size()<minsize){
        m_data->buffer.resize(minsize);
      }
      
      /**
          Here we faced an error in the Magick++ library when reading png images ?
          image->getConstPixels(0,0,size.width,size.height);
          image->writePixels(Magick::RGBQuantum,m_data->buffer.data());
          
          icl::ensureCompatible(dest,depth8u,size,formatRGB);
          icl::interleavedToPlanar(m_data->buffer.data(),(*dest)->asImg<icl8u>());
      **/
  
      const Magick::PixelPacket *pix = image->getConstPixels(0,0,size.width,size.height);
      core::ensureCompatible(dest,depth8u,size,formatRGB);
  
      icl8u *r = (*dest)->asImg<icl8u>()->begin(0);
      icl8u *g = (*dest)->asImg<icl8u>()->begin(1);
      icl8u *b = (*dest)->asImg<icl8u>()->begin(2);
      const int dim = size.getDim();
      if(sizeof(Magick::PixelPacket) == 4){
        for(int i=0;i<dim;++i){
          const Magick::PixelPacket &p = pix[i];
          r[i] = clipped_cast<Magick::Quantum,icl8u>(p.red);
          g[i] = clipped_cast<Magick::Quantum,icl8u>(p.green);
          b[i] = clipped_cast<Magick::Quantum,icl8u>(p.blue);
        }
      }else{
        for(int i=0;i<dim;++i){
          const Magick::PixelPacket &p = pix[i];
          r[i] = clipped_cast<Magick::Quantum,icl8u>(p.red>>8);
          g[i] = clipped_cast<Magick::Quantum,icl8u>(p.green>>8);
          b[i] = clipped_cast<Magick::Quantum,icl8u>(p.blue>>8);
        }
      }
      ICL_DELETE(image);
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


