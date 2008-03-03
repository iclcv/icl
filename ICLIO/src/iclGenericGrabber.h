#ifndef ICL_GENERIC_GRABBER_H
#define ICL_GENERIC_GRABBER_H

#include <iclGrabber.h>
#include <string>
#include <iclMacros.h>

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
                                  undesired devices can be left out!
        
        @param params comma seperated device depend parameter list: e.g.
                                  "pwc=0,file=images//image*.ppm,dc=0" with self-explaining syntax\n
                                  Semantics:\n
                                  - pwc=device-index (int)
                                  - dc=device-index (int)
                                  - file=pattern (string)
                                  - unicap=device pattern (string)
        @param notifiyErrors if set to false, no error output is given; use isNull() function 
                             at runtime instead!
    */
    GenericGrabber(const std::string &devicePriorityList="dc,pwc,file", 
                   const std::string &params="pwc=0,dc=0,file=images/*.ppm",
                   bool notifyErrors = true);
    
    std::string getType() const { 
      return m_sType; 
    }
    
    /// Destructor
    virtual ~GenericGrabber(){
      ICL_DELETE(m_poGrabber);
    }
    
    /// grabbing function
    virtual const ImgBase* grab(ImgBase **ppoDst=0){
      ICLASSERT_RETURN_VAL(!isNull(),0);
      return m_poGrabber->grab(ppoDst);
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
    bool isNull(){ return m_poGrabber == 0; }
  };
  
 
} 

#endif
