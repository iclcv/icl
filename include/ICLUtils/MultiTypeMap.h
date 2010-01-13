#ifndef ICL_MULTI_TYPE_MAP_H
#define ICL_MULTI_TYPE_MAP_H

#include <string>
#include <map>
#include <typeinfo>
#include <cstdio>

#include <ICLUtils/Mutex.h>
#include <ICLUtils/SmartPtr.h>

namespace icl{
  
  /// Abstract and associative Data Container for Data of different types
  /** The MultiTypeMap class can be used to create an associative container
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
      elements a MultiTypeMap object contains, the slower a single data element
      will be accessible (it's just a map internally :-) ). \n
      To accelerate data access just store a reference to the data element or
      a pointer anywhere and work with that pointer!.
   */
  class MultiTypeMap{
    public:
    /// Default constructor (create a new MultiTypeMap object)
    MultiTypeMap();
    
    /// Destructor (deletes all remaining data)
    ~MultiTypeMap();
    
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
      if(!n){
        ERROR_LOG("unable to create an array of size 0 for id " << id << "!");
        return 0;
      }
      if(contains(id)){
        ERROR_LOG("id " << id << "is already defined");
        return 0;
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
      return const_cast<MultiTypeMap*>(this)->getValue<T>(id,checkType);
    }

   
    
    /// returns the RTTI string type identifier, for the entry associated with id
    /** @param id name of the entry
        @return RTTI string type identifier
    */
    const std::string &getType(const std::string &id) const;
    
    /// checks if the type-id associated with the template parameter T is compatible to the entry for id
    /** @param id name of the entry
        @return whether the entry associated with id has type T */
    template<class T>
    inline bool checkType(const std::string &id) const{
      return check_type_internal(id,get_type_name<T>());
    }
    
    /// returns whether an entry is an array or a value
    /** @param id name of the entry to check
        @return whether the entry for id is an array 
    */
    bool isArray(const std::string &id) const;
    
    /// returns whether a given value is already set
    /** @param id name of the parameter
        @return whether a parameter with that id is contained*/
    bool contains(const std::string &id) const;

    
    // internally locks the datastore
    inline void lock() const { m_oMutexPtr->lock(); }
    
    /// internally unlocks the data store
    inline void unlock() const { m_oMutexPtr->unlock(); }
    protected:

    bool check_type_internal(const std::string &id, const std::string &typestr) const;
    
    /// internally used data handling structure
    struct DataArray{
      
      /// delete function, given to the data Array after construction to delete its own data
      /** @param da filled with the "this" argument internally by the Parent MultiTypeMap object
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
      void (*release_func)(DataArray*); //<! data release function called by the parent MultiTypeMap object
    };
    
    public:
    /// shows a list of currently contained data
    void listContents() const;

    // removes all data from this data store
    void clear();

    
    /// entry struct used in getEntryList function
    struct Entry{
      Entry(){}
      Entry(const std::string &key,const std::string &type, int len):
        key(key),type(type),len(len){}
      std::string key;
      std::string type;
      int len;
    };
    
    /// returns a list of all entries
    std::vector<Entry> getEntryList() const;

    protected:

    /// internal definition
    typedef std::map<std::string,DataArray> DataMap;

    /// internal definition
    typedef SmartPtr<DataMap,PointerDelOp> SmartDataMapPtr;
    
    /// internal definition
    typedef SmartPtr<Mutex,PointerDelOp> SmartMutexPtr;
 
    /// Smart-Pointer to the underlying data (allows shallow copies)
    mutable SmartDataMapPtr m_oDataMapPtr;
    
    /// mutex to handle syncronous calls
    mutable SmartMutexPtr m_oMutexPtr;
  };
}


#endif
