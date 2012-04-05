/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

#ifdef HAVE_LIBPNG
#include <ICLIO/FileGrabberPluginPNG.h>
#endif

#ifdef HAVE_IMAGEMAGICK
#include <ICLIO/FileGrabberPluginImageMagick.h>
#endif

#include <string>
#include <map>
#include <ICLIO/FileList.h>

namespace icl{

  struct FileGrabberImpl::Data{
    /// internal file list
    FileList oFileList;
    
    /// current file list index
    int iCurrIdx;
    
    /// buffer for buffered mode
    std::vector<ImgBase*> vecImageBuffer;
    
    /// flag whether to pre-buffer images or not
    bool bBufferImages;

    /// indicates whether to jump to next frame automatically
    bool bAutoNext;
    
    /// if true, the grabber grabs all given images in a loop, otherwise it ends with an execption
    bool loop;
    
    /// A special buffer image
    ImgBase *poBufferImage;
    
    /// forced plugin name
    std::string forcedPluginType;
  };
  
  static FileGrabberPlugin *find_plugin(const std::string &type){
    static std::map<std::string,SmartPtr<FileGrabberPlugin> > plugins;
    if(!plugins.size()){
      plugins[".ppm"] = new FileGrabberPluginPNM;  
      plugins[".pgm"] = new FileGrabberPluginPNM; 
      plugins[".pnm"] = new FileGrabberPluginPNM; 
      plugins[".icl"] = new FileGrabberPluginPNM; 
      plugins[".csv"] = new FileGrabberPluginCSV; 

#ifdef HAVE_LIBJPEG
      plugins[".jpg"] = new FileGrabberPluginJPEG; 
      plugins[".jpeg"] = new FileGrabberPluginJPEG; 
#elif HAVE_IMAGEMAGICK
      plugins[".jpg"] = new FileGrabberPluginImageMagick;
      plugins[".jpeg"] = new FileGrabberPluginImageMagick;
#endif

#ifdef HAVE_LIBZ
      plugins[".ppm.gz"] = new FileGrabberPluginPNM; 
      plugins[".pgm.gz"] = new FileGrabberPluginPNM; 
      plugins[".pnm.gz"] = new FileGrabberPluginPNM; 
      plugins[".icl.gz"] = new FileGrabberPluginPNM; 
      plugins[".csv.gz"] = new FileGrabberPluginCSV;       
#endif

#ifdef HAVE_LIBPNG
      plugins[".png"] = new FileGrabberPluginPNG;
#endif

#ifdef HAVE_IMAGEMAGICK
      const char *imageMagickFormats[] = {
#ifndef HAVE_LIBPNG
        "png",
#endif
        "gif","pdf","ps","avs","bmp","cgm","cin","cur","cut","dcx",
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
        plugins[std::string(".")+*pc] = new FileGrabberPluginImageMagick;
      }
#endif
      // add additional plugins to the map
    }
    std::string lowerType = type;
    for(unsigned int i=0;i<lowerType.length();++i){
      lowerType[i] = tolower(lowerType[i]);
    }
    std::map<std::string,SmartPtr<FileGrabberPlugin> >::iterator it = plugins.find(lowerType);
    if(it == plugins.end()) return 0;
    else return it->second.get();
  }
  
  FileGrabberImpl::FileGrabberImpl():m_data(new Data){
    m_data->iCurrIdx  = 0;
    m_data->bBufferImages = false;
    m_data->bAutoNext = true;
    m_data->loop = true;
    m_data->poBufferImage = 0;
  }
  
  FileGrabberImpl::FileGrabberImpl(const std::string &pattern, 
                                   bool buffer, 
                                   bool ignoreDesired) throw(FileNotFoundException):m_data(new Data){
    // {{{ open

    m_data->oFileList = pattern;
    m_data->iCurrIdx  = 0;
    m_data->bBufferImages = false;
    m_data->bAutoNext = true;
    m_data->loop = true;
    m_data->poBufferImage = 0;
    
    m_data->oFileList = FileList(pattern);
    if(!m_data->oFileList.size()){
      throw FileNotFoundException(pattern);
    }
    
    if(buffer){
      bufferImages(false);
    }    
  }

