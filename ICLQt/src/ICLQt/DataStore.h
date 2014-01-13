/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DataStore.h                            **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/MultiTypeMap.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/StringUtils.h>
#include <ICLQt/MouseEvent.h>

namespace icl{
  namespace qt{
  
   
    /// Extension of the associative container MultiTypeMap \ingroup UNCOMMON
    /** Adds an index operator[string] for direct access to contained values
     */
    class ICL_QT_API DataStore : public utils::MultiTypeMap{
      public:
  
      /// Internal Exception type thrown if operator[] is given an unknown index string
      struct KeyNotFoundException : public utils::ICLException{
        KeyNotFoundException(const std::string &key):utils::ICLException("Key not found: " + key){}
      };
      
      /// Internal Exception type thrown if Data::operator= is called for incompatible values
      struct UnassignableTypesException : public utils::ICLException{
        UnassignableTypesException(const std::string &tSrc, const std::string &tDst):
        utils::ICLException("Unable to assign "+ tDst+ " = "+ tSrc){}
      };
      
      
      /// Arbitrary Data encapsulation type
      class ICL_QT_API Data{
        
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
        Event(const std::string &msg, const utils::Function<void> &cb): message(msg),data(0),cb(cb){}
        Event(const std::string &msg, const utils::Function<void,const std::string&> &cb2): message(msg),data(0),cb2(cb2){}
          std::string message;
          void *data;
          utils::Function<void> cb;
          utils::Function<void,const std::string&> cb2;
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
        
  
        /** Currently supported for Data-types ImageHandle, DrawHandle, FPSHandle and PlotHandle */
        void render() throw (UnassignableTypesException){ 
          *this = Event("render"); 
        }
  
        /// links DrawWidget3D and GLCallback
        void link(void *data) throw (UnassignableTypesException){ 
          *this = Event("link", data); 
        }
        
        /// data must be of type MouseHandler*
        void install(void *data){
          *this  = Event("install",data);
        }
        
        /// installs a function directly
        void install(utils::Function<void,const MouseEvent &> f);
  
        // installs a global function (should be implicit)
        //void install(void (*f)(const MouseEvent &)){
        //  install(function(f));
        //}
        
        /// register simple callback type
        void registerCallback(const utils::Function<void> &cb){
          *this = Event("register",cb);
        }
  
        /// register simple callback type
        void registerCallback(const utils::Function<void,const std::string&> &cb){
          *this = Event("register-complex",cb);
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
  
      /// gives a list of possible assignemts for optinally given src and dst Type
      static void list_possible_assignments(const std::string &srcType, const std::string &dstType);
      
      /// internally used assignment structure
      struct Assign{
        std::string srcType,dstType,srcRTTI,dstRTTI;
        Assign(const std::string &srcType, const std::string &dstType, 
               const std::string &srcRTTI, const std::string &dstRTTI):
        srcType(srcType),dstType(dstType),srcRTTI(srcRTTI),dstRTTI(dstRTTI){}

        virtual bool operator()(void *src, void *dst){ return false; }
      };
  

      private:
      
      /// internal assign method
      static void register_assignment_rule(Assign *assign);
      
      public:
      
      /// registers a new assignment rule to the DataStore class
      /** After the call, the two types can be assigned (for reading
          and writing DataStore entries) */
      template<class SRC, class DST>
      static inline void register_assignment_rule(const std::string &srcTypeName,
                                                  const std::string &dstTypeName,
                                                  utils::Function<void,const SRC&,DST&> assign){
        struct TypeDependentAssign : public Assign{
          utils::Function<void,const SRC&,DST&> assign;
          TypeDependentAssign(const std::string &srcTypeName, const std::string &dstTypeName,
                              utils::Function<void,const SRC&,DST&> assign):
          Assign(srcTypeName, dstType, get_type_name<SRC>(), get_type_name<DST>()),assign(assign){}
          
          virtual bool operator()(void *src, void *dst){ 
            assign( *(const SRC*)src, *(DST*)dst );
            return true;
          }
        };
        register_assignment_rule(new TypeDependentAssign(srcTypeName,dstTypeName,assign));
      }

      /// registers trivial assignment rule to the DataStore class
      /** Trivial assignments are performed using
          dst = src */
      template<class SRC, class DST>
      static inline void register_trivial_assignment_rule(const std::string &srcTypeName,
                                                          const std::string &dstTypeName){
        struct TypeDependentTrivialAssign : public Assign{
          TypeDependentTrivialAssign(const std::string &srcTypeName, const std::string &dstTypeName):
          Assign(srcTypeName, dstTypeName, get_type_name<SRC>(),  get_type_name<DST>()){}
          
          virtual bool operator()(void *src, void *dst){ 
            *(DST*)dst = *(const SRC*)src;
            return true;
          }
        };
        register_assignment_rule(new TypeDependentTrivialAssign(srcTypeName,dstTypeName));
      }
      
    };
  } // namespace qt
}


