#include "iclFileWriterPluginPNM.h"
#include <string>
#include <vector>
#include <iclCC.h>
#include <iclTime.h>
#include "iclIOUtils.h"
#include "iclStringUtils.h"

using namespace std;
using namespace icl::ioutils;

namespace icl{
  
  namespace{

    void pnm_write_gray2rgb(File &file, const ImgBase *poSrc, vector<icl8u> &buffer){
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
    void pnm_write_2channels2rgb(File &file, const ImgBase *poSrc, vector<icl8u> &buffer){
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

    void pnm_write3(File &file,const ImgBase *image, int channeloffs, vector<icl8u> &buffer){
      Rect fullROI(Point::null,image->getSize());
      format fmt = image->getFormat();
      const ImgBase *img = image->shallowCopy(fullROI,ioutils::vec3(channeloffs,channeloffs+1,channeloffs+2),fmt);
      buffer.resize(3*image->getDim());
      switch(img->getDepth()){

#define ICL_INSTANTIATE_DEPTH(D) case depth##D: planarToInterleaved<icl##D,icl8u>(img->asImg<icl##D>(),&buffer[0]); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
      file.write(&buffer[0],buffer.size());
      delete img;
    }
    
    void pgm_write(File &file,const ImgBase *image, int channel, vector<icl8u> &buffer){
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
    
    string suffix = toLower( file.getSuffix() );    
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
    
    string endl = "\n";
    
    if(!bICL){
      file << string(bPPM ? "P6" :"P5") << endl;
    }
    file << "# Format " << translateFormat(poSrc->getFormat()) << endl;
    file << "# TimeStamp " << ioutils::time2str( poSrc->getTime().toMicroSeconds() ) << endl;
    file << "# NumFeatures " << iNumImages << endl;
    file << "# ImageDepth " << translateDepth(poSrc->getDepth()) << endl;
    file << "# ROI " << poSrc->getROI().x << " " << poSrc->getROI().y 
         << " " << poSrc->getROI().width << " "  << poSrc->getROI().height << endl;
    file << poSrc->getWidth() << " " << poSrc->getHeight()*(bICL ? 1 : iNumImages) << endl << 255 << endl;

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
}


/*
      void FileWriterPluginPNM::write(File &file, const ImgBase *poSrc){
    
    ICLASSERT_RETURN(poSrc);
    file.open(File::writeText);
    ICLASSERT_RETURN(file.isOpen());
    
    string suffix = toLower( file.getSuffix() );    
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
    
    string endl = "\n";
    
    if(!bICL){
      file << string(bPPM ? "P6" :"P5") << endl;
    }
    file << "# Format " << translateFormat(poSrc->getFormat()) << endl;
    file << "# TimeStamp " << ioutils::time2str( poSrc->getTime().toMicroSeconds() ) << endl;
    file << "# NumFeatures " << iNumImages << endl;
    file << "# ImageDepth " << translateDepth(poSrc->getDepth()) << endl;
    file << "# ROI " << poSrc->getROI().x << " " << poSrc->getROI().y 
         << " " << poSrc->getROI().width << " "  << poSrc->getROI().height << endl;
    file << poSrc->getWidth() << " " << poSrc->getHeight()*(bICL ? 1 : iNumImages) << endl << 255 << endl;

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
