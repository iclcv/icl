/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/FileGrabber.cpp                              **
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

#include <string>
#include <map>
#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileList.h>
#include <ICLIO/FilenameGenerator.h>
#include <ICLUtils/Exception.h>
#include <ICLIO/IOUtils.h>
#include <ICLUtils/StringUtils.h>
// plugins
#include <ICLIO/FileGrabberPluginPNM.h>
#include <ICLIO/FileGrabberPluginCSV.h>

#ifdef HAVE_LIBJPEG
#include <ICLIO/FileGrabberPluginJPEG.h>
#endif

#ifdef HAVE_IMAGEMAGICK
#include <ICLIO/FileGrabberPluginImageMagick.h>
#endif
using namespace std;

namespace icl{

  map<string,FileGrabberPlugin*> FileGrabberImpl::s_mapPlugins;

  struct FileGrabberPluginMapInitializer{
    // {{{ open PLUGINS ARE INCLUDED HERE

    FileGrabberPluginMapInitializer(){
      FileGrabberImpl::s_mapPlugins[".ppm"] = new FileGrabberPluginPNM;  
      FileGrabberImpl::s_mapPlugins[".pgm"] = new FileGrabberPluginPNM; 
      FileGrabberImpl::s_mapPlugins[".pnm"] = new FileGrabberPluginPNM; 
      FileGrabberImpl::s_mapPlugins[".icl"] = new FileGrabberPluginPNM; 
      FileGrabberImpl::s_mapPlugins[".csv"] = new FileGrabberPluginCSV; 

#ifdef HAVE_LIBJPEG
      FileGrabberImpl::s_mapPlugins[".jpg"] = new FileGrabberPluginJPEG; 
      FileGrabberImpl::s_mapPlugins[".jpeg"] = new FileGrabberPluginJPEG; 
#elif HAVE_IMAGEMAGICK
      FileGrabberImpl::s_mapPlugins[".jpg"] = new FileGrabberPluginImageMagick;
      FileGrabberImpl::s_mapPlugins[".jpeg"] = new FileGrabberPluginImageMagick;
#endif

#ifdef HAVE_LIBZ
      FileGrabberImpl::s_mapPlugins[".ppm.gz"] = new FileGrabberPluginPNM; 
      FileGrabberImpl::s_mapPlugins[".pgm.gz"] = new FileGrabberPluginPNM; 
      FileGrabberImpl::s_mapPlugins[".pnm.gz"] = new FileGrabberPluginPNM; 
      FileGrabberImpl::s_mapPlugins[".icl.gz"] = new FileGrabberPluginPNM; 
      FileGrabberImpl::s_mapPlugins[".csv.gz"] = new FileGrabberPluginCSV;       
#endif

#ifdef HAVE_IMAGEMAGICK
      static const char *imageMagickFormats[] = {
        "png","gif","pdf","ps","avs","bmp","cgm","cin","cur","cut","dcx",
        "dib","dng","dot","dpx","emf","epdf","epi","eps","eps2","eps3",
        "epsf","epsi","ept","fax","gplt","gray","hpgl","html","ico","info",
        "jbig","jng","jp2","jpc","man","mat","miff","mono","mng","mpeg","m2v",
        "mpc","msl","mtv","mvg","palm","pbm","pcd","pcds","pcl","pcx","pdb",
        "pfa","pfb","picon","pict","pix","ps","ps2","ps3","psd","ptif","pwp",
        "rad","rgb","pgba","rla","rle","sct","sfw","sgi","shtml","sun","svg",
        "tga","tiff","tim","ttf","txt","uil","uyuv","vicar","viff","wbmp",
        "wmf","wpg","xbm","xcf","xpm","xwd","ydbcr","ycbcra","yuv",0
      };
      
      for(const char **pc=imageMagickFormats;*pc;++pc){
        FileGrabberImpl::s_mapPlugins[std::string(".")+*pc] = new FileGrabberPluginImageMagick;
      }
#endif
      
      

      // add additional plugins to the map
    }
    ~FileGrabberPluginMapInitializer(){
      for(map<string,FileGrabberPlugin*>::iterator it = FileGrabberImpl::s_mapPlugins.begin(); 
          it!= FileGrabberImpl::s_mapPlugins.end(); ++it){
        delete it->second;
      }
    }
  };

