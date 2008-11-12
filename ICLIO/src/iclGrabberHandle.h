#ifndef ICL_GRABBER_HANDLE_H
#define ICL_GRABBER_HANDLE_H

#include <iclGrabber.h>
#include <iclMutex.h>
#include <iclSmartPtr.h>
#include <iclStringUtils.h>
#include <map>
#include <string>

namespace icl{
  
  /** \cond just an internal structure */
  template<class G>
  struct GrabberHandleInstance{
    GrabberHandleInstance(std::string id="", G* ptr = 0):
      id(id),ptr(ptr){}
    ~GrabberHandleInstance(){
      ICL_DELETE(ptr);
    }
    std::string id;
    G *ptr;
    Mutex mutex;
  };
  /** \endcond */


  /// Wrapper class for Grabber instances to provide "shared" grabber instances
  /** In general, grabber implementations do not provide shared device functionalities. Once
      having implemented a Grabber class e.g. MyGrabber, the GrabberHandle class can easily 
      be used to make MyGrabber Support shared access to underlying grabbing devices.
      
      \section PROB The Problem
      If we are using an instance of a dc grabber associated with a specific DC-Device, it's not 
      possible to access the grabber and therewith the input image stream from another instance. 
      Hence underlying dc devices can not be opened twice, instantiating another Grabber on
      the same device would cause some serious problems that will cause a segmentation fault 
      (in the best case).

      \section ADAPT Adapting an existing Grabber to provide shared device access
      E.g. you have written a grabber class called DemoGrabber, that doesn't provide shared
      access to the underlying grabbing device (we use the actually existing ICLIO/DemoGrabber here).

      Firstly, you have to rename the DemoGrabber to another name e.g. DemoGrabberImpl. Subsequent,
      you can implement a new DemoGrabber class by deriving the GrabberHandle<DemoGrabberImpl> template
      class. In your new DemoGrabber implementation you have to implement only the constructor, with
      respect to the following points:
      
      \code
      // new demo grabber class deriving the appropiate GrabberHandle-template
      class DemoGrabber : public GrabberHandle<DemoGrabberImpl>{
         public:
         // Constructor (no explicit call to the super class constructor)
         // but identical parameter list
         DemoGrabber(float maxFPS=30){  
            std::string id = str(maxFPS);   // we need an ID that represents the
                                            // the underlying wrapped device, and that can be 
                                            // associated uniquely 

            // initialization of the top level grabber class:
            // 1st: check, if given grabber id is brand new
            // 2nd: ** if the id is new: initialize a real new grabber 
            //         by passing the parameters to the constructor of
            //         of the underlying class
            //      ** if not, then initialize this instance without a new 
            //         grabber, by calling initialize(id). Here, this becomes
            //         a shared grabber copy of the already existing one
            if(isNew(id)){
               initialize(new DemoGrabberImpl(maxFPS),id);
            }else{
               initialize(id);
            }
         }
      };
      \endcode
  */
  template<class G>
  class GrabberHandle : public Grabber{
    public:

    typedef SmartPtr<GrabberHandleInstance<G>,PointerDelOp> InstancePtr;
    typedef std::map<std::string,InstancePtr> InstanceMap;

    inline std::string getID() const{
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return m_instance->id;
    } 
    
    static inline bool isNew(const std::string &id){
      Mutex::Locker l(s_mutex);
      return s_instances.find(id) == s_instances.end();
    }

    inline void initialize(const std::string &id){
      Mutex::Locker l(s_mutex);
      m_instance = s_instances[id];
    }
    inline void initialize(G* g, const std::string &id){
      ICLASSERT_RETURN(isNew(id));
      Mutex::Locker l(s_mutex);
      m_instance = s_instances[id] = InstancePtr(new GrabberHandleInstance<G>(id,g));
    }
    inline ~GrabberHandle(){
      if(isNull()) return;
      Mutex::Locker l(s_mutex);
      if(m_instance.use_count() == 2){
        // only two remaining instances: m_instance and s_instances[getID()]
     
        // nice C++ code here -> the typename must be added here because the iterator
        // type depends indirectly on the template parameter (or something like that)
        typename InstanceMap::iterator it = s_instances.find(m_instance->id);
        ICLASSERT_RETURN(it != s_instances.end());
        s_instances.erase(it); // releases s_instances[getID()]

        // m_instance itself is released automatically by it's destructor
      }
    }
    
    inline bool isNull() const{
      return !static_cast<bool>(m_instance);
    }
    virtual inline const ImgBase* grab(ImgBase **ppoDst=0){
      ICLASSERT_RETURN_VAL(!isNull(),0);
      Mutex::Locker l(m_instance->mutex);
      return m_instance->ptr->grab(ppoDst);
    }
    virtual inline void setProperty(const std::string &property, const std::string &value){      
      ICLASSERT_RETURN(!isNull());
      Mutex::Locker l(m_instance->mutex);
      m_instance->ptr->setProperty(property,value);
    }
    virtual inline std::vector<std::string> getPropertyList(){
      ICLASSERT_RETURN_VAL(!isNull(),std::vector<std::string>());
      Mutex::Locker l(m_instance->mutex);
      return m_instance->ptr->getPropertyList();
    }
    virtual inline  bool supportsProperty(const std::string &property){
      ICLASSERT_RETURN_VAL(!isNull(),false);
      Mutex::Locker l(m_instance->mutex);
      return m_instance->ptr->supportsProperty(property);
    }
    virtual inline std::string getType(const std::string &name){
      ICLASSERT_RETURN_VAL(!isNull(),"undefined");
      Mutex::Locker l(m_instance->mutex);
      return m_instance->ptr->getType(name);
    }
    virtual inline std::string getInfo(const std::string &name){
      ICLASSERT_RETURN_VAL(!isNull(),"undefined");
      Mutex::Locker l(m_instance->mutex);
      return m_instance->ptr->getInfo(name);
    }
    virtual inline std::string getValue(const std::string &name){
      ICLASSERT_RETURN_VAL(!isNull(),"undefined");
      Mutex::Locker l(m_instance->mutex);
      return m_instance->ptr->getValue(name);
    }
  
    protected:
    /// internal instance pointer
    InstancePtr m_instance;
    
    private:

    /// list of all instances
    static InstanceMap s_instances;
    
    /// static mutex protecting the instance map
    static Mutex s_mutex;
  };

  /** \cond */
  template<class G> std::map<std::string,icl::SmartPtr<GrabberHandleInstance<G>,PointerDelOp> > GrabberHandle<G>::s_instances;
  template<class G> Mutex GrabberHandle<G>::s_mutex; 
  /** \endcond */
}

#endif
