// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/FileWriterPluginImageMagick.h>
#include <ICLCore/CoreFunctions.h>

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

#include <ICLCore/CCFunctions.h>
#include <ICLCore/Converter.h>
#include <ICLUtils/Exception.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  #ifdef ICL_HAVE_IMAGEMAGICK



    class FileWriterPluginImageMagick::InternalData{
    public:
      InternalData(){
        buffer = 0;
      }
      ~InternalData(){
        ICL_DELETE(buffer);
      }
      ImgBase *buffer;
      Converter converter;
      std::vector<icl8u> interleavedBuffer;

    };
    FileWriterPluginImageMagick::FileWriterPluginImageMagick():m_data(new FileWriterPluginImageMagick::InternalData){
    }
    FileWriterPluginImageMagick::~FileWriterPluginImageMagick(){
      ICL_DELETE(m_data);
    }

  #ifdef ICL_HAVE_IMAGEMAGICK
    Magick::StorageType get_magick_storage_type(depth d){
      switch(d){
        case depth8u: return Magick::CharPixel;
        case depth16s: return Magick::ShortPixel;
        case depth32s: return Magick::LongPixel;
        case depth32f: return Magick::FloatPixel;
        case depth64f: return Magick::DoublePixel;
        default:
          ERROR_LOG("unknown depth");
          return Magick::CharPixel;
      }
    }
  #endif
    void FileWriterPluginImageMagick::write(File &file, const ImgBase *image){
      icl_initialize_image_magick_context();

      switch(image->getChannels()){
        case 1:{
          const ImgBase *useImage = image;
          if(image->getDepth() == depth32f || image->getDepth() == depth64f){
            core::ensureCompatible(&m_data->buffer,image->getDepth(),image->getParams());
            image->deepCopy(&m_data->buffer);
            m_data->buffer->normalizeAllChannels(utils::Range64f(0,1));
            useImage = m_data->buffer;
          }
          try{
            // here we're just using the undocumented map parameter "I"
            Magick::Image mi(useImage->getWidth(),useImage->getHeight(),"I",
                             get_magick_storage_type(useImage->getDepth()),
                             useImage->getDataPtr(0));
            mi.write(file.getName());
          }catch(Magick::Error &err){
            throw ICLException(std::string("ImageMagick-FileWriter::")+err.what());
          }
          break;
        }
        case 3:{
          const ImgBase *useImage = image;
          if(useImage->getFormat() != formatRGB){
            ensureCompatible(&m_data->buffer,image->getDepth(),image->getSize(),formatRGB);
            cc(image,m_data->buffer);
            if(m_data->buffer->getDepth() == depth32f || m_data->buffer->getDepth() == depth64f){
              m_data->buffer->normalizeAllChannels(utils::Range64f(0,1));
            }
            useImage = m_data->buffer;
          }else if(useImage->getDepth() == depth32f || useImage->getDepth() == depth64f){
            ensureCompatible(&m_data->buffer,image->getDepth(),image->getSize(),formatRGB);
            image->deepCopy(&m_data->buffer);
            m_data->buffer->normalizeAllChannels(utils::Range64f(0,1));
            useImage = m_data->buffer;
          }


          try{
            unsigned int minsize = useImage->getDim()*core::getSizeOf(useImage->getDepth())*3;
            if(m_data->interleavedBuffer.size() < minsize){
              m_data->interleavedBuffer.resize(minsize);
            }
            void *data = m_data->interleavedBuffer.data();
            switch(useImage->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                        \
              case depth##D:                                              \
              core::planarToInterleaved(useImage->asImg<icl##D>(),         \
                                        reinterpret_cast<icl##D*>(data)); \
              break;
              ICL_INSTANTIATE_ALL_DEPTHS;
              default:
              ICL_INVALID_DEPTH;
  #undef ICL_INSTANTIATE_DEPTH
            }

            Magick::Image mi(useImage->getWidth(),useImage->getHeight(),"RGB",
                             get_magick_storage_type(useImage->getDepth()),
                             data);

            mi.write(file.getName());
          }catch(Magick::Error &err){
            throw ICLException(std::string("ImageMagick-FileWriter::")+err.what());
          }
          break;
        }

      case 4:{
          const ImgBase *useImage = image;

          try{
            unsigned int minsize = useImage->getDim()*core::getSizeOf(useImage->getDepth())*4;
            if(m_data->interleavedBuffer.size() < minsize){
              m_data->interleavedBuffer.resize(minsize);
            }
            void *data = m_data->interleavedBuffer.data();
            switch(useImage->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
              case depth##D:                                            \
                core::planarToInterleaved(useImage->asImg<icl##D>(),    \
                                          reinterpret_cast<icl##D*>(data)); \
              break;
              ICL_INSTANTIATE_ALL_DEPTHS;
              default:
              ICL_INVALID_DEPTH;
  #undef ICL_INSTANTIATE_DEPTH
            }

            Magick::Image mi(useImage->getWidth(),useImage->getHeight(),"RGBA",
                             get_magick_storage_type(useImage->getDepth()),
                             data);

            mi.write(file.getName());
          }catch(Magick::Error &err){
            throw ICLException(std::string("ImageMagick-FileWriter::")+err.what());
          }
          break;
        }
        default:
          ERROR_LOG("Yet ImageMagick FileWriterPlugin supports only 1, 3 and 4 channel data");
          throw ICLException("Unable to write image using FileWriterPluginImageMagick");
      }

    }
    void icl_initialize_image_magick_context(){
      static bool first = true;
      if(first){
        first = false;
        Magick::InitializeMagick( nullptr );
      }
    }

  #else
    void icl_initialize_image_magick_context(){}
    class FileWriterPluginImageMagick::InternalData{};
    FileWriterPluginImageMagick::FileWriterPluginImageMagick():m_data(0){}
    FileWriterPluginImageMagick::~FileWriterPluginImageMagick(){}

    void FileWriterPluginImageMagick::write(File&, const ImgBase*){}
  #endif

  } // namespace icl::io