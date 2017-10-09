/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileGrabberPluginPNM.cpp               **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Goetting                  **
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

#include <ICLIO/FileGrabberPluginPNM.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/CCFunctions.h>

using namespace std;
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    static int ti(const std::string &t) { return parse<int>(t); }
    static std::vector<int> vec3(int a, int b, int c){
      int abc[] = {a,b,c};
      return std::vector<int>(abc,abc+3);
    }

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
        nextLine = skipWhitespaces(file.readLine());
        vector<string> ts = tok(nextLine," ");

        if(ts.size() < 3) continue;
        string sKey = ts[1];

        if (sKey == "NumFeatures" || sKey == "NumImages") {
          oInfo.imageCount = parse<int>(ts[2]);
          if(!oInfo.imageCount) throw InvalidFileFormatException();
          oInfo.channelCount *= oInfo.imageCount;
        } else if (sKey == "ROI") {
          oInfo.roi = Rect(ti(ts[2]),ti(ts[3]),ti(ts[4]),ti(ts[5]));
          continue;
        } else if (sKey == "ImageDepth") {
          if (!bIsICL) continue; // ignore image depth for all formats but ICL
          oInfo.imageDepth = parse<depth>( ts[2] );
          continue;
        } else if (sKey == "Format") {
          oInfo.imageFormat = parse<format>(ts[2]);
        } else if (sKey == "TimeStamp") {
          oInfo.time = parse<Time>(ts[2]);
          continue;
        }
        if(getChannelsOfFormat(oInfo.imageFormat) != oInfo.channelCount){
          oInfo.imageFormat = formatMatrix;
        }
      } while (nextLine.length()==0 || nextLine[0]=='#');

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

  } // namespace io
}