  // }}}


  static void init_filegrabber(){
    static FileGrabberPluginMapInitializer i;
  }
  
  FileGrabberImpl::FileGrabberImpl():m_iCurrIdx(0){
    init_filegrabber();
  }
  
  FileGrabberImpl::FileGrabberImpl(const std::string &pattern, 
                           bool buffer, 
                           bool ignoreDesired) throw(FileNotFoundException):
    // {{{ open

    m_oFileList(pattern),
    m_iCurrIdx(0),
    m_bBufferImages(false),
    m_poBufferImage(0){
    
    init_filegrabber();
        
    if(!m_oFileList.size()){
      throw FileNotFoundException(pattern);
    }

    setIgnoreDesiredParams(ignoreDesired);
    
    if(buffer){
      bufferImages(false);
    }    
  }

  // }}}

  FileGrabberImpl::~FileGrabberImpl(){  
    // {{{ open

    ICL_DELETE(m_poBufferImage);
    for(unsigned int i=0;i<m_vecImageBuffer.size();i++){
      ICL_DELETE(m_vecImageBuffer[i]);
    }
  }

  // }}}

  const FileList &FileGrabberImpl::bufferImages(bool omitExceptions){
    // {{{ open

    if(!m_vecImageBuffer.size()){
      vector<string> correctNames;
      m_vecImageBuffer.resize(m_oFileList.size());
      std::fill(m_vecImageBuffer.begin(),m_vecImageBuffer.end(),(ImgBase*)0);
      for(int i=0;i<m_oFileList.size();i++){
        if(omitExceptions){
          try{
            grab(&m_vecImageBuffer[i]);
            correctNames.push_back(m_oFileList[i]);
          }catch(ICLException &ex){
            (void)ex;
          }
        }else{
          grab(&m_vecImageBuffer[i]);
          correctNames.push_back(m_oFileList[i]);
        }
      }
      vector<ImgBase*> buf;
      for(unsigned int i=0;i<m_vecImageBuffer.size();++i){
        if(m_vecImageBuffer[i]){
          buf.push_back(m_vecImageBuffer[i]);
        }
      }
      m_vecImageBuffer = buf;
      m_oFileList = FileList(correctNames);
      if(!buf.size()){
        throw FileNotFoundException("...");
      }
    }
    m_bBufferImages = true;
    return m_oFileList;
  }

  // }}}

  void FileGrabberImpl::next(){
    // {{{ open

    ICLASSERT_RETURN(m_oFileList.size());
    m_iCurrIdx++;
    if(m_iCurrIdx >= m_oFileList.size()) m_iCurrIdx = 0;
  }

  // }}}
  void FileGrabberImpl::prev(){
    // {{{ open

    ICLASSERT_RETURN(m_oFileList.size());
    m_iCurrIdx--;
    if(m_iCurrIdx <= 0) m_iCurrIdx = m_oFileList.size()-1;
  }

  // }}}
  
  unsigned int FileGrabberImpl::getFileCount() const{
    // {{{ open

    return m_oFileList.size();
  }

  // }}}
  
  const std::string &FileGrabberImpl::getNextFileName() const{
    // {{{ open

    return m_oFileList[m_iCurrIdx];
  }

  // }}}
  
