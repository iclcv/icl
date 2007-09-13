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
    
    void pnm_write3(File &file,const ImgBase *image, int channeloffs, vector<icl8u> &buffer){
      const ImgBase *img = image->selectChannels(ioutils::vec3(channeloffs,channeloffs+1,channeloffs+2));
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
}

