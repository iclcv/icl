#ifndef ICL_GENERIC_GRABBER_H
#define ICL_GENERIC_GRABBER_H

#include <ICLIO/Grabber.h>
#include <string>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Exception.h>

namespace icl {

  /// Common interface class for all grabbers \ingroup GRABBER_G
  /** The generic grabber provides an interface for a multiplatform
      compatible grabber. */
  class GenericGrabber: public Grabber{
    
    Grabber *m_poGrabber;
    
    std::string m_sType;

    public:
    
    /// Create a generic grabber instance with given device priority list
    /** @param devicePriorityList Comma separated list of device tokens (no white spaces).
                                  something like "dc,pwc,file,unicap" with arbitrary order
                                  undesired devices can be left out. In particular you can 
                                  also give only a single desired device type e.g. "pwc".
                                  The following device types are supported:
                                  - <b>pwc</b> pwc grabber 
                                  - <b>dc</b> dc grabber 
                                  - <b>dc800</b> dc grabber but with 800MBit iso-speed 
                                  - <b>unicap</b> unicap grabber 
                                  - <b>file</b> file grabber
                                  - <b>demo</b> demo grabber (moving red spot)
                                  - <b>xcfp</b> xcf publisher grabber
                                  - <b>xcfs</b> xcf server grabber
                                  - <b>xcfm</b> xcf memory grabber
                                  - <b>mv</b> matrix vision grabber
                                  - <b>sr</b> SwissRanger camera (mesa-imaging)
                                  - <b>video</b> Xine based video grabber (grabbing videos frame by frame)
        
        @param params comma separated device depend parameter list: e.g.
                                  "pwc=0,file=images//image*.ppm,dc=0" with self-explaining syntax\n
                                  Semantics:\n
                                  - pwc=device-index (int)
                                  - dc=device-index (int)
                                  - dc800=device-index (int)
                                  - file=pattern (string)
                                  - unicap=device pattern (string)
                                  - demo=anything (not regarded)
                                  - xcfp=publisher's-stream-name (string)
                                  - xcfs=server-name (string) (currently method name is always "retreiveImage")
                                  - xcfm=memory-name (string) (currently image-xpath is always "//IMAGE")
                                  - mv=device-name (string)
                                  - sr=device-serial-number (-1 -> menu, 0 -> auto-select)
                                    <b>or</b>
                                    sr=NcC where N is the device numer as above, c is the character 'c' and C is
                                    the channel index to pick (0: depth-map, 1: confidence map, 2: intensity image
                                  - video=video-filename (string)
        @param notifiyErrors if set to false, no exception is thrown if no suitable device was found

        @suppressDoubledImages if set, this flag makes the wrapped grabber return each
                               video frame only once. Currently this feature is only supported
                               for the DCGrabber
    */
    GenericGrabber(const std::string &devicePriorityList="dc,pwc,file", 
                   const std::string &params="pwc=0,dc=0,file=images/*.ppm",
                   bool notifyErrors = true) throw (ICLException);
    
    /// resets resource on given devices (e.g. firewire bus)
    static void resetBus(const std::string &deviceList="dc", bool verbose=false);
   
    /// return the actual grabber type
    std::string getType() const { 
      return m_sType; 
    }
    
    /// returns the wrapped grabber itself
    Grabber *getGrabber() const { 
      return m_poGrabber; 
    }
    
    /// Destructor
    virtual ~GenericGrabber(){
      ICL_DELETE(m_poGrabber);
    }
    
    /// grabbing function
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0){
      ICLASSERT_RETURN_VAL(!isNull(),0);
      return m_poGrabber->grabUD(ppoDst);
    }

    /// returns a list of all properties, that can be set
    virtual std::vector<std::string> getPropertyList(){
      ICLASSERT_RETURN_VAL(!isNull(),std::vector<std::string>());
      return m_poGrabber->getPropertyList();
    }

    /// setting up properties of underlying grabber
    virtual void setProperty(const std::string &property, const std::string &value){
      ICLASSERT_RETURN(!isNull());
      m_poGrabber->setProperty(property,value);
    }

    /// returns whether property is supported by underlying grabber
    virtual bool supportsProperty(const std::string &property){
      ICLASSERT_RETURN_VAL(!isNull(),false);
      return m_poGrabber->supportsProperty(property);
    }
    
    /// returns the property type of given property
    virtual std::string getType(const std::string &name){
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getType(name);
    }
     
    /// retuns property information
    virtual std::string getInfo(const std::string &name){
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getInfo(name);
    }
    
    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name){
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getValue(name);
    }

    /// returns wheter an underlying grabber could be created
    bool isNull() const { return m_poGrabber == 0; }
    
    virtual const ImgParams &getDesiredParams()const{
      static ImgParams nullParams;
      ICLASSERT_RETURN_VAL(!isNull(),nullParams);
      return m_poGrabber->getDesiredParams();
    }
     
    /// returns current desired image size (default is "320x240"
    virtual const Size &getDesiredSize()const{
      static Size nullSize;
      ICLASSERT_RETURN_VAL(!isNull(),nullSize);
      return m_poGrabber->getDesiredSize();
     }
     
     /// returns current desired image format (default is formatRGB)
     virtual format getDesiredFormat() const{
       ICLASSERT_RETURN_VAL(!isNull(),formatMatrix);
       return m_poGrabber->getDesiredFormat();
     }

     /// returns current desired image depth (default is depth8u)
     virtual depth getDesiredDepth() const{
       ICLASSERT_RETURN_VAL(!isNull(),depth8u);
       return m_poGrabber->getDesiredDepth();
     }
     
     /// sets current desired image parameters
     virtual void setDesiredParams(const ImgParams &p){
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredParams(p);
     }

     /// sets current desired image size
     virtual void setDesiredSize(const Size &s){
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredSize(s);
     }
     
     /// sets current desired image format
     virtual void setDesiredFormat(format f){
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredFormat(f);
     }
     
     /// returns current desired image depth
     virtual void setDesiredDepth(depth d){
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredDepth(d);
     }
     
     virtual void setIgnoreDesiredParams(bool flag){
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setIgnoreDesiredParams(flag);
     }
     virtual bool getIgnoreDesiredParams() const{
       ICLASSERT_RETURN_VAL(!isNull(),false);
       return m_poGrabber->getIgnoreDesiredParams();
     }
     
  };

#define FROM_PROGARG(ARG) pa(ARG),*pa(ARG)+"="+*pa(ARG,1)
 
} 

#endif
