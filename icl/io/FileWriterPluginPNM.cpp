// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting

#include <icl/io/FileWriterPluginPNM.h>
#include <string>
#include <vector>
#include <icl/core/CCFunctions.h>
#include <icl/utils/Time.h>
#include <icl/utils/StringUtils.h>
using namespace icl::utils;
using namespace icl::core;


namespace icl{
  namespace io{

    namespace{
      static std::vector<int> vec3(int a, int b, int c){
        int abc[] = {a,b,c};
        return std::vector<int>(abc,abc+3);
      }

      void pnm_write_gray2rgb(File &file, const ImgBase *poSrc, std::vector<icl8u> &buffer){
        int dim = poSrc->getDim();
        buffer.resize(3*dim);
        switch(poSrc->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                        \
          case depth##D:{                                                 \
            const icl##D *src = poSrc->asImg<icl##D>()->getData(0);       \
            icl8u *dst = &buffer[0];                                      \
            for(int i=0;i<dim;i++){                                       \
              dst[3*i]=dst[3*i+1]=dst[3*i+2]=clipped_cast<icl##D,icl8u>(src[i]); \
            }                                                             \
            break;}
          ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
        }
        file.write(&buffer[0],buffer.size());
      }
      void pnm_write_2channels2rgb(File &file, const ImgBase *poSrc, std::vector<icl8u> &buffer){
        int dim = poSrc->getDim();
        buffer.resize(3*dim);
        switch(poSrc->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                             \
          case depth##D:{                                                      \
            const icl##D *src1 = poSrc->asImg<icl##D>()->getData(0);           \
            const icl##D *src2 = poSrc->asImg<icl##D>()->getData(1);           \
            icl8u *dst = &buffer[0];                                           \
            for(int i=0;i<dim;i++){                                            \
              dst[3*i]=clipped_cast<icl##D,icl8u>(src1[i]);                      \
              dst[3*i+1]=clipped_cast<icl##D,icl8u>(src2[i]);                    \
              dst[3*i+2]=0;                                                    \
            }                                                                  \
            break;}
          ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
        }
        file.write(&buffer[0],buffer.size());
      }

      void pnm_write3(File &file,const ImgBase *image, int channeloffs, std::vector<icl8u> &buffer){
        Rect fullROI(Point::null,image->getSize());
        format fmt = image->getFormat();
        const ImgBase *img = image->shallowCopy(fullROI,vec3(channeloffs,channeloffs+1,channeloffs+2),fmt);
        buffer.resize(3*image->getDim());
        switch(img->getDepth()){

  #define ICL_INSTANTIATE_DEPTH(D) case depth##D: planarToInterleaved<icl##D,icl8u>(img->asImg<icl##D>(),&buffer[0]); break;
          ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
        }
        file.write(&buffer[0],buffer.size());
        delete img;
      }