  // }}}

  FileGrabberImpl::~FileGrabberImpl(){  
    // {{{ open

    ICL_DELETE(m_data->poBufferImage);
    for(unsigned int i=0;i<m_data->vecImageBuffer.size();i++){
      ICL_DELETE(m_data->vecImageBuffer[i]);
    }
    delete(m_data);
  }

  // }}}

  void FileGrabberImpl::bufferImages(bool omitExceptions){
    // {{{ open

    if(!m_data->vecImageBuffer.size()){
      std::vector<std::string> correctNames;
      m_data->vecImageBuffer.resize(m_data->oFileList.size());
      std::fill(m_data->vecImageBuffer.begin(),m_data->vecImageBuffer.end(),(ImgBase*)0);
      for(int i=0;i<m_data->oFileList.size();i++){
        if(omitExceptions){
          try{
            grab(&m_data->vecImageBuffer[i]);
            correctNames.push_back(m_data->oFileList[i]);
          }catch(ICLException &ex){
            (void)ex;
          }
        }else{
          grab(&m_data->vecImageBuffer[i]);
          correctNames.push_back(m_data->oFileList[i]);
        }
      }
      std::vector<ImgBase*> buf;
      for(unsigned int i=0;i<m_data->vecImageBuffer.size();++i){
        if(m_data->vecImageBuffer[i]){
          buf.push_back(m_data->vecImageBuffer[i]);
        }
      }
      m_data->vecImageBuffer = buf;
      m_data->oFileList = FileList(correctNames);
      if(!buf.size()){
        throw FileNotFoundException("...");
      }
    }
    m_data->bBufferImages = true;
  }

  // }}}

  void FileGrabberImpl::next(){
    // {{{ open

    ICLASSERT_RETURN(m_data->oFileList.size());
    m_data->iCurrIdx++;
    if(m_data->iCurrIdx >= m_data->oFileList.size()) m_data->iCurrIdx = 0;
  }

  // }}}
  void FileGrabberImpl::prev(){
    // {{{ open

    ICLASSERT_RETURN(m_data->oFileList.size());
    m_data->iCurrIdx--;
    if(m_data->iCurrIdx <= 0) m_data->iCurrIdx = m_data->oFileList.size()-1;
  }

  // }}}
  
  unsigned int FileGrabberImpl::getFileCount() const{
    // {{{ open

    return m_data->oFileList.size();
  }

  // }}}
  
  const std::string &FileGrabberImpl::getNextFileName() const{
    // {{{ open

    return m_data->oFileList[m_data->iCurrIdx];
  }

  // }}}
  
  const ImgBase *FileGrabberImpl::acquireImage(){
    // {{{ open

    if(m_data->bBufferImages){
      ICLASSERT_RETURN_VAL(m_data->vecImageBuffer.size(),NULL);
      ImgBase *p = m_data->vecImageBuffer[m_data->iCurrIdx];
      if(m_data->bAutoNext) ++m_data->iCurrIdx;
      if(m_data->iCurrIdx >= (int)m_data->vecImageBuffer.size()) m_data->iCurrIdx = 0;
      return p;
    }

    ICLASSERT_RETURN_VAL(!m_data->oFileList.isNull(),NULL);
    File f(m_data->oFileList[m_data->iCurrIdx]);
    if(m_data->bAutoNext) ++m_data->iCurrIdx;
    if(!f.exists()) throw FileNotFoundException(f.getName());
    if(m_data->iCurrIdx >= m_data->oFileList.size()){
      if(m_data->loop){
        m_data->iCurrIdx = 0;
      }else{
        throw ICLException("No more files available");
      }
    }
    
    FileGrabberPlugin *p = find_plugin(m_data->forcedPluginType == "" ? f.getSuffix() : m_data->forcedPluginType);
    if(!p){
      throw InvalidFileException(str("file type (filename was \"")+f.getName()+"\")");
      return 0;
    }
    
    try{
      p->grab(f,&m_data->poBufferImage);
    }catch(ICLException&){
      if(f.isOpen()) f.close();
      throw;
    }
    return m_data->poBufferImage;
  }

  // }}}

