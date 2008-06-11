#include <iclGenericGrabber.h>
#include <iclStringUtils.h>
#include <iclFileGrabber.h>
#include <iclFileList.h>

#ifdef LINUX
#include <iclPWCGrabber.h>
#endif

#ifdef WITH_LIBDC_SUPPORT
#include <iclDCGrabber.h>
#endif

#ifdef WITH_UNICAP_SUPPORT
#include <iclUnicapGrabber.h>
#endif



namespace icl{
  GenericGrabber::GenericGrabber(const std::string &desiredAPIOrder, const std::string &params, bool notifyErrors){

    m_poGrabber = 0;
    std::vector<std::string> lP = tok(params,",");
    std::string pPWC,pDC,pUnicap,pFile;

    for(unsigned int i=0;i<lP.size();++i){
      if(lP[i].length() > 4 && lP[i].substr(0,3) == "pwc"){
        pPWC = lP[i].substr(4);
      }else if(lP[i].length() > 3 && lP[i].substr(0,2) == "dc"){
        pDC = lP[i].substr(3);
      }else if(lP[i].length() > 7 && lP[i].substr(0,6) == "unicap"){
        pUnicap = lP[i].substr(7);
      }else if(lP[i].length() > 5 && lP[i].substr(0,4) == "file"){
        pFile = lP[i].substr(5);
      }

      
    }

    std::vector<std::string> l = tok(desiredAPIOrder,",");
    
    for(unsigned int i=0;i<l.size();++i){
#ifdef LINUX
      if(l[i] == "pwc"){
        PWCGrabber *pwc = new PWCGrabber;
        if(pwc->init(Size(640,480),24,to32s(pPWC))){
          m_poGrabber = pwc;
          m_sType = "pwc";
          break;
        }else{
          delete pwc;
          continue;
        }
      }
#endif
      
#ifdef WITH_LIBDC_SUPPORT
      if(l[i] == "dc"){
        std::vector<DCDevice> devs = DCGrabber::getDeviceList();
        int idx = to32s(pDC);
        printf("index is %d devs size is %d \n",idx,devs.size());
        if(idx < 0) idx = 0;
        if(idx >= (int)devs.size()){
          continue;
        }else{
          m_poGrabber = new DCGrabber(devs[idx]);
          m_sType = "dc";
          break;
        }        
      }
#endif

#ifdef WITH_UNICAP_SUPPORT
      if(l[i] == "unicap"){
        static std::vector<UnicapDevice> devs = UnicapGrabber::getDeviceList(pUnicap);
        if(!devs.size()){
          continue;
        }else{
          m_poGrabber = new UnicapGrabber(devs[0]);
          m_sType = "unicap";
          break;
        }        
      }
#endif
      
      if(l[i] == "file"){
        if(FileList(pFile).size()){
          m_poGrabber = new FileGrabber(pFile);
          ((FileGrabber*)m_poGrabber)->setIgnoreDesiredParams(false);
          m_sType = "file";
          break;
        }else{
          continue;
        }
      }
    }
    if(!m_poGrabber && notifyErrors){
      ERROR_LOG("Generic Grabber was not able to find any suitable device!");
    }
  }  
}
