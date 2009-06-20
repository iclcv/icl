#ifndef ICL_DATA_STORE_H
#define ICL_DATA_STORE_H

#include <string>
#include <map>
#include <iclSmartPtr.h>
#include <typeinfo>
#include <QMutex>
#include <QMutexLocker>
#include <iclException.h>
#include <cstdio>

namespace icl{
  
  /// Abstract and associative Data Container for Data of different types \ingroup UNCOMMON
  /** The GUIDataStore class can be used to create an associative container
      for different types. It provides an interface for a type-save handling
      of single elements as well as arrays of data.\n
      Single elements are created internally as a copy (copy constructor)
      of a given value (by default the empty constructor for a specific type
      is used to create a default initializing instance). Arrays elements and
      value elements may not be mixed up as array elements must be created 
      and released in a different way (using new[] and delete[] instead of 
      new() and delete).\n
      In addition the class provides some utility functions to get information
      about all contained data elements.\n
      The type-safety is facilitated using the C++ RTTI (Run-Time Type 
      Identification) which is not very fast. Also the access functions 
      getValue() and getArray() are not very fast, because the underlying memory
      is organized in a std::map, which must be searched. Hence, the more
      elements a DataStore object contains, the slower a single data element
      will be accessible (it's just a map internally :-) ). \n
      To accelerate data access just store a reference to the data element or
      a pointer anywhere and work with that pointer!.
   */
  class DataStore{
    public:
    /// Default constructor (create a new DataStore object)
    inline DataStore(){
      m_oDataMapPtr = SmartDataMapPtr(new DataMap);
      m_oMutexPtr = SmartMutexPtr(new QMutex);
    }
    
    /// Destructor (deletes all remaining data)
    inline ~DataStore(){
      if(m_oDataMapPtr.use_count() == 1){
        for(DataMap::iterator it = m_oDataMapPtr->begin(); it != m_oDataMapPtr->end(); ++it){
          DataArray &da = it->second;
          da.release_func(&da);
        }
        m_oDataMapPtr->clear();
      }
    }
    
    /// internally used wrapper function for RTTI
    template<class T>
    static inline const std::string &get_type_name(){
      static std::string NAME = typeid(T).name();
      return NAME;
    }
   
    /// Allocates a new memory block (new T[n]) for the given id string
    /** @param id name of this data block
        @param n count of elements (min 1) 
        @return data pointer that was just created
    */
    template<class T>
    inline T *allocArray(const std::string &id,unsigned int n){
      static T* _NULL = 0;
      if(!n){
        ERROR_LOG("unable to create an array of size 0 for id " << id << "!");
        return _NULL;
      }
      if(contains(id)){
        ERROR_LOG("id " << id << "is already defined");
        return _NULL;
      }
      DataArray &da = (*m_oDataMapPtr)[id];
      da.data = new T[n];
      da.len = n;
      da.type = get_type_name<T>();
      da.release_func = DataArray::release_data_array<T>;
      return reinterpret_cast<T*>(da.data);
    }
    
    /// Allocates a new memory elements (new T(val)) for the given id string
    /** @param id name of this data block 
        @param val initial value for this data block
        @return reference the the data element, that was just created
    */
    template<class T>
    inline T &allocValue(const std::string &id, const T &val=T()){
      static T _NULL = T();
      if(contains(id)){
        ERROR_LOG("id " << id << "is already defined");
        return _NULL;
      }
      DataArray &da = (*m_oDataMapPtr)[id];
      da.data = new T(val);
      da.len = 0; // indicates a single value !
      da.type = get_type_name<T>();

      da.release_func = DataArray::release_data_array<T>;
      return *(reinterpret_cast<T*>(da.data));
    }
    
    /// release the data element that is associated with the given id
    /** @param id name of the entry to release */
    template<class T>
    inline void release(const std::string &id){
      if(!contains(id)){
        ERROR_LOG("id "<<  id  << " not found \n");
        return;
      }
      DataArray &da = (*m_oDataMapPtr)[id];
      if(da.type != get_type_name<T>()){
        ERROR_LOG("unable to cast "<<  id  << " to a given type "<< get_type_name<T>() <<"\n");
        ERROR_LOG("type is " << da.type << "\n");
        return;
      }
      da.release_func(&da);
      m_oDataMapPtr->erase(m_oDataMapPtr->find(id));
    }
    
    /// get a T* that is associated with the given id
    /** @param id name of the entry to get 
        @param lenDst pointer to store the array len (in T's) in if not NULL 
        @return data pointer that is associated with id or NULL if the id is invalid 
    */
    template<class T>
    inline T* getArray(const std::string &id, int *lenDst=0){
      if(!contains(id)){
        ERROR_LOG("id "<<  id  << " not found \n");
        return 0;
      }

      DataArray &da = (*m_oDataMapPtr)[id];

      if(da.type != get_type_name<T>()){
        ERROR_LOG("unable to cast "<<  id  << " to a given type "<< get_type_name<T>() <<"\n");
        return 0;
      }
      if(!da.len){
        ERROR_LOG("unable to access entry " << id << " as array, because it is a value!");
        return 0;
      }
      if(lenDst) *lenDst = da.len;
      return reinterpret_cast<T*>(da.data);
    }
   
