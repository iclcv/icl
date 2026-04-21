// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/file-plugins/FileWriterPluginImageMagick.h>
#include <icl/core/CoreFunctions.h>

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

#include <icl/core/CCFunctions.h>
#include <icl/core/Converter.h>
#include <icl/utils/Exception.h>

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

#ifdef ICL_HAVE_IMAGEMAGICK
#include <icl/io/FileWriter.h>  // fileWriterRegistry
namespace {
  using icl::io::FileWriterPluginImageMagick;
  using icl::io::fileWriterRegistry;

  // ImageMagick claims a *lot* of formats. Register them all under one
  // factory. JPEG/PNG slots are also wired here as a fallback for builds
  // without libjpeg/libpng — registerExtension does first-wins by
  // default, so libjpeg/libpng's registrations (in their own .cpp files)
  // take precedence when those libs are present.
  static const char *imageMagickFormats[] = {
    "png","jpeg","jpg",
    "gif","pdf","ps","avs","bmp","cgm","cin","cur","cut","dcx",
    "dib","dng","dot","dpx","emf","epdf","epi","eps","eps2","eps3",
    "epsf","epsi","ept","fax","gplt","gray","hpgl","html","ico","info",
    "jbig","jng","jp2","jpc","man","mat","miff","mono","mng","mpeg","m2v",
    "mpc","msl","mtv","mvg","palm","pbm","pcd","pcds","pcl","pcx","pdb",
    "pfa","pfb","picon","pict","pix","ps2","ps3","psd","ptif","pwp",
    "rad","rgb","pgba","rla","rle","sct","sfw","sgi","shtml","sun","svg",
    "tga","tiff","tim","ttf","txt","uil","uyuv","vicar","viff","wbmp",
    "wmf","wpg","xbm","xcf","xpm","xwd","ydbcr","ycbcra","yuv",nullptr
  };
}

// One shared IM impl across all extensions, protected by its own mutex.
// This differs from the pre-4b per-extension-instance caching but is
// semantically stricter (IM's InternalData holds a single Converter +
// buffers, so concurrent writes already raced — now they're serialized).
static void iclImageMagickWrite(icl::utils::File &f, const icl::core::ImgBase *img) {
  static std::mutex m;
  static FileWriterPluginImageMagick impl;
  std::scoped_lock lock(m);
  impl.write(f, img);
}

extern "C" __attribute__((constructor, used)) void
iclRegisterFileWriterPluginsImageMagick() {
  // Register at low priority so libpng / libjpeg (priority 0) win for
  // extensions they also claim. For formats libpng/libjpeg don't handle
  // (tiff, gif, bmp, svg, …), ImageMagick's registration is unopposed
  // and wins by default.
  constexpr int kImageMagickPriority = -10;
  for (const char **pc = imageMagickFormats; *pc; ++pc) {
    fileWriterRegistry().registerPlugin(std::string(".") + *pc,
                                        &iclImageMagickWrite,
                                        /*description*/ {},
                                        kImageMagickPriority);
  }
}
#endif