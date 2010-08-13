/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/GenericGrabber.cpp                           **
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

#include <ICLIO/GenericGrabber.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/FileGrabber.h>
#include <ICLIO/CreateGrabber.h>
#include <ICLIO/FileList.h>

#ifdef SYSTEM_LINUX
#ifdef HAVE_VIDEODEV
#include <ICLIO/PWCGrabber.h>
#endif
#endif

#ifdef HAVE_LIBDC
#include <ICLIO/DCGrabber.h>
#endif

#ifdef HAVE_UNICAP
#include <ICLIO/UnicapGrabber.h>
#endif

#ifdef HAVE_XCF
#include <ICLIO/XCFPublisherGrabber.h>
#include <ICLIO/XCFServerGrabber.h>
#include <ICLIO/XCFMemoryGrabber.h>
#endif

#ifdef HAVE_LIBMESASR
#include <ICLIO/SwissRangerGrabber.h>
#endif

#ifdef HAVE_MV
#include </MVGrabber.h>
#endif

#ifdef HAVE_XINE
#include <ICLIO/VideoGrabber.h>
#endif

#ifdef HAVE_OPENCV
#include <ICLIO/OpenCVCamGrabber.h>
#endif

#ifdef HAVE_OPENCV2
#include <ICLIO/OpenCVVideoGrabber.h>
#endif

#include <ICLIO/DemoGrabber.h>
#include <ICLUtils/Exception.h>



namespace icl{


  GenericGrabber::GenericGrabber(const std::string &desiredAPIOrder, 
                                 const std::string &params, 
                                 bool notifyErrors) throw(ICLException):m_poGrabber(0){
    init(desiredAPIOrder,params,notifyErrors);
  }

  
  void GenericGrabber::init(const std::string &desiredAPIOrder, 
                                 const std::string &params, 
                                 bool notifyErrors) throw(ICLException){
    Mutex::Locker __lock(m_mutex);
    ICL_DELETE(m_poGrabber);
    m_sType = "";
    std::vector<std::string> lP = tok(params,",");
    
    
    // todo optimize this code using a map or a table or ...
    // std::string pPWC,pDC,pDC800,pUnicap,pFile,pDemo,pCreate,pXCF_P,pXCF_S,pXCF_M,pMV,pSR,pVideo;
    std::map<std::string,std::string> pmap;
    static const int NUM_PLUGINS = 15;
    static const std::string plugins[NUM_PLUGINS] = { "pwc","dc","dc800","unicap","file","demo","create",
                                                      "xcfp","xcfs","xcfm","mv","sr","video","cvvideo", 
                                                      "cvcam" };

    for(unsigned int i=0;i<lP.size();++i){
      for(int j=0;j<NUM_PLUGINS;++j){
        const std::string &D = plugins[j];
        if(lP[i].length() > D.length() && lP[i].substr(0,D.length()) == D){ \
          pmap[D] = lP[i].substr(D.length()+1);                          \
        }  
      }
    }

    std::string errStr;

#define ADD_ERR(P) errStr += errStr.size() ? std::string(",") : ""; \
                   errStr += std::string(P)+"("+pmap[P]+")" 

    std::vector<std::string> l = tok(desiredAPIOrder,",");
    
    for(unsigned int i=0;i<l.size();++i){
#ifdef SYSTEM_LINUX
#ifdef HAVE_VIDEODEV
      if(l[i] == "pwc"){
        PWCGrabber *pwc = new PWCGrabber;
        if(pwc->init(Size(640,480),24,to32s(pmap["pwc"]),true)){
          m_poGrabber = pwc;
          m_sType = "pwc";
          break;
        }else{
          ADD_ERR("pwc");
          delete pwc;
          continue;
        }
      }
#endif
#endif
      
#ifdef HAVE_LIBDC
      if(l[i] == "dc" || l[i] == "dc800"){
        std::vector<DCDevice> devs = DCGrabber::getDeviceList();
        int idx = (l[i]=="dc") ? to32s(pmap["dc"]) : to32s(pmap["dc800"]);

        //printf("index is %d devs size is %d \n",idx,devs.size());
        if(idx < 0) idx = 0;
        if(idx >= (int)devs.size()){
          if(l[i]=="dc"){
            ADD_ERR("dc");
          }else{
            ADD_ERR("dc800");
          }
          continue;
        }else{
          m_poGrabber = new DCGrabber(devs[idx], l[i]=="dc"?400:800);
          m_sType = l[i];
          break;
        }        
      }
#endif

#ifdef HAVE_LIBMESASR
      if(l[i] == "sr"){
        std::vector<std::string> srts = tok(pmap["sr"],"c");
        int device = 0;
        int channel = -1;
        if(srts.size() > 1){
          device = to32s(srts[0]);
          channel = to32s(srts[1]);
        }else{
          device = to32s(srts[0]);
        }

        try{
          m_poGrabber = new SwissRangerGrabber(device,depth32f,channel);
        }catch(ICLException &e){
          ADD_ERR("sr");
          continue;
        }
        break;
      }
#endif

#ifdef HAVE_XINE
      if(l[i] == "video"){
        try{
          m_poGrabber = new VideoGrabber(pmap["video"]);
          m_sType = "video";
        }catch(ICLException &e){
          ADD_ERR("video");
          continue;
        }
        break;
      }
#endif

#ifdef HAVE_UNICAP
      if(l[i] == "unicap"){
        static std::vector<UnicapDevice> devs = UnicapGrabber::getDeviceList(pmap["unicap"]);
        if(!devs.size()){
          ADD_ERR("unicap");
          continue;
        }else{
          m_poGrabber = new UnicapGrabber(devs[0]);
          m_sType = "unicap";
          break;
        }        
      }
#endif


#ifdef HAVE_XCF
      if(l[i].size()==4 && l[i].substr(0,3) == "xcf"){
        switch(l[i][3]){
          case 's':
            try{
              m_poGrabber = new XCFServerGrabber(pmap["xcfs"]);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfs");
              }
            }
            break;
          case 'p':
            try{
              m_poGrabber = new XCFPublisherGrabber(pmap["xcfp"]);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfp");
              }
            }
            break;
          case 'm':
            try{
              m_poGrabber = new XCFMemoryGrabber(pmap["xcfm"]);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfm");
              }
            }
            break;
          default:
            break;
        }
        if(m_poGrabber){
          m_sType = l[i];
          break;
        }else{
          continue;
        }
      }