    /// get a T reference that is associated with the given id
    /** @param id name of the entry to get 
        @return reference to the value that is associated with the given id
    */
    template<class T>
    inline T &getValue(const std::string &id, bool checkType=true){
      static T _NULL;
      if(!contains(id)){
        ERROR_LOG("id "<<  id  << " not found \n");
        return _NULL;
      }

      // orig
      DataArray &da = (*m_oDataMapPtr)[id];
      //DEBUG_LOG("type of da is " << da.type);
      if(checkType && (da.type != get_type_name<T>())){
        ERROR_LOG("unable to cast "<<  id  << " to a given type "<< get_type_name<T>() <<"\n");
        ERROR_LOG("type is " << da.type << "\n");
        return _NULL;
      }
      if(da.len){
        ERROR_LOG("unable to access entry " << id << " as value, because it is an array!");
        return _NULL;
      }
      return *reinterpret_cast<T*>(da.data);
    }
      
    template<class T>
    inline const T &getValue(const std::string &id, bool checkType=true) const{
      return const_cast<DataStore*>(this)->getValue<T>(id,checkType);
    }

   
    
    /// returns the RTTI string type identifier, for the entry associated with id
    /** @param id name of the entry
        @return RTTI string type identifier
    */
    inline const std::string &getType(const std::string &id) const{
      if(!contains(id)){
        ERROR_LOG("id "<<  id  << " not found \n");
        static const std::string _NULL = "null";
        return _NULL;
      }
      return (*m_oDataMapPtr)[id].type;
    }
    
    /// checks if the type-id associated with the template parameter T is compatible to the entry for id
    /** @param id name of the entry
        @return whether the entry associated with id has type T */
    template<class T>
    inline bool checkType(const std::string &id) const{
      if(!contains(id)){
        ERROR_LOG("id "<<  id  << " not found \n");
        return false;
      }
      return (*m_oDataMapPtr)[id].type == get_type_name<T>();
    }
    
    /// returns whether an entry is an array or a value
    /** @param id name of the entry to check
        @return whether the entry for id is an array 
    */
    inline bool isArray(const std::string &id) const{
      if(!contains(id)){
        ERROR_LOG("id "<<  id  << " not found \n");
        return false;
      }
      return (*m_oDataMapPtr)[id].len != 0; 
    }
    
    /// returns whether a given value is already set
    /** @param id name of the parameter
        @return whether a parameter with that id is contained*/
    inline bool contains(const std::string &id) const{
      return m_oDataMapPtr->find(id) != m_oDataMapPtr->end();
    }
    
    /// shows a list of all entries to std::out
    inline void show() const {
      printf("DataArray: \n");
      int i=0;
      for(DataMap::iterator it = m_oDataMapPtr->begin(); it != m_oDataMapPtr->end(); ++it){
        printf("Entry %3d of length %9d  is \"%s\" \n",i++,it->second.len, it->first.c_str());
      }
      
    }
    // internally locks the datastore
    inline void lock() const { m_oMutexPtr->lock(); }
    
    /// internally unlocks the data store
    inline void unlock() const { m_oMutexPtr->unlock(); }
    private:
    /// internally used data handling structure
    struct DataArray{
      
      /// delete function, given to the data Array after construction to delete its own data
      /** @param da filled with the "this" argument internally by the Parent DataStore object
      */
      template<class T>
      static void release_data_array(DataArray *da){
        ICLASSERT_RETURN(da->type == get_type_name<T>());
        if(da->len) delete [] reinterpret_cast<T*>(da->data);
        else delete reinterpret_cast<T*>(da->data);
      }
      /// Create an empty DataArray object
      DataArray(void *data=0, int len=0):data(data),len(len),type(""),release_func(0){}
      void *data; //<! identified using the type string
      int len;    //<! length of the data array or 0 if it was created using () instead of []
      std::string type; //<! created using RTTI
      void (*release_func)(DataArray*); //<! data release function called by the parent DataStore object
    };
    
    public:
    /// shows a list of currently contained data
    void listContents() const{
      printf("DataStore content: \n");
      int i=0;
      for(DataMap::const_iterator it=m_oDataMapPtr->begin(); it != m_oDataMapPtr->end(); ++it){
        printf("%d: name:\"%s\" type:\"%s\"  arraylen:\"%d\" \n",i++,it->first.c_str(),it->second.type.c_str(),it->second.len);
      }
      printf("----------------------\n");
    }
    // removes all data from this data store
    void clear(){
      *this = DataStore();
    }
    