      void pgm_write(File &file,const ImgBase *image, int channel, std::vector<icl8u> &buffer){
        buffer.resize(image->getDim());
        switch(image->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                                            \
          case depth##D:                                                                      \
            convert<icl##D,icl8u>(image->asImg<icl##D>()->getData(channel),                   \
                                  image->asImg<icl##D>()->getData(channel)+image->getDim(),   \
                                  &buffer[0]);                                                \
            break;
          ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
        }
        file.write(&buffer[0],buffer.size());
      }
    }

    void FileWriterPluginPNM::write(File &file, const ImgBase *poSrc){
      ICLASSERT_RETURN(poSrc);
      file.open(File::writeText);

      std::string suffix = toLower( file.getSuffix() );
      bool bPPM=false;
      bool bICL = suffix == ".icl" || suffix == ".icl.gz";
      int  iNumImages = poSrc->getChannels ();


      if(suffix == ".ppm" || suffix == ".ppm.gz") {
        bPPM = true;
        if( (poSrc->getChannels() % 3)  && poSrc->getChannels() > 2 ){
          throw ICLException ("Image cannot be written as \".ppm\" channel count must be 1, 2 or a multiple of 3!");
        }
      }else if (suffix == ".pnm" || suffix == ".pnm.gz") {
        bPPM = (poSrc->getChannels () == 3 && getChannelsOfFormat(poSrc->getFormat()) == 3);
      }
      if(bPPM){
        if(poSrc->getChannels() % 3){
          iNumImages = 1;
        }else{
          iNumImages /= 3;
        }
      }
      bool bPGM = !bICL && !bPPM;

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      //// WRITE HEADER INFORMATION  ////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////

      std::string newline = "\n";

      if(!bICL){
        file << std::string(bPPM ? "P6" :"P5") << newline;
      }


      std::ostringstream os;

      static const std::string H = "# ";
      Rect roi = poSrc->getROI();

      os   << H << "TimeStamp " << poSrc->getTime() << newline
           << H << "NumFeatures " << iNumImages << newline
           << H << "ImageDepth " << poSrc->getDepth() << newline
           << H << "ROI " << roi.x << ' ' << roi.y << ' '  << roi.width << ' ' << roi.height << newline
           << H << "Format " << poSrc->getFormat() << newline // not shure, this is new!
           << poSrc->getWidth() << " " << poSrc->getHeight()*(bICL ? 1 : iNumImages) << newline << 255 << newline;

      file << os.str();

      ///////////////////////////////////////////////////////////////////////////////////////////////////
      //// WRITE IMAGE DATA  ////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////

      if (bPPM) { // file format is interleaved, i.e. RGB or something similar
        if(!(poSrc->getChannels() % 3)){
          for(int i=0;i<iNumImages;i++){
            m_oBufferMutex.lock();
            pnm_write3(file, poSrc, 3*i, m_vecBuffer);
            m_oBufferMutex.unlock();
          }
        }else{
          m_oBufferMutex.lock();
          if( poSrc->getChannels() == 1){
            pnm_write_gray2rgb(file, poSrc,m_vecBuffer);
          }else if(poSrc->getChannels() == 2){
            pnm_write_2channels2rgb(file, poSrc,m_vecBuffer);
          }else{
            throw ICLException ("Error writing file as .ppm! (This case may not occur)!");
          }
          m_oBufferMutex.unlock();
        }
      }else if(bPGM){
        m_oBufferMutex.lock();
        for (int i=0;i<iNumImages;i++) {
          pgm_write(file, poSrc, i, m_vecBuffer);
        }
        m_oBufferMutex.unlock();
      }else{
        for (int i=0;i<iNumImages;i++) {
          file.write(poSrc->getDataPtr(i),poSrc->getDim () * getSizeOf(poSrc->getDepth()));
        }
      }
    }
  } // namespace io
}


/*
      void FileWriterPluginPNM::write(File &file, const ImgBase *poSrc){

    ICLASSERT_RETURN(poSrc);
    file.open(File::writeText);
    ICLASSERT_RETURN(file.isOpen());

    std::string suffix = toLower( file.getSuffix() );
    bool bPPM=false;
    bool bICL = suffix == ".icl" || suffix == ".icl.gz";
    int  iNumImages = poSrc->getChannels ();


    if(suffix == ".ppm" || suffix == ".ppm.gz") {
      bPPM = !(poSrc->getChannels() % 3);
      if (!bPPM) throw ICLException ("Image cannot be written as \".ppm\" channel count must be a multiple of 3!");
    }else if (suffix == ".pnm" || suffix == ".pnm.gz") {
      bPPM = (poSrc->getChannels () == 3 && getChannelsOfFormat(poSrc->getFormat()) == 3);
    }
    if(bPPM){
      iNumImages/=3;
    }
    bool bPGM = !bICL && !bPPM;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //// WRITE HEADER INFORMATION  ////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    std::string newline = "\n";

    if(!bICL){
      file << std::string(bPPM ? "P6" :"P5") << newline;
    }
    file << "# Format " << translateFormat(poSrc->getFormat()) << newline;
    file << "# TimeStamp " << ioutils::time2str( poSrc->getTime().toMicroSeconds() ) << newline;
    file << "# NumFeatures " << iNumImages << newline;
    file << "# ImageDepth " << translateDepth(poSrc->getDepth()) << newline;
    file << "# ROI " << poSrc->getROI().x << " " << poSrc->getROI().y
         << " " << poSrc->getROI().width << " "  << poSrc->getROI().height << newline;
    file << poSrc->getWidth() << " " << poSrc->getHeight()*(bICL ? 1 : iNumImages) << newline << 255 << newline;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //// WRITE IMAGE DATA  ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    if (bPPM) { // file format is interleaved, i.e. RGB or something similar
      ICLASSERT_RETURN(poSrc->getChannels() > 2);
      for(int i=0;i<iNumImages;i++){
        m_oBufferMutex.lock();
        pnm_write3(file, poSrc, 3*i, m_vecBuffer);
        m_oBufferMutex.unlock();
      }
    }else if(bPGM){
      m_oBufferMutex.lock();
      for (int i=0;i<iNumImages;i++) {
        pgm_write(file, poSrc, i, m_vecBuffer);
      }
      m_oBufferMutex.unlock();
    }else{
      for (int i=0;i<iNumImages;i++) {
        file.write(poSrc->getDataPtr(i),poSrc->getDim () * getSizeOf(poSrc->getDepth()));
      }
    }
  }


    */
