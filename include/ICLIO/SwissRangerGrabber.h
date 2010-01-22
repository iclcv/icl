#ifndef ICL_SWISSRANGER_GRABBER_H
#define ICL_SWISSRANGER_GRABBER_H

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  class SwissRangerGrabber : public Grabber{
    public:
    class SwissRanger;
    
    /// if serialNumber < 0 -> open device dialog box
    /// if 0 -> select any device
    SwissRangerGrabber(int serialNumber=0, depth bufferDepth=depth32f, int pickChannel=-1) 
    throw (ICLException);

    ~SwissRangerGrabber();
    
    /// not yet supported ...
    static std::vector<int> getDeviceList();

    const ImgBase *grabUD(ImgBase **dst=0);

    virtual void setProperty(const std::string &property, const std::string &value);
     
    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();

    virtual std::string getType(const std::string &name);

    virtual std::string getInfo(const std::string &name);

    virtual std::string getValue(const std::string &name);
    
    static float getMaxRangeMM(const std::string &modulationFreq) throw (ICLException);


    private:
    /// utility function
    float getMaxRangeVal() const;

    SwissRanger *m_sr;
    Mutex m_mutex;
  };
  
}

#endif
