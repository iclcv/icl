/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/FileWriterPluginPNG.cpp                      **
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

#include <ICLIO/FileWriterPluginPNG.h>
#include <ICLCore/Types.h>
#include <ICLUtils/StringUtils.h>
//#include <ICLIO/IOUtils.h>
#include <ICLCC/CCFunctions.h>

#include <png.h>
#include <cstdio>

namespace icl{


  void FileWriterPluginPNG::write(File &file, const ImgBase *image){
    Mutex::Locker lock(mutex);

    DEBUG_LOG("writing png file " << file.getName());
    
    ICLASSERT_RETURN(image);

    FILE *cfile = fopen(file.getName().c_str(), "wb");
    if (!cfile){
      ERROR_LOG("error opening png file " << file.getName());
      return;
    }

    png_structp writer = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!writer){
      ERROR_LOG("unable to create png writer struct for file " << file.getName());
      return;
    }

    png_infop info = png_create_info_struct(writer);
    if (!info){
      ERROR_LOG("unable to create png info struct for file " << file.getName());
      return;
    }
    
    
    if (setjmp(png_jmpbuf(writer))){
      ERROR_LOG("error while initing png IO for file " << file.getName());
      return;
    }

    png_init_io(writer,cfile);
    
    if (setjmp(png_jmpbuf(writer))){
      ERROR_LOG("error writing png header for file " << file.getName());
      return;
    }
    
    depth d = image->getDepth(); (void)d;
    int w = image->getWidth();
    int h = image->getHeight();
    int c = image->getChannels();

    if(c < 1 || c > 4){
      ERROR_LOG("the png format supports only 1,2,3 and 4 channel images");
      return;
    }
    
    // Note that tests have 0: no compression, 9: max
    // shown that zlib compression levels 3-6 usually perform as well as level 9
    // for PNG images, and do considerably fewer caclulations. 
    png_set_compression_level(writer, 4);

    png_set_IHDR(writer,info,w,h,
                 8, // bits for now: later you will be able to select this
                 c==1 ? PNG_COLOR_TYPE_GRAY :
                 c==2 ? PNG_COLOR_TYPE_GRAY_ALPHA :
                 c==3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
    png_write_info(writer,info);
   
    const icl8u *p = 0;
    switch(c){
      case 1: 
        if(image->getDepth() == depth8u){
          p = image->asImg<icl8u>()->begin(0);
        }else{
          data.resize(w*h);
          Img8u tmp(Size(w,h),1,std::vector<icl8u*>(1,data.data()));
          image->convert(&tmp);
          p = tmp.begin(0);
        }
        break;
      case 2: case 3: case 4:
        data.resize(w*h*c);
        switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: planarToInterleaved(image->asImg<icl##D>(),data.data()); break;
          ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
          default:
          ICL_INVALID_DEPTH;
        }
        
        p = data.data();
        break;
      default: 
        ERROR_LOG("unable to write an image with " << image->getChannels() << " as png file");
        return;
    }
    rows.resize(h);
    for(int i=0;i<h;++i){
      rows[i] = (icl8u*)(p+i*w*c);
    }
   
    if(setjmp(png_jmpbuf(writer))){
      ERROR_LOG("an error occured while writing png file " << file.getName());
      return;
    }

    png_write_image(writer, (png_bytep*)rows.data());


    if(setjmp(png_jmpbuf(writer))){
      ERROR_LOG("an error occured while finalizing writing of png file " << file.getName());
      return;
    }
    
    png_write_end(writer, NULL);

    fclose(cfile);
  }
  
  
  
}