  void FileGrabberImpl::forcePluginType(const std::string &suffix){
    m_data->forcedPluginType = suffix;
  }  



  void FileGrabberImpl::setProperty(const std::string &property, const std::string &value){
    if(property == "next") { 
      next();
    }else if(property == "prev"){
      prev();
    }else if(property == "loop"){
      m_data->loop = parse<bool>(value);
    }else if(property == "jump-to-start"){
      m_data->iCurrIdx = 0;
    }else if(property == "auto-next"){
      if(value == "on"){
        m_data->bAutoNext = true;
      }else if(value == "off"){
        m_data->bAutoNext = false;
      }else{
        ERROR_LOG("cannot set property \"auto-next\" to \"" 
                  << value 
                  << "\" (allowed values are  \"on\" and \"off\")");
      }
    }else if(property ==  "frame-index"){
      if(m_data->bAutoNext){
        WARNING_LOG("the \"frame-index\" property cannot be set if \"auto-next\" is on");
      }else{
        int idx = parse<int>(value);
        if(idx < 0 || idx >= m_data->oFileList.size()){
          if(idx < 0){
            idx = 0;
          }else{
            idx = m_data->oFileList.size()-1;
          }
          WARNING_LOG("given frame-index was not within the valid range (given value was clipped)");
        }
        m_data->iCurrIdx = parse<int>(value) % (m_data->oFileList.size()-1);
      }
    }else{
      ERROR_LOG("property \"" << property << "\" is not available of cannot be set");
    }
  }
  
  std::vector<std::string> FileGrabberImpl::getPropertyList(){
    static const std::string ps[11] = {
      "next","prev","next filename","current filename","jump-to-start",
      "relative progress","absolute progress","auto-next","loop","file-count","frame-index"
    };
    return std::vector<std::string>(ps,ps+11);
  }
  
  std::string FileGrabberImpl:: getType(const std::string &name){
    if(name == "next" || name == "prev" || name == "jump-to-start"){
      return "command";
    }else if (name == "next filename" || name == "current filename" || name == "relative progress"
              || name == "absolute progress" || name == "file-count"){
      return "info";
    }else if(name == "auto-next" || name == "loop"){
      return "menu";
    }else if(name == "frame-index"){
      return "range";
    }else{
      ERROR_LOG("nothing known about property \"" << name << "\"");
      return "undefined";
    }
  }
  
  std::string FileGrabberImpl::getInfo(const std::string &name){
    if(name == "auto-next" || name == "loop") return "{\"on\",\"off\"}";
    else if(name == "frame-index"){
      return "[0," + str(m_data->oFileList.size()-1) + "]:1";
    }
    ERROR_LOG("no info available for info \"" << name << "\"");
    return "undefined";
  }
  
  std::string FileGrabberImpl::getValue(const std::string &name){
    if(name == "next filename"){
      return getNextFileName();
    }else if(name == "current filename"){
      return m_data->oFileList[iclMax(m_data->iCurrIdx-1,0)];
    }else if(name == "file-count"){
      return str(m_data->oFileList.size());
    }else if(name == "relative progress"){
      return str((100* (m_data->iCurrIdx+1)) / float(m_data->oFileList.size()))+" %";
    }else if(name == "absolute progress"){
      return str(m_data->iCurrIdx+1) + " / " + str(m_data->oFileList.size());
    }else if(name == "auto-next"){
      return m_data->bAutoNext ? "on" : "off";
    }else if(name == "frame-index"){
      return str(m_data->iCurrIdx);
    }else if(name == "loop"){
      return m_data->loop ? "on" : "off";
    }else{
      ERROR_LOG("no value available for property \"" << name << "\"");
      return "undefined";
    }
  }

  int FileGrabberImpl::isVolatile(const std::string &name){
    if(name == "next" || name == "prev" || name == "jump-to-start" || name == "auto-next" || name == "file-count"){
      return 0;
    }else if (name == "next filename" || name == "current filename" || name == "relative progress"
              || name == "absolute progress" || name == "frame-index"){
      return 20;
    }else{
      ERROR_LOG("nothing known about property \"" << name << "\"");
      return 0;
    }
  }




}

