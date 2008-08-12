#include "iclFileGrabberPluginImageMagick.h"
#include <iclCC.h>
#include <iclCore.h>

#ifdef WITH_IMAGEMAGIC_SUPPORT  
#include <Magick++.h>
#endif

namespace icl{


#ifdef WITH_IMAGEMAGIC_SUPPORT  
  struct FileGrabberPluginImageMagick::InternalData{
    std::vector<icl8u> buffer;


  };
  
  FileGrabberPluginImageMagick::FileGrabberPluginImageMagick():
    m_data(new FileGrabberPluginImageMagick::InternalData){}
  
  FileGrabberPluginImageMagick::~FileGrabberPluginImageMagick(){
    delete m_data;
  }
  
  void FileGrabberPluginImageMagick::grab(File &file, ImgBase **dest){
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
    image->getConstPixels(0,0,size.width,size.height);
    image->writePixels(Magick::RGBQuantum,m_data->buffer.data());
    
    icl::ensureCompatible(dest,depth8u,size,formatRGB);
    icl::interleavedToPlanar(m_data->buffer.data(),(*dest)->asImg<icl8u>());
    
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
}


