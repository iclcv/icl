#include <iclFileGrabberPluginPNM.h>
#include <iclException.h>
#include <iclStringUtils.h>
#include <iclCC.h>
#include <iclIOUtils.h>

using namespace std;
using namespace icl::ioutils;

namespace icl{

  void FileGrabberPluginPNM::grab(File &file, ImgBase **dest){
    ICLASSERT_RETURN(dest);
    file.open(File::readBinary); 

    string nextLine;
    bool bIsICL = file.getSuffix() == ".icl" || file.getSuffix() == ".icl.gz";

    //////////////////////////////////////////////////////////////////////
    /// READ HEADER INFORMATION FROM THE FILE  ///////////////////////////
    //////////////////////////////////////////////////////////////////////
    FileGrabberPlugin::HeaderInfo oInfo;
    oInfo.imageFormat = formatGray;
    oInfo.imageDepth = depth8u;
    oInfo.channelCount = 0;
    oInfo.imageCount = 1;

    if(!bIsICL){
      string l = file.readLine();
      if(l.length() < 2 || l[0] != 'P') throw InvalidFileFormatException();
      switch (l[1]) {
        case '6': oInfo.imageFormat = formatRGB; break;
        case '5': oInfo.imageFormat = formatGray; break;
        default: throw InvalidFileFormatException();
      }
    }
    oInfo.channelCount = getChannelsOfFormat(oInfo.imageFormat);
    
    // {{{ Read special header info

    
    do {
      nextLine = ioutils::skipWhitespaces(file.readLine());
      vector<string> ts = tok(nextLine," ");
      if(ts.size() < 3) break;
    
      string sKey = ts[1];
    
      if (sKey == "NumFeatures" || sKey == "NumImages") {
        oInfo.imageCount = ti(ts[2]);
        if(!oInfo.imageCount) throw InvalidFileFormatException();
        oInfo.channelCount *= oInfo.imageCount;
      } else if (sKey == "ROI") {
        oInfo.roi = Rect(ti(ts[2]),ti(ts[3]),ti(ts[4]),ti(ts[5]));
        continue;
      } else if (sKey == "ImageDepth") {
        if (!bIsICL) continue; // ignore image depth for all formats but ICL
        oInfo.imageDepth = translateDepth( ts[2] );
        continue;
      } else if (sKey == "Format") {
        oInfo.imageFormat = translateFormat(ts[2]);
      } else if (sKey == "TimeStamp") {
        oInfo.time = Time::microSeconds(tl(ts[2]));
        continue;
      }
      if(getChannelsOfFormat(oInfo.imageFormat) != oInfo.channelCount){
        oInfo.imageFormat = formatMatrix;
      }
    } while (true);
    
    // }}}

    // read image size 
    vector<string> ts = tok(nextLine," ");
    if(ts.size() != 2) throw InvalidFileFormatException();
    if(bIsICL){
      oInfo.size = Size(ti(ts[0]),ti(ts[1]));
    }else{
      oInfo.size = Size(ti(ts[0]),ti(ts[1])/oInfo.imageCount);
    }
  
    // the next line shows the maximal pixel value -> skip 
    if(file.readLine().size() == 0) throw InvalidFileFormatException();

    //////////////////////////////////////////////////////////////////////
    /// ADAPT THE DESTINATION IMAGE //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////

    //printf("destination roi = %s \n",translateRect(oInfo.roi).c_str());
    //printf("destination size = %s \n",translateSize(oInfo.size).c_str());
    
    ensureCompatible (dest, oInfo.imageDepth, oInfo.size,
                      oInfo.channelCount,oInfo.imageFormat, oInfo.roi);

    ImgBase *poImg = *dest;
    poImg->setTime(oInfo.time);

    //////////////////////////////////////////////////////////////////////
    /// READ THE DATA ////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    if (oInfo.imageCount == oInfo.channelCount || bIsICL) {
       // file format is non-interleaved, i.e. grayscale or icl proprietary
       int iDim = poImg->getDim () * getSizeOf (poImg->getDepth ());
       for (int i=0;i<oInfo.channelCount;i++){
         if(file.read(iDim,poImg->getDataPtr(i)) != iDim){
           throw InvalidFileFormatException();
         }
       }
    } else if (poImg->getDepth() == depth8u) {
      /// interleaved channels image by image
      int iImageStep = poImg->getDim()*(poImg->getChannels()/oInfo.imageCount)*sizeof(icl8u);
      if(file.bytesAvailable() < oInfo.imageCount*iImageStep){
        throw InvalidFileFormatException();
      }
      Img8u buf;
      for(int i=0;i<oInfo.imageCount;i++){
        poImg->asImg<icl8u>()->selectChannels(vec3(3*i,3*i+1,3*i+2),&buf);
        buf.setFullROI();
        interleavedToPlanar(file.getCurrentDataPointer()+i*iImageStep,&buf);
      }
    } else {
      ERROR_LOG ("This should not happen!");
    }
  }

}


