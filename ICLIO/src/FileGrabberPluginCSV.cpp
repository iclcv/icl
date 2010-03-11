/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLIO/FileGrabberPluginCSV.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/StrTok.h>

#include <string>
#include <vector>
#include <ICLIO/IOUtils.h>
#include <ICLUtils/StringUtils.h>

using namespace std;

namespace icl{
  namespace{
    
    template<class T>
    void tokenzie_line_tmpl(const std::vector<string> &v, T *data){
      // {{{ open

      for(unsigned int i=0;i<v.size();i++){
        data[i] = clipped_cast<icl64f,T>(atof(v[i].c_str()));
      }
    }

    // }}}
    
    void tokezize_line(const std::vector<string> &v,ImgBase *image, int c, int y){
      // {{{ open

      if(image->getWidth() > (int)v.size()){
        throw InvalidFileFormatException(str("image width:")+
                                         str(image->getWidth())+ 
                                         str(" line width:") + 
                                         str(v.size()));
      }
      switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: tokenzie_line_tmpl(v,image->asImg<icl##D>()->getROIData(c,Point(0,y))); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }

    // }}}
  }
  
  FileGrabberPluginCSV::FileGrabberPluginCSV(){
    // {{{ open

    m_poReadBuffer = new Img64f;
    m_poReadBufferMutex = new Mutex;
  }

  // }}}
  FileGrabberPluginCSV::~FileGrabberPluginCSV(){
    // {{{ open

    ICL_DELETE( m_poReadBuffer );
    ICL_DELETE( m_poReadBufferMutex );
  }

  // }}}

  void FileGrabberPluginCSV::grab(File &file, ImgBase **dest){
    // {{{ open

    ICLASSERT_RETURN(dest);
    file.open(File::readText);
    ICLASSERT_RETURN(file.isOpen());
    
    //////////////////////////////////////////////////////////////////////
    //// Estimate Header Data 1st: look at the filename  /////////////////
    //////////////////////////////////////////////////////////////////////
    FileGrabberPlugin::HeaderInfo oInfo;
    oInfo.channelCount = 1;
    oInfo.imageDepth = depth64f;
    oInfo.imageFormat = formatMatrix;
    oInfo.size = Size::null;
    oInfo.roi = Rect::null;
    oInfo.time = Time();

    string filename = file.getBaseName();       //printf("basename is {%s} \n",filename.c_str());
    string line;
    
    string::size_type t = string::npos;
    if((t=filename.find("-ICL:"))!= string::npos){
      while(1){ // this is necessary to use break on errors
        vector<string> ts = tok(filename.substr(t+5),":");
        if(ts.size() != 3){
          ERROR_LOG("Invalid ICL-CSV filen name appendix in \"" << filename << "\"[CODE 1]"); 
          break;
        }
        vector<string> whc = StrTok(ts[0],"x").allTokens();
        if(whc.size() != 3) {
          ERROR_LOG("Invalid ICL-CSV filen name appendix in \"" << filename << "\"[CODE 2]"); 
          break;
        }
        oInfo.imageDepth = parse<depth>(ts[1]);
        oInfo.imageFormat = parse<format>(ts[2]);
        oInfo.size = parse<Size>(whc[0]+"x"+whc[1]);
        oInfo.roi = Rect(Point::null,oInfo.size);
        oInfo.channelCount = parse<int>(whc[2]);
        oInfo.time = Time();
        break;
      }
      line = file.readLine();
    }else do{
      line = ioutils::skipWhitespaces(file.readLine());
      if(line.length() && line[0] == '#'){
        vector<string> ts = tok(line," ");
        if(ts.size() < 3) continue;
        
        if (ts[1] == "ROI") {
          oInfo.roi = Rect(atoi(ts[2].c_str()),atoi(ts[3].c_str()),atoi(ts[4].c_str()),atoi(ts[5].c_str()));
          continue;
        } else if (ts[1] == "ImageDepth") {
          oInfo.imageDepth = parse<depth>( ts[2] );
          continue;
        } else if (ts[1] == "Format") {
          oInfo.imageFormat = parse<format>(ts[2]);
        } else if (ts[1] == "TimeStamp") {
          oInfo.time = Time::microSeconds(atoi(ts[2].c_str()));
          continue;
        } else if (ts[1] == "Size") {
          oInfo.size = Size(parse<int>(ts[2]),parse<int>(ts[3]));
          continue;
        } else if (ts[1] == "Channels") {
          oInfo.channelCount = parse<int>(ts[2]);
          continue;
        }
      }else{
        break;
      }        
    }while(true);
    //////////////////////////////////////////////////////////////////////
    // CHECK IF A "SIZE" HINT WAS FOUND (IN THE HEADER OR IN THE FILENAME/
    //////////////////////////////////////////////////////////////////////
    if(oInfo.size == Size::null){
      /// analyse the file and reopen it!
      bool end = false;
      while(!end){
        oInfo.size.height++; // additional line
        oInfo.size.width = iclMax((string::size_type)oInfo.size.width,tok(line,",").size());
        if(file.hasMoreLines()){
          line = file.readLine();
        }else{
          end = true;
        }
      }
      file.reset();
      line = file.readLine();
      // skip header again
      while(file.hasMoreLines() && line.length() && line[0]=='#'){
        line = file.readLine();
      }
      oInfo.imageDepth = depth64f;
      oInfo.imageFormat = formatMatrix;
      oInfo.roi = Rect(Point::null,oInfo.size);
      oInfo.channelCount = 1;
      oInfo.time = Time();
    }    
    //////////////////////////////////////////////////////////////////////
    /// ADAPT THE DESTINATION IMAGE //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    ensureCompatible (dest, oInfo.imageDepth, oInfo.size,
                      oInfo.channelCount,oInfo.imageFormat, oInfo.roi);

    ImgBase *poImg = *dest;
    poImg->setTime(oInfo.time);

    //////////////////////////////////////////////////////////////////////
    /// READ THE IMAGE DATA  /////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    for(int c=0;c<poImg->getChannels();c++){
      for(int y=0;y<poImg->getHeight();++y){
        tokezize_line(tok(line,","),poImg,c,y);
        if(!(c==poImg->getChannels()-1 && y == poImg->getHeight()-1)){
          line = file.readLine();
        }
      }
    }    
  }  

  // }}}
}

