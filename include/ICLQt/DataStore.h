/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/DataStore.h                              **
** Module : ICLQt                                                  **
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

#ifndef ICL_DATA_STORE_H
#define ICL_DATA_STORE_H

#include <ICLUtils/MultiTypeMap.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StringUtils.h>
namespace icl{
  
  /// Extension of the associative container MultiTypeMap \ingroup UNCOMMON
  /** Adds an index operator[string] for direct access to contained values
   */
  class DataStore : public MultiTypeMap{
    public:

    /// Internal Exception type thrown if operator[] is given an unknown index string
    struct KeyNotFoundException : public ICLException{
      KeyNotFoundException(const std::string &key):ICLException("Key not found: " + key){}
    };
    
    /// Internal Exception type thrown if Data::operator= is called for incompatible values
    struct UnassignableTypesException : public ICLException{
      UnassignableTypesException(const std::string &tSrc, const std::string &tDst):
      ICLException("Unable to assign "+ tDst+ " = "+ tSrc){}
    };
    
    
    /// Arbitrary Data encapsulation type
    class Data{
      
      /// internally reference DataStore entry
      DataArray *data;
      
      /// Constructor (private->only the parent DataStore is allowed to contruct Data's)
      inline Data(DataArray *data):data(data){}
      
      /// This is the mighty and magic conversion function 
      static void assign(void *src, const std::string &srcType, void *dst, 
                         const std::string &dstType) throw (UnassignableTypesException); 

      public:
      
      /// Internally used Data- Structure
      struct Event{
        Event(const std::string &msg="", void *data=0):message(msg),data(data){}
        std::string message;
        void  *data;
      };
      
      friend class DataStore;
      
      /// Trys to assign an instance of T to this Data-Element
      /** This will only work, if the data types are assignable */
      template<class T>
      inline void operator=(const T &t) throw (UnassignableTypesException){
        assign(const_cast<void*>(reinterpret_cast<const void*>(&t)),
               get_type_name<T>(),data->data,data->type);
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

      /// implicit conversion into l-value type (little dangerous)
      template<class T>
      operator T() const throw (UnassignableTypesException){
        return as<T>();
      }

      /// returns the internal type ID (obtained by C++'s RTTI)
      const std::string &getTypeID() const { return data->type; }
      
      /// this is necessary for some gui components
      /** Currently supported for Data-types ImageHandle, DrawHandle and FPSHandle */
      void update() throw (UnassignableTypesException){ 
        *this = Event("update"); 
      }
      
      /// data must be of type MouseHandler*
      void install(void *data){
        *this  = Event("install",data);
      }
      
      /// cb must be a GUI::Callback*
      void registerCallback(void *cb){
        *this = Event("register",cb);
      }
      
      /// possible for all handle-types
      void enable(){
        *this = Event("enable");
      }
      
      /// possible for all handle types
      void disable(){
        *this = Event("disable");
      }
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
    Data operator[](const std::string &key) throw (KeyNotFoundException);
    
    
    /// convenience function that allows collecting data from different source entries
    /** This function can e.g. be used to obtain data from an array of 'float' GUI
        components */
    template<class T>
    std::vector<T> collect(const std::vector<std::string> &keys){
      std::vector<T> v(keys.size());
      for(unsigned int i=0;i<keys.size();++i) v[i] = operator[](keys[i]);
      return v;
    }
    
  };
}


#endif
