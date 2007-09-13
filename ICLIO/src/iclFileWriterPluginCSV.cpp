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
      string newFileName = file.getDir()+
                           file.getBaseName()+
                           "-ICL:"+translateSize(image->getSize())+
                           "x"+toStr(image->getChannels())+
                           ":"+translateDepth(image->getDepth())+
                           ":"+translateFormat(image->getFormat())+
                           file.getSuffix();
      file = File(newFileName);
    }
    
    file.open(File::writeText);
    ICLASSERT(file.isOpen());
    
    if(!s_bExtendFileName){
      static const string H = "# ";
      file.write(H+"Size "+str(image->getWidth())+" "+str(image->getHeight())+'\n');
      file.write(H+"Channels "+str(image->getChannels())+'\n');
      Rect roi = image->getROI();
      file.write(H+"ROI "+str(roi.x)+" "+str(roi.y)+" "+str(roi.width)+" "+str(roi.height)+'\n');
      file.write(H+"Format "+translateFormat(image->getFormat())+'\n');
      file.write(H+"ImageDepth "+translateDepth(image->getDepth())+'\n');
      file.write(H+"TimeStamp "+ioutils::time2str(image->getTime().toMicroSeconds())+'\n');
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

