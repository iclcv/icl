#include <ICLIO/GenericGrabber.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/FileGrabber.h>
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

#include <ICLIO/DemoGrabber.h>
#include <ICLUtils/Exception.h>



namespace icl{
  
  GenericGrabber::GenericGrabber(const std::string &desiredAPIOrder, 
                                 const std::string &params, 
                                 bool notifyErrors) throw(ICLException){

    m_poGrabber = 0;
    std::vector<std::string> lP = tok(params,",");
    
    // todo optimize this code using a map or a table or ...
    std::string pPWC,pDC,pDC800,pUnicap,pFile,pDemo,pXCF_P,pXCF_S,pXCF_M,pMV,pSR,pVideo;

#define PARAM(D,PNAME)                                                  \
    if(lP[i].length() > strlen(D) && lP[i].substr(0,strlen(D)) == D){   \
      PNAME = lP[i].substr(strlen(D)+1);                                \
    }  
    for(unsigned int i=0;i<lP.size();++i){
      //      DEBUG_LOG("lP[" << i << "]" << lP[i]);
      if(false){}
      PARAM("pwc",pPWC); 
      PARAM("dc",pDC); 
      PARAM("dc800",pDC800);
      PARAM("unicap",pUnicap); 
      PARAM("file",pFile);
      PARAM("demo",pDemo);
      PARAM("xcfp",pXCF_P);
      PARAM("xcfs",pXCF_S);
      PARAM("xcfm",pXCF_M);
      PARAM("mv",pMV);
      PARAM("sr",pSR);
      PARAM("video",pVideo);
#undef PARAM
      /*
          if(lP[i].length() > 4 && lP[i].substr(0,3) == "pwc"){
          pPWC = lP[i].substr(4);
          }else if(lP[i].length() > 3 && lP[i].substr(0,2) == "dc"){
          pDC = lP[i].substr(3);
          }else if(lP[i].length() > 6 && lP[i].substr(0,5) == "dc800"){
          pDC800 = lP[i].substr(6);
          }else if(lP[i].length() > 7 && lP[i].substr(0,6) == "unicap"){
          pUnicap = lP[i].substr(7);
          }else if(lP[i].length() > 5 && lP[i].substr(0,4) == "file"){
          pFile = lP[i].substr(5);
          }else if(lP[i].length() > 5 && lP[i].substr(0,4) == "demo"){
          pDemo = lP[i].substr(5);
          }else if(lP[i].length() > 5 && lP[i].substr(0,4) == "xcfp"){
          pXCF_P = lP[i].substr(5);
          }else if(lP[i].length() > 5 && lP[i].substr(0,4) == "xcfs"){
          pXCF_S = lP[i].substr(5);
          }else if(lP[i].length() > 5 && lP[i].substr(0,4) == "xcfm"){
          pXCF_M = lP[i].substr(5);
          }
     */

    }

    std::string errStr;

#define ADD_ERR(X,A) errStr += errStr.size() ? std::string(",") : ""; \
                     errStr += std::string(#X)+"("+A+")" 

    std::vector<std::string> l = tok(desiredAPIOrder,",");
    
    for(unsigned int i=0;i<l.size();++i){
#ifdef SYSTEM_LINUX
#ifdef HAVE_VIDEODEV
      if(l[i] == "pwc"){
        PWCGrabber *pwc = new PWCGrabber;
        if(pwc->init(Size(640,480),24,to32s(pPWC),true)){
          m_poGrabber = pwc;
          m_sType = "pwc";
          break;
        }else{
          ADD_ERR(pwc,pPWC);
          delete pwc;
          continue;
        }
      }
#endif
#endif
      
#ifdef HAVE_LIBDC
      if(l[i] == "dc" || l[i] == "dc800"){
        std::vector<DCDevice> devs = DCGrabber::getDeviceList();
        int idx = (l[i]=="dc") ? to32s(pDC) : to32s(pDC800);

        //printf("index is %d devs size is %d \n",idx,devs.size());
        if(idx < 0) idx = 0;
        if(idx >= (int)devs.size()){
          if(l[i]=="dc"){
            ADD_ERR(dc,pDC);
          }else{
            ADD_ERR(dc800,pDC800);
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
        std::vector<std::string> srts = tok(pSR,"c");
        int device = 0;
        int channel = -1;
        if(srts.size() > 1){
          device = to32s(srts[0]);
          channel = to32s(srts[1]);
          DEBUG_LOG("using pick channel channel");
        }else{
          device = to32s(srts[0]);
        }

        try{
          m_poGrabber = new SwissRangerGrabber(device,depth32f,channel);
        }catch(ICLException &e){
          ADD_ERR(sr,pSR);
          continue;
        }
        break;
      }
#endif

#ifdef HAVE_XINE
      if(l[i] == "video"){
        try{
          m_poGrabber = new VideoGrabber(pVideo);
        }catch(ICLException &e){
          ADD_ERR(video,pVideo);
          continue;
        }
        break;
      }
#endif

#ifdef HAVE_UNICAP
      if(l[i] == "unicap"){
        static std::vector<UnicapDevice> devs = UnicapGrabber::getDeviceList(pUnicap);
        if(!devs.size()){
          ADD_ERR(unicap,pUnicap);
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
              m_poGrabber = new XCFServerGrabber(pXCF_S);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR(xcf-server,pXCF_S);
              }
            }
            break;
          case 'p':
            try{
              m_poGrabber = new XCFPublisherGrabber(pXCF_P);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR(xcf-publisher,pXCF_P);
              }
            }
            break;
          case 'm':
            try{
              m_poGrabber = new XCFMemoryGrabber(pXCF_M);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR(xcf-memory,pXCF_M);
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
      if(l[i] == "mv") {
        std::vector<MVDevice> devs = MVGrabber::getDeviceList();
        
        if(!devs.size()) {
          ADD_ERR(mv,pMV);
          continue;
        } else {
          m_poGrabber = new MVGrabber();
          m_sType = "mv";
          break;
        }
      }
#endif
      
      if(l[i] == "file"){
        try{
          if(FileList(pFile).size()){
            m_poGrabber = new FileGrabber(pFile);
            ((FileGrabber*)m_poGrabber)->setIgnoreDesiredParams(false);
            break;
          }else{
            ADD_ERR(file,pFile);
            continue;
          }
        }catch(icl::FileNotFoundException &ex){
          ADD_ERR(file,pFile);
          continue;
        }
      }
      if(l[i] == "demo"){
        m_poGrabber = new DemoGrabber(to32s(pDemo));
        m_sType = "demo";
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
