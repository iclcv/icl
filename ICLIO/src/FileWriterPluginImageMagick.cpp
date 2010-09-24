/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/FileWriterPluginImageMagick.cpp              **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLIO/FileWriterPluginImageMagick.h>

#ifdef HAVE_IMAGEMAGICK
#ifndef ICL_SYSTEM_WINDOWS
#include <GraphicsMagick/Magick++.h>
#else
#include <Magick++.h>
#endif
#include <ICLCC/CCFunctions.h>
#include <ICLCC/Converter.h>
#include <ICLUtils/Exception.h>
#endif

namespace icl{
  
#ifdef HAVE_IMAGEMAGICK
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
  FileWriterPluginImageMagick::FileWriterPluginImageMagick():m_data(new FileWriterPluginImageMagick::InternalData){}
  FileWriterPluginImageMagick::~FileWriterPluginImageMagick(){
    ICL_DELETE(m_data);
  }
  
#ifdef HAVE_IMAGEMAGICK
  Magick::StorageType get_magick_storage_type(depth d){
    switch(d){
      case depth8u: return Magick::CharPixel;
      case depth16s: return Magick::ShortPixel;
      case depth32s: return Magick::IntegerPixel;
      case depth32f: return Magick::FloatPixel;
      case depth64f: return Magick::DoublePixel;
      default:
        ERROR_LOG("unknown depth");
        return Magick::CharPixel;
    }
  }
#endif  
  void FileWriterPluginImageMagick::write(File &file, const ImgBase *image){
    switch(image->getChannels()){
      case 1:{
        const ImgBase *useImage = image;
        if(image->getDepth() == depth32f || image->getDepth() == depth64f){
          icl::ensureCompatible(&m_data->buffer,image->getDepth(),image->getParams());
          image->deepCopy(&m_data->buffer);
          m_data->buffer->normalizeAllChannels(icl::Range64f(0,1));
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
            m_data->buffer->normalizeAllChannels(icl::Range64f(0,1));
          }
          useImage = m_data->buffer;
        }else if(useImage->getDepth() == depth32f || useImage->getDepth() == depth64f){
          ensureCompatible(&m_data->buffer,image->getDepth(),image->getSize(),formatRGB);
          image->deepCopy(&m_data->buffer);
          m_data->buffer->normalizeAllChannels(icl::Range64f(0,1));
          useImage = m_data->buffer;
        }
        
        
        try{
          unsigned int minsize = useImage->getDim()*icl::getSizeOf(useImage->getDepth())*3;
          if(m_data->interleavedBuffer.size() < minsize){
            m_data->interleavedBuffer.resize(minsize);
          }
          void *data = m_data->interleavedBuffer.data();
          switch(useImage->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
            case depth##D:                                              \
            icl::planarToInterleaved(useImage->asImg<icl##D>(),         \
                                     reinterpret_cast<icl##D*>(data));  \
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
          unsigned int minsize = useImage->getDim()*icl::getSizeOf(useImage->getDepth())*4;
          if(m_data->interleavedBuffer.size() < minsize){
            m_data->interleavedBuffer.resize(minsize);
          }
          void *data = m_data->interleavedBuffer.data();
          switch(useImage->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
            case depth##D:                                              \
            icl::planarToInterleaved(useImage->asImg<icl##D>(),         \
                                     reinterpret_cast<icl##D*>(data));  \
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

#else
  class FileWriterPluginImageMagick::InternalData{};
  FileWriterPluginImageMagick::FileWriterPluginImageMagick():m_data(0){}
  FileWriterPluginImageMagick::~FileWriterPluginImageMagick(){}
  
  void FileWriterPluginImageMagick::write(File&, const ImgBase*){}
#endif

}