#endif

#ifdef HAVE_MV
      // not yet supported, and maybe, already replaced by pylon grabber?
      if(l[i] == "mv") {
        std::vector<MVDevice> devs = MVGrabber::getDeviceList();
        
        if(!devs.size()) {
          ADD_ERR("mv");
          continue;
        } else {
          m_poGrabber = new MVGrabber(pmap["mv"]);
          m_sType = "mv";
          break;
        }
      }
#endif


#ifdef HAVE_OPENCV2
      if(l[i] == "cvvideo") {
        try{
          m_poGrabber = new OpenCVVideoGrabber(pmap["cvvideo"]);
          m_sType = "cvvideo";
          break;
        }catch(ICLException &e){
          ADD_ERR("cvvideo");
          continue;
        }
      }
#endif
#ifdef HAVE_OPENCV
      if(l[i] == "cvcam") {
        try{
          m_poGrabber = new OpenCVCamGrabber(to32s(pmap["cvcam"]));
          m_sType = "cvcam";
          break;
        }catch(ICLException &e){
          ADD_ERR("cvcam");
          continue;
        }
      }
      
      
#endif
      
      if(l[i] == "file"){
        try{
          if(FileList(pmap["file"]).size()){
            m_poGrabber = new FileGrabber(pmap["file"]);
            ((FileGrabber*)m_poGrabber)->setIgnoreDesiredParams(false);
            break;
          }else{
            ADD_ERR("file");
            continue;
          }
        }catch(icl::FileNotFoundException &ex){
          ADD_ERR("file");
          continue;
        }
      }
      if(l[i] == "demo"){
        m_poGrabber = new DemoGrabber(to32f(pmap["demo"]));
        m_sType = "demo";
      }

      if(l[i] == "create"){
        m_poGrabber = new CreateGrabber(pmap["create"]);
        m_sType = "create";
      }

    }
    if(!m_poGrabber && notifyErrors){
      std::string errMsg("generic grabber was not able to find any suitable device\ntried:");
      throw ICLException(errMsg+errStr);
    }
  }  
  
  void GenericGrabber::resetBus(const std::string &deviceList, bool verbose){
    std::vector<std::string> ts = tok(deviceList,",");
    
    for(unsigned int i=0;i<ts.size();++i){
      const std::string &t = ts[i];
      (void)t; // to avoid warnings in case of no dc support
#ifdef HAVE_LIBDC
      if(t == "dc" || t == "dc800"){
        DCGrabber::dc1394_reset_bus(verbose);
      }
#endif
      // others are not supported yet
    }
  
  }
}
