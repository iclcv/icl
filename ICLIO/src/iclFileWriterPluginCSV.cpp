#include <iclFileWriterPluginCSV.h>
#include <iclTypes.h>
#include <iclStringUtils.h>
#include <iclIOUtils.h>

using namespace std;

namespace icl{

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
}

