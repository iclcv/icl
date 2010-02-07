#ifndef ICL_GENERIC_GRABBER_H
#define ICL_GENERIC_GRABBER_H

#include <ICLIO/Grabber.h>
#include <string>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Lockable.h>

namespace icl {

  /// Common interface class for all grabbers \ingroup GRABBER_G
  /** The generic grabber provides an interface for a multi-platform
      compatible grabber. */
  class GenericGrabber: public Grabber{
    
    Grabber *m_poGrabber; //!< internally wrapped grabber instance
    
    std::string m_sType; //!< type of current grabber implementation
    
    mutable Mutex m_mutex; //! << internal protection for re-initialization
    public:
    
    /// Empty default constructor, which creates a null-instance
    /** null instances of grabbers can be adapted using the init-function*/
    GenericGrabber():m_poGrabber(0){}
    
    /// Create a generic grabber instance with given device priority list
    /** internally this function calls the init function immediately*/
    GenericGrabber(const std::string &devicePriorityList,
                   const std::string &params,
                   bool notifyErrors = true) throw (ICLException);
    
    /// initialization function to change/initialize the grabber back-end
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
                                  - <b>create</b> create grabber (create an image using ICL's create function)
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
                                  - create=image name (@see icl::TestImages::create)
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
    **/
    void init(const std::string &devicePriorityList,
              const std::string &params,
              bool notifyErrors = true) throw (ICLException);
        
    /// resets resource on given devices (e.g. firewire bus)
    static void resetBus(const std::string &deviceList="dc", bool verbose=false);
   
    /// return the actual grabber type
    std::string getType() const { 
      Mutex::Locker __lock(m_mutex);
      return m_sType; 
    }
    
    /// returns the wrapped grabber itself
    Grabber *getGrabber() const {
      Mutex::Locker __lock(m_mutex); 
      return m_poGrabber; 
    }
    
    /// Destructor
    virtual ~GenericGrabber(){
      Mutex::Locker __lock(m_mutex);
      ICL_DELETE(m_poGrabber);
    }
    
    /// grabbing function
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),0);
      return m_poGrabber->grabUD(ppoDst);
    }

    /// returns a list of all properties, that can be set
    virtual std::vector<std::string> getPropertyList(){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),std::vector<std::string>());
      return m_poGrabber->getPropertyList();
    }

    /// setting up properties of underlying grabber
    virtual void setProperty(const std::string &property, const std::string &value){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN(!isNull());
      m_poGrabber->setProperty(property,value);
    }

    /// returns whether property is supported by underlying grabber
    virtual bool supportsProperty(const std::string &property){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),false);
      return m_poGrabber->supportsProperty(property);
    }
    
    /// returns the property type of given property
    virtual std::string getType(const std::string &name){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getType(name);
    }
     
    /// retuns property information
    virtual std::string getInfo(const std::string &name){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getInfo(name);
    }
    
    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name){
      Mutex::Locker __lock(m_mutex);
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_poGrabber->getValue(name);
    }

    /// returns wheter an underlying grabber could be created
    bool isNull() const { return m_poGrabber == 0; }
    
    /// simpler interface for isNull() (returns !isNull()
    operator bool() const { return !isNull(); }

    virtual const ImgParams &getDesiredParams()const{
      Mutex::Locker __lock(m_mutex);
      static ImgParams nullParams;
      ICLASSERT_RETURN_VAL(!isNull(),nullParams);
      return m_poGrabber->getDesiredParams();
    }
     
    /// returns current desired image size (default is "320x240"
    virtual const Size &getDesiredSize()const{
      Mutex::Locker __lock(m_mutex);
      static Size nullSize;
      ICLASSERT_RETURN_VAL(!isNull(),nullSize);
      return m_poGrabber->getDesiredSize();
     }
     
     /// returns current desired image format (default is formatRGB)
     virtual format getDesiredFormat() const{
       Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN_VAL(!isNull(),formatMatrix);
       return m_poGrabber->getDesiredFormat();
     }

     /// returns current desired image depth (default is depth8u)
     virtual depth getDesiredDepth() const{
       Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN_VAL(!isNull(),depth8u);
       return m_poGrabber->getDesiredDepth();
     }
     
     /// sets current desired image parameters
     virtual void setDesiredParams(const ImgParams &p){
       Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredParams(p);
     }

     /// sets current desired image size
     virtual void setDesiredSize(const Size &s){
      Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredSize(s);
     }
     
     /// sets current desired image format
     virtual void setDesiredFormat(format f){
       Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredFormat(f);
     }
     
     /// returns current desired image depth
     virtual void setDesiredDepth(depth d){
       Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setDesiredDepth(d);
     }
     
     virtual void setIgnoreDesiredParams(bool flag){
       Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN(!isNull());
       m_poGrabber->setIgnoreDesiredParams(flag);
     }
     virtual bool getIgnoreDesiredParams() const{
       Mutex::Locker __lock(m_mutex);
       ICLASSERT_RETURN_VAL(!isNull(),false);
       return m_poGrabber->getIgnoreDesiredParams();
     }
     
  };

#define FROM_PROGARG(ARG) pa(ARG),*pa(ARG)+"="+*pa(ARG,1)
 
} 

#endif