  const ImgBase *FileGrabberImpl::grabUD(ImgBase **ppoDst){
    // {{{ open

    if(m_bBufferImages){
      ICLASSERT_RETURN_VAL(m_vecImageBuffer.size(),NULL);
      if(!m_bIgnoreDesiredParams){
        ImgBase* useDestImage = prepareOutput(ppoDst);
        m_oConverter.apply(m_vecImageBuffer[m_iCurrIdx++],useDestImage);
        if(m_iCurrIdx >= (int)m_vecImageBuffer.size()) m_iCurrIdx = 0;
        return useDestImage;
      }else{
        ImgBase *p = m_vecImageBuffer[m_iCurrIdx++];
        if(m_iCurrIdx >= (int)m_vecImageBuffer.size()) m_iCurrIdx = 0;
        return p;
      }
    }
    ICLASSERT_RETURN_VAL(!m_oFileList.isNull(),NULL);
    
    File f(m_oFileList[m_iCurrIdx++]);
    if(!f.exists()) throw FileNotFoundException(f.getName());
    if(m_iCurrIdx >= m_oFileList.size()) m_iCurrIdx = 0;
    
    std::map<string,FileGrabberPlugin*>::iterator it;
    if( m_forcedPluginType == ""){
      it = s_mapPlugins.find(toLower(f.getSuffix()));
    }else{
      it = s_mapPlugins.find("."+toLower(m_forcedPluginType));
    }
    if(it == s_mapPlugins.end()){     
      throw InvalidFileException(string("file type (filename was \"")+f.getName()+"\")");
      return 0;
    }
    if(m_bIgnoreDesiredParams){
      ImgBase** useDestImage = ppoDst ? ppoDst : &m_poBufferImage;
      try{
        it->second->grab(f,useDestImage);
      }catch(ICLException &ex){
        if(f.isOpen()) f.close();
        throw ex;
      }
      if(f.isOpen()) f.close();

      if(!useDestImage){
        throw InvalidFileException(f.getName());
        return 0;
      }
      return *useDestImage;
    }else{
      try{
        it->second->grab(f,&m_poBufferImage);
      }catch(ICLException &ex){
        if(f.isOpen()) f.close();
        throw ex;
      }
      if(f.isOpen()) f.close();
      
      if(!m_poBufferImage){
        throw InvalidFileException(f.getName());
        return 0;
      }

      ImgBase* useDestImage = prepareOutput(ppoDst);

      m_oConverter.apply(m_poBufferImage,useDestImage);  
      
      return useDestImage;        
    }
    

  }

  // }}}

  void FileGrabberImpl::forcePluginType(const std::string &suffix){
    m_forcedPluginType = suffix;
  }  



  void FileGrabberImpl::setProperty(const std::string &property, const std::string &value){
    if(property == "next") { 
      next();
    }else if(property == "prev"){
      prev();
    }else if(property == "jump-to-start"){
      m_iCurrIdx = 0;
    }else{
      ERROR_LOG("property \"" << property << "\" is not available of cannot be set");
    }
  }
  
  std::vector<std::string> FileGrabberImpl::getPropertyList(){
    static const std::string ps[7] = {
      "next","prev","next filename","current filename","jump-to-start","relative progress","absolute progress"
    };
    return std::vector<std::string>(ps,ps+7);
  }
  
  std::string FileGrabberImpl:: getType(const std::string &name){
    if(name == "next" || name == "prev" || name == "jump-to-start"){
      return "command";
    }else if (name == "next filename" || name == "current filename" || name == "relative progress"
              || name == "absolute progress"){
      return "info";
    }else{
      ERROR_LOG("nothing known about property \"" << name << "\"");
      return "undefined";
    }
  }
  
  std::string FileGrabberImpl::getInfo(const std::string &name){
    ERROR_LOG("no info available for info \"" << name << "\"");
    return "undefined";
  }
  
  std::string FileGrabberImpl::getValue(const std::string &name){
    if(name == "next filename"){
      return getNextFileName();
    }else if(name == "current filename"){
      return m_oFileList[iclMax(m_iCurrIdx-1,0)];
    }else if(name == "relative progress"){
      return str((100* (m_iCurrIdx+1)) / float(m_oFileList.size()))+" %";
    }else if(name == "absolute progress"){
      return str(m_iCurrIdx+1) + " / " + str(m_oFileList.size());
    }else{
      ERROR_LOG("no info available for property \"" << name << "\"");
      return "undefined";
    }
  }

  int FileGrabberImpl::isVolatile(const std::string &name){
    if(name == "next" || name == "prev" || name == "jump-to-start"){
      return 0;
    }else if (name == "next filename" || name == "current filename" || name == "relative progress"
              || name == "absolute progress"){
      return 20;
    }else{
      ERROR_LOG("nothing known about property \"" << name << "\"");
      return 0;
    }
  }




}

