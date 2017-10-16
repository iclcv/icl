/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileWriterPluginCSV.cpp                **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLIO/FileWriterPluginCSV.h>
#include <ICLCore/Types.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::core;

using namespace std;

namespace icl{
  namespace io{

    bool FileWriterPluginCSV::s_bExtendFileName = false;

    void FileWriterPluginCSV::setExtendFileName(bool value){
      s_bExtendFileName = value;
    }

    void FileWriterPluginCSV::write(File &file, const ImgBase *image){

      //////////////////////////////////////////////////////////////////////
      /// WRITE HEADER DATA DEPENDEND ON THE CURRENT EXTEND-FLAG-VALUE  ////
      //////////////////////////////////////////////////////////////////////

      if(s_bExtendFileName){
        std::ostringstream os;
        os << file.getDir() << file.getBaseName() << "-ICL:" << image->getSize() << 'x'
           << image->getChannels() << ':' <<image->getDepth() << ':' <<image->getFormat()
           << file.getSuffix();
        /*
            string newFileName = file.getDir()+
            file.getBaseName()+
            "-ICL:"+translateSize(image->getSize())+
            "x"+toStr(image->getChannels())+
            ":"+translateDepth(image->getDepth())+
            ":"+translateFormat(image->getFormat())+
            file.getSuffix();
        */
        file = File(os.str());
      }

      file.open(File::writeText);

      if(!s_bExtendFileName){
        std::ostringstream os;
        static const string H = "# ";
        Rect roi = image->getROI();
        os << H << "Size " << image->getWidth() << ' ' << image->getHeight() << std::endl
           << H << "Channels " << image->getChannels() << std::endl
           << H << "ROI" << roi.x << ' ' << roi.y << ' '  << roi.width << ' ' << roi.height << std::endl
           << H << "Format " << image->getFormat() << std::endl
           << H << "ImageDepth " << image->getDepth() << std::endl
           << H << "TimeStamp " << image->getTime() << std::endl;
        file << os.str();
      }

      //////////////////////////////////////////////////////////////////////
      /// WRITE THE IMAGE DATA TEMPLATE BASED  /////////////////////////////
      //////////////////////////////////////////////////////////////////////

      for(int c=0;c<image->getChannels();++c){
        for(int y=0;y<image->getHeight();++y){
          switch(image->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D) case depth##D:{                                \
            const icl##D *p = image->asImg<icl##D>()->getROIData(c,Point(0,y));   \
            for(int x=0;x<image->getWidth();++x){                                 \
              file << str<icl##D>(p[x]) << ",";                                   \
            }                                                                     \
            break;}
            ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
          }
          file << "\n";
        }
      }
    }
  } // namespace io
}