    /// entry struct used in getEntryList function
    struct Entry{
      Entry(){}
      Entry(const std::string &key,const std::string &type, int len):
        key(key),type(type),len(len){}
      std::string key;
      std::string type;
      int len;
    };
    

    /// Internal Exception type 
    struct KeyNotFoundException : public ICLException{
      KeyNotFoundException(const std::string &key):ICLException("Key not found: " + key){}
    };
    
    /// Internal Exception type 
    struct UnassignableTypesException : public ICLException{
      UnassignableTypesException(const std::string &tSrc, const std::string &tDst):ICLException("Unable to assign "+ tDst+ " = "+ tSrc){}
    };
    
    
    /// Arbitrary Data encapsulation type
    class Data{
      
      /// internally reference DataStore entry
      DataArray *data;
      
      /// Constructor (private->only the parent DataStore is allowed to contruct Data's)
      inline Data(DataArray *data):data(data){}
      
      /// This is the mighty and magic conversion function 
      static void assign(void *src, const std::string &srcType, void *dst, const std::string &dstType) throw (UnassignableTypesException); 

      public:
      
      /// Internally used Data- Structure
      struct Event{
        Event(const std::string &msg=""):message(msg){}
        std::string message; 
      };
      
      friend class DataStore;
      
      /// Trys to assign an instance of T to this Data-Element
      /** This will only work, if the data types are assignable */
      template<class T>
      inline void operator=(const T &t) throw (UnassignableTypesException){
        assign(const_cast<void*>(reinterpret_cast<const void*>(&t)), get_type_name<T>(),data->data,data->type);
      }

      /// Trys to convert a Data element into a (by template parameter) given type
      /** this will only work, if the data element is convertable to the
          desired type. Otherwise an exception is thrown*/
      template<class T>
      inline T as() const throw (UnassignableTypesException){
        T t;
        assign(data->data,data->type,&t,get_type_name<T>());
        return t;
      }
      
      /// returns the internal type ID (obtained by C++'s RTTI)
      const std::string &getTypeID() const { return data->type; }
      
      /// this is necessary for some gui components
      /** Currently supported for Data-types ImageHandle, DrawHandle and FPSHandle */
      void update() throw (UnassignableTypesException){ *this = Event("update"); }
    };
    

    /// Allows to assign new values to a entry with given key (*NEW*)
    /** The returned Data-Element is a shallow Reference of a DataStore entry of
        internal type DataArray.
        Each entry is typed by C++'s RTTI, so, the function can determine if the
        assigned value is compatible to actual type of the data element. 
        Internally a <em>magic</em> assignment system is used to determine whether
        two types can be assigned, and what to do if.
        Basically one can say, type assignments line T = T are of course allowed
        as well as type assignments A = B, if A and B are C++ built-in numerical
        type (int, float, etc.)
        Additionally a lot of GUIHandle types (contained by a GUI's DataStore)
        can be assigned (and updated, see void DataStore::Data::update()) with
        this mechanism
        
        A detailed description of all allowed assigmnet must ba added here:
        TODO...
        
        Here are some examples ...
        \code
        DataStore x;
        x.allocValue("hello",int(5));
        x.allocValue("world",float(5));
        x.allocValue("!",std::string("yes yes!"));
        
        x["hello"] = "44"; // string to int
        x["world"] = 4;    // int to float
        x["!"] = 44;       // int to string
        x["hello"] = 44;   // int to int
        
        x.allocValue("image",ImgHandle(...));
        x["image"] = icl::create("parrot");   // ImgQ to ImageHandle
        
        x.allocValue("sl",SliderHandle(...));
        
        x["sl"] = 7;               // sets slider value to 7
        std::cout << "slider val is:" << x["sl"].as<int>() << std::endl;
        
        x["sl"] = Range32s(3,9);   // sets slider's Range ...
        \endcode
    */
    inline Data operator[](const std::string &key) throw (KeyNotFoundException){
      DataMap::iterator it = m_oDataMapPtr->find(key);
      if(it == m_oDataMapPtr->end()) throw KeyNotFoundException(key);
      return Data(&it->second);
    }
    
    /// returns a list of all entries
    std::vector<Entry> getEntryList() const{
      std::vector<Entry> es;
      for(DataMap::const_iterator it = m_oDataMapPtr->begin(); 
          it != m_oDataMapPtr->end(); ++it){
        es.push_back(Entry(it->first,it->second.type,it->second.len));
      }
      return es;
    }

    private:

    /// internal definition
    typedef std::map<std::string,DataArray> DataMap;

    /// internal definition
    typedef SmartPtr<DataMap,PointerDelOp> SmartDataMapPtr;
    
    /// internal definition
    typedef SmartPtr<QMutex,PointerDelOp> SmartMutexPtr;
 
    /// Smart-Pointer to the underlying data (allows shallow copies)
    mutable SmartDataMapPtr m_oDataMapPtr;
    
    /// mutex to handle syncronous calls
    mutable SmartMutexPtr m_oMutexPtr;
  };
}


#endif
