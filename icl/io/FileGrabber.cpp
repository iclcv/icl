// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <string>
#include <map>
#include <icl/io/FileGrabber.h>
#include <icl/io/FileList.h>
#include <icl/io/FilenameGenerator.h>
#include <icl/utils/Exception.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Thread.h>
#include <icl/utils/File.h>
// plugins
#include <icl/io/FileGrabberPluginPNM.h>
#include <icl/io/FileGrabberPluginBICL.h>
#include <icl/io/FileGrabberPluginCSV.h>

#ifdef ICL_HAVE_LIBJPEG
#include <icl/io/FileGrabberPluginJPEG.h>
#endif

#ifdef ICL_HAVE_LIBPNG
#include <icl/io/FileGrabberPluginPNG.h>
#endif

#ifdef ICL_HAVE_IMAGEMAGICK
#include <icl/io/FileGrabberPluginImageMagick.h>
#endif

#include <string>
#include <map>
#include <icl/io/FileList.h>

#if defined(ICL_SYSTEM_WINDOWS) && defined(ICL_HAVE_QT)
#include <QtCore/QDir>
#include <mutex>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  namespace{
    struct FileListEndedException : public ICLException{
      inline FileListEndedException(const std::string &what):ICLException(what){}
    };
  }

  struct FileGrabber::Data{
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

      /// for time stamp based image acquisition
      bool useTimeStamps;

      /// also for time stamp based image acquisition
      Time referenceTime;

      /// also for time stamp based image acquisition
      Time referenceTimeReal;

  };

  // Plugins self-register via REGISTER_FILE_GRABBER_PLUGIN at static-init
  // time. Each registered callable carries its own state; no external cache.

  FileGrabberRegistry& fileGrabberRegistry() {
    static FileGrabberRegistry inst(utils::OnDuplicate::KeepHighestPriority);
    return inst;
  }

  static const FileGrabberFn *find_plugin(const std::string &type){
    std::string lowerType = type;
    for (unsigned int i = 0; i < lowerType.length(); ++i) {
      lowerType[i] = tolower(lowerType[i]);
    }
    const auto *e = fileGrabberRegistry().get(lowerType);
    return e ? &e->payload : nullptr;
  }

  FileGrabber::FileGrabber()
    :  m_data(new Data), m_propertyMutex(), m_updatingProperties(false)
  {
    m_data->iCurrIdx  = 0;
    m_data->bBufferImages = false;
    m_data->bAutoNext = true;
    m_data->loop = true;
    m_data->poBufferImage = 0;
    m_data->useTimeStamps = false;
    addProperties();
  }

  FileGrabber::FileGrabber(const std::string &pattern,
                                   bool buffer,
                                   bool ignoreDesired)
    : m_data(new Data), m_propertyMutex(), m_updatingProperties(false)
  {

    if(File(pattern).isDirectory()){
#if defined(ICL_SYSTEM_WINDOWS) && defined(ICL_HAVE_QT)
		  std::vector<std::string> files;
		 QDir dir(pattern.c_str());
		 QStringList es = dir.entryList();
		 for (int i = 0; i < es.size(); ++i){
			 std::string ei = es[i].toLatin1().data();
			 if (!File(ei).isDirectory()){
				 files.push_back(ei);
			 }
		 }
		 m_data->oFileList = FileList(files);
#else
      m_data->oFileList = pattern+"/*";
#endif
      if(!m_data->oFileList.size()){
        throw FileNotFoundException(pattern);
      }
      std::vector<std::string> readable;
      for(int i=0;i<m_data->oFileList.size();++i){
        File f(m_data->oFileList[i]);
        if(find_plugin(f.getSuffix())){
          readable.push_back(m_data->oFileList[i]);
        }
      }
      if(readable.size()){
        m_data->oFileList = FileList(readable);
      }else{
        throw FileNotFoundException("didn't find any file whose format is supported");
      }
    }else{
#ifdef ICL_SYSTEM_WINDOWS
		  if (!File(pattern).exists()) {
			  throw ICLException("Error: file globbing is thus far not supported under windows. You can, however use a directory-name if possible");
		  }
#endif
        m_data->oFileList = pattern;
        if(!m_data->oFileList.size()){
          throw FileNotFoundException(pattern);
        }
      }

      m_data->iCurrIdx  = 0;
      m_data->bBufferImages = false;
      m_data->bAutoNext = true;
      m_data->loop = true;
      m_data->poBufferImage = 0;
      m_data->useTimeStamps = false;


      if(buffer){
        bufferImages(false);
      }
      addProperties();
    }


    FileGrabber::~FileGrabber(){

      ICL_DELETE(m_data->poBufferImage);
      for(unsigned int i=0;i<m_data->vecImageBuffer.size();i++){
        ICL_DELETE(m_data->vecImageBuffer[i]);
      }
      delete(m_data);
    }


    void FileGrabber::bufferImages(bool omitExceptions){

      if(!m_data->vecImageBuffer.size()){
        std::vector<std::string> correctNames;
        m_data->vecImageBuffer.resize(m_data->oFileList.size());
        std::fill(m_data->vecImageBuffer.begin(),m_data->vecImageBuffer.end(),(ImgBase*)0);
        for(int i=0;i<m_data->oFileList.size();i++){
          if(omitExceptions){
            try{
              grab(&m_data->vecImageBuffer[i]);
              correctNames.push_back(m_data->oFileList[i]);
            }catch([[maybe_unused]] ICLException &ex){
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


    void FileGrabber::next(){

      ICLASSERT_RETURN(m_data->oFileList.size());
      m_data->iCurrIdx++;
      if(m_data->iCurrIdx >= m_data->oFileList.size()) m_data->iCurrIdx = 0;
    }

    void FileGrabber::prev(){

      ICLASSERT_RETURN(m_data->oFileList.size());
      m_data->iCurrIdx--;
      if(m_data->iCurrIdx < 0) m_data->iCurrIdx = m_data->oFileList.size()-1;
    }


    unsigned int FileGrabber::getFileCount() const{

      return m_data->oFileList.size();
    }


    const std::string &FileGrabber::getNextFileName() const{
      static const std::string myNull("null");
      return ( m_data->iCurrIdx >= m_data->oFileList.size()
               ? (m_data->loop ? m_data->oFileList[0] : myNull)
               : m_data->oFileList[m_data->iCurrIdx] );
    }


    const ImgBase *FileGrabber::acquireDisplay(){
      try{
        const ImgBase* img = grabDisplay();
        updateProperties(img);

        std::string print = getPropertyValue("print meta-data");
        if(print != "disregard"){
          if(print == "to std::out"){
            std::cout << "image meta data: [" << img->getMetaData() << "]" << std::endl;
          }else if(print == "to meta-data label"){
            setPropertyValue("meta-data", img->getMetaData());
          }
        }

        return img;
      } catch(FileListEndedException &ex){
        throw;
      } catch (ICLException &e){
        DEBUG_LOG("could not grab image. Name: "
                  << m_data->oFileList[iclMax(m_data->iCurrIdx-1,0)]
                  << " Error: " << e.what());
        return nullptr;
      }
    }

    const core::ImgBase *FileGrabber::grabDisplay(){
      if(m_data->bBufferImages){
        if(m_data->useTimeStamps) {
          ERROR_LOG("buffering images and using timestamps cannot be used in parallel! (deactivating use of timestamps)");
          m_data->useTimeStamps = false;
        }
        ICLASSERT_RETURN_VAL(m_data->vecImageBuffer.size(),nullptr);
        ImgBase *p = m_data->vecImageBuffer[m_data->iCurrIdx];
        if(m_data->bAutoNext) ++m_data->iCurrIdx;
        if(m_data->iCurrIdx >= static_cast<int>(m_data->vecImageBuffer.size())) m_data->iCurrIdx = 0;
        return p;
      }

      ICLASSERT_RETURN_VAL(!m_data->oFileList.isNull(),nullptr);

      if(m_data->iCurrIdx >= m_data->oFileList.size()){
        if(m_data->loop){
          m_data->iCurrIdx = 0;
        }else{
          throw FileListEndedException("No more files available");
        }
      }
      //DEBUG_LOG("creating file with index " << m_data->iCurrIdx);
      File f(m_data->oFileList[m_data->iCurrIdx]);
      if(!f.exists()) throw FileNotFoundException(f.getName());
      if(m_data->bAutoNext){
        ++m_data->iCurrIdx;
        //DEBUG_LOG("updating curr idx to " << m_data->iCurrIdx);
      }

      const auto *fn = find_plugin(m_data->forcedPluginType == "" ? f.getSuffix() : m_data->forcedPluginType);
      if(!fn){
        throw InvalidFileException(str("file type (filename was \"")+f.getName()+"\")");
        return 0;
      }

      try{
        (*fn)(f,&m_data->poBufferImage);
      }catch(ICLException&){
        if(f.isOpen()) f.close();
        throw;
      }

      if(m_data->useTimeStamps){
        Time now = Time::now();
        Time &ref = m_data->referenceTime;
        Time &refReal = m_data->referenceTimeReal;
        Time t = m_data->poBufferImage->getTime();
        if(t == Time(0)){
          ERROR_LOG("property 'use-time-stamps' activated, but image with time-stamp '0' found (deactivating 'use-time-stamps')");
          m_data->useTimeStamps = false;
          return m_data->poBufferImage;
        }
        if(ref == Time(0) || m_data->iCurrIdx == 1){
          ref = t;
          refReal = now;
        }else{
          Time desiredDT = t - ref;
          Time currentDT = now - refReal;
          if(desiredDT > currentDT){
            Time ddt = desiredDT - currentDT;
            Thread::usleep(ddt.toMicroSeconds());
          }else{
            static bool first = true;
            if(first && m_data->iCurrIdx != 2){ // hack!!
              first = false;
              WARNING_LOG("property 'use-time-stamps' is activated, but the processing framerate\n"
                          "    is slower than the captured image framerate (this message is only shown once)");
            }
          }
        }
      }

      return m_data->poBufferImage;
    }


    void FileGrabber::forcePluginType(const std::string &suffix){
      m_data->forcedPluginType = suffix;
    }

    void FileGrabber::addProperties(){
      addProperty("format","info","","unknown",0,"");
      addProperty("size","info","","unknown",0,"");
      addProperty("next","command","",Any(),0,"Increments the file counter for the grabber");
      addProperty("prev","command","",Any(),0,"Decrements the file counter for the grabber");
      addProperty("use-time-stamps","flag","",m_data->useTimeStamps,0,"Whether to use timestamps"); //TODO: what is this?
      addProperty("next filename","info","",getNextFileName(),0,"Name of the next file to grab");
      addProperty("current filename","info","",m_data->oFileList[iclMax(m_data->iCurrIdx-1,0)],0,"Name of the last grabbed file");
      addProperty("jump-to-start","command","",Any(),0,"Reset the file counter to 0");
      addProperty("relative progress","info","",str((100* (m_data->iCurrIdx+1)) / float(m_data->oFileList.size()))+" %",0,"The relative progress through the files in percent");
      addProperty("absolute progress","info","",str(m_data->iCurrIdx+1) + " / " + str(m_data->oFileList.size()),0,"The absolute progress through the files. 'current nunmber/total number'");
      addProperty("auto-next","flag","",m_data->bAutoNext,0,"Whether to automatically grab the next file for every frame");
      addProperty("loop","flag","",m_data->loop,0,"Whether to reset the file counter to zero after reaching the last");
      addProperty("file-count","info","",str(m_data->oFileList.size()),0,"Total count of files the grabber will show");
      //addProperty("frame-index","range","[0," + str(m_data->oFileList.size()-1) + "]1",m_data->iCurrIdx,20,"Currently grabbed frame");
      addProperty("frame-index","range:spinbox","[0," + str(m_data->oFileList.size()-1) + "]",m_data->iCurrIdx,20,"Currently grabbed frame");
      addProperty("print meta-data","menu","disregard,to std::out,to meta-data label","disregard");
      addProperty("meta-data","info","","",0,"current image meta-data. Depends on mode set in print meta-data.");

      registerCallback([this](const utils::Configurable::Property &p){ processPropertyChange(p); });
    }

    void FileGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      std::scoped_lock<std::recursive_mutex> l(m_propertyMutex);
      if (m_updatingProperties) return;
      if(prop.name == "next") {
        next();
      }else if(prop.name == "prev"){
        prev();
      }else if(prop.name == "loop"){
        m_data->loop = parse<bool>(prop.value);
      }else if(prop.name == "use-time-stamps"){
        bool val = parse<bool>(prop.value);
        if(val != m_data->useTimeStamps){
          m_data->useTimeStamps = val;
          m_data->referenceTime = Time(0);
          m_data->referenceTimeReal = Time(0);
        }
      }else if(prop.name == "jump-to-start"){
        m_data->iCurrIdx = 0;
      }else if(prop.name == "auto-next"){
        m_data->bAutoNext = parse<bool>(prop.value);
      }else if(prop.name ==  "frame-index"){
        if(m_data->bAutoNext){
          WARNING_LOG("the \"frame-index\" property cannot be set if \"auto-next\" is on");
        }else{
          int idx = parse<int>(prop.value);
          if(idx < 0 || idx >= m_data->oFileList.size()){
            if(idx < 0){
              idx = 0;
            }else{
              idx = m_data->oFileList.size()-1;
            }
            WARNING_LOG("given frame-index was not within the valid range (given value was clipped)");
          }
          m_data->iCurrIdx = parse<int>(prop.value) % (m_data->oFileList.size()-1);
          Thread::sleep(0.2);
        }
      }else{
        ERROR_LOG("property \"" << prop.name << "\" is not available of cannot be set");
      }
    }

    void FileGrabber::updateProperties(const ImgBase* img){
      std::scoped_lock<std::recursive_mutex> l(m_propertyMutex);
      m_updatingProperties = true;
      int s = m_data->oFileList.size();
      int usedIdx = m_data->iCurrIdx - (m_data->bAutoNext ? 1 : 0);
      if(usedIdx < 0) usedIdx = s-1;

      //DEBUG_LOG("in update properties: use idx = " << usedIdx);
      //std::cout << "--" << std::endl;
      setPropertyValue("next filename", m_data->oFileList[usedIdx == s-1 ? 0 : usedIdx+1]);
      setPropertyValue("current filename", m_data->oFileList[usedIdx]);
      setPropertyValue("relative progress", str((100* (usedIdx+1)) / float(s))+" %");
      setPropertyValue("absolute progress", str(usedIdx+1) + " / " + str(s));
      setPropertyValue("format", Any(img -> getFormat()));
      setPropertyValue("size", Any(img -> getSize()));
      //setPropertyValue("frame-index", m_data->iCurrIdx);
      m_updatingProperties = false;
    }

    REGISTER_CONFIGURABLE(FileGrabber, return new FileGrabber("*", false, false));

    Grabber* createGrabber(const std::string &param){
      if(FileList(param).size()){
        return new FileGrabber(param);
      }else{
        std::ostringstream error;
        error << "Could not create a file list from '" << param << "'";
        throw ICLException(error.str());
      }
    }

    const std::vector<GrabberDeviceDescription>& getFileDeviceList(std::string filter, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      // if filter exists, add grabber with filter
      if(filter.size()) deviceList.push_back(
        GrabberDeviceDescription("file", filter, "A grabber for image files.")
        );
      return deviceList;
    }

    REGISTER_GRABBER(file,createGrabber,getFileDeviceList,"file:file name or file-pattern (in ''):image source for single or a list of image files");

  } // namespace icl::io