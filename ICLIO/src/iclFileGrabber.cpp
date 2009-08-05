#include <string>
#include <map>
#include <iclFileGrabber.h>
#include <iclFileList.h>
#include <iclFilenameGenerator.h>
#include <iclException.h>
#include <iclIOUtils.h>
#include <iclStringUtils.h>
// plugins
#include <iclFileGrabberPluginPNM.h>
#include <iclFileGrabberPluginCSV.h>

#ifdef HAVE_LIBJPEG
#include <iclFileGrabberPluginJPEG.h>
#endif

#ifdef HAVE_IMAGEMAGICK
#include <iclFileGrabberPluginImageMagick.h>
#endif
using namespace std;

namespace icl{

  map<string,FileGrabberPlugin*> FileGrabber::s_mapPlugins;

  struct FileGrabberPluginMapInitializer{
    // {{{ open PLUGINS ARE INCLUDED HERE

    FileGrabberPluginMapInitializer(){
      FileGrabber::s_mapPlugins[".ppm"] = new FileGrabberPluginPNM;  
      FileGrabber::s_mapPlugins[".pgm"] = new FileGrabberPluginPNM; 
      FileGrabber::s_mapPlugins[".pnm"] = new FileGrabberPluginPNM; 
      FileGrabber::s_mapPlugins[".icl"] = new FileGrabberPluginPNM; 
      FileGrabber::s_mapPlugins[".csv"] = new FileGrabberPluginCSV; 

#ifdef HAVE_LIBJPEG
      FileGrabber::s_mapPlugins[".jpg"] = new FileGrabberPluginJPEG; 
      FileGrabber::s_mapPlugins[".jpeg"] = new FileGrabberPluginJPEG; 
#elif HAVE_IMAGEMAGICK
      FileGrabber::s_mapPlugins[".jpg"] = new FileGrabberPluginImageMagick;
      FileGrabber::s_mapPlugins[".jpeg"] = new FileGrabberPluginImageMagick;
#endif

#ifdef HAVE_LIBZ
      FileGrabber::s_mapPlugins[".ppm.gz"] = new FileGrabberPluginPNM; 
      FileGrabber::s_mapPlugins[".pgm.gz"] = new FileGrabberPluginPNM; 
      FileGrabber::s_mapPlugins[".pnm.gz"] = new FileGrabberPluginPNM; 
      FileGrabber::s_mapPlugins[".icl.gz"] = new FileGrabberPluginPNM; 
      FileGrabber::s_mapPlugins[".csv.gz"] = new FileGrabberPluginCSV;       
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
        FileGrabber::s_mapPlugins[std::string(".")+*pc] = new FileGrabberPluginImageMagick;
      }
#endif
      
      

      // add additional plugins to the map
    }
    ~FileGrabberPluginMapInitializer(){
      for(map<string,FileGrabberPlugin*>::iterator it = FileGrabber::s_mapPlugins.begin(); 
          it!= FileGrabber::s_mapPlugins.end(); ++it){
        delete it->second;
      }
    }
  };

  // }}}

  static FileGrabberPluginMapInitializer ___filegrabber_plugin_map_initializer__;
  
  
  FileGrabber::FileGrabber():m_iCurrIdx(0){}
  
  FileGrabber::FileGrabber(const std::string &pattern, 
                           bool buffer, 
                           bool ignoreDesired) throw(FileNotFoundException):
    // {{{ open

    m_oFileList(pattern),
    m_iCurrIdx(0),
    m_bBufferImages(false),
    m_poBufferImage(0){
    
    if(!m_oFileList.size()){
      throw FileNotFoundException(pattern);
    }

    setIgnoreDesiredParams(ignoreDesired);
    
    if(buffer){
      bufferImages(false);
    }    
  }

  // }}}

  FileGrabber::~FileGrabber(){  
    // {{{ open

    ICL_DELETE(m_poBufferImage);
    for(unsigned int i=0;i<m_vecImageBuffer.size();i++){
      ICL_DELETE(m_vecImageBuffer[i]);
    }
  }

  // }}}

  const FileList &FileGrabber::bufferImages(bool omitExceptions){
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

  void FileGrabber::next(){
    // {{{ open

    ICLASSERT_RETURN(m_oFileList.size());
    m_iCurrIdx++;
    if(m_iCurrIdx >= m_oFileList.size()) m_iCurrIdx = 0;
  }

  // }}}
  void FileGrabber::prev(){
    // {{{ open

    ICLASSERT_RETURN(m_oFileList.size());
    m_iCurrIdx--;
    if(m_iCurrIdx <= 0) m_iCurrIdx = m_oFileList.size()-1;
  }

  // }}}
  
  unsigned int FileGrabber::getFileCount() const{
    // {{{ open

    return m_oFileList.size();
  }

  // }}}
  
  const std::string &FileGrabber::getNextFileName() const{
    // {{{ open

    return m_oFileList[m_iCurrIdx];
  }

  // }}}
  
  const ImgBase *FileGrabber::grabUD(ImgBase **ppoDst){
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
    
    std::map<string,FileGrabberPlugin*>::iterator it = s_mapPlugins.find(toLower(f.getSuffix()));
    if(it == s_mapPlugins.end()){     
      throw InvalidFileException(string("file type \"*")+f.getSuffix()+"\"");
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
  
}

