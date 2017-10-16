/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/ConfigFile.h                     **
** Module : ICLUtils                                               **
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
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Lockable.h>
#include <ICLUtils/SmartPtr.h>
#include <map>
#include <typeinfo>

/** \cond */
namespace pugi{
  class xml_document;
}
/** \endcond */


namespace icl{
  namespace utils{
    /** \cond */
    struct ICLUtils_API XMLDocumentDelOp{ static void delete_func(pugi::xml_document *h); };
    /** \endcond */

    /// Utility class for creating and reading XML-based hierarchical configuration files
    /** ConfigFile class can be used in object based as well as in static manner.

        ConfigFile objects can e.g. be used, to locally read a configuration file or
        to create a configuration file. Besides a static singleton ConfigFile object
        accessible via ConfigFile::getConfig can be used as a global configuration for
        applications. \n
        Furthermore a powerful runtime-editor called ConfigFileGUI is available in the
        ICLQt package.

        ConfigFiles are XML-based (using ICL's XML environment for parsing and creating XML
        structure). The document is hierarchical as the following example demonstrates

        <pre>
        \<?xml version='1.0' encoding='ISO-8859-1'?\>
        \<config\>
          \<title\>This is the new title\</title\>
          \<section id="general" \>
            \<section id="params" \>
              \<data type="int" id="threshold" \>7\</data\>
              \<data type="double" id="value" \>6.450000\</data\>
              \<data type="string" id="filename" \>./notHallo.txt\</data\>
            \</section\>
          \</section\>
          \<section id="special" \>
            \<data type="char" id="hint" \>a\</data\>
            \<data type="char" id="no-hint" \>b\</data\>
          \</section\>
        \</config\>
        </pre>

        \section RANGES Data Ranges and Value lists

        In addition to the syntax above, each data-tag can be set up with a range property or a value list. <b>Currently
        data ranges are only used for int- and float- typed data elements</b>. If a range property is defined like this

        <pre>
        \<data id="threshold" type="int" range="[0,255]"\>127\</data\>
        </pre>

        the ConfigFileGUI editor will automatically create an integer slider with given range for this entry.
        (As mentioned above, this feature is also available for float-typed entries).

        <b>Value lists are only supported for string-typed entries!</b> value lists must be defined like this:
        <pre>
        \<data id="grabber-type" type="string" values="[pwc,dc,file,unicap]"\>dc\</data\>
        </pre>
        (Don't forget enclosing brackets). Value lists are translated to a combobox containing all given entries.
        It's self-evident. that the initial value must be within the value list (otherwise an error is shown, and
        the combo-box value is out of date until the combo-box is use for the first time. The same is true for
        ranged float- or int-entry restriction.


        \section OTHERS Other Information

        When accessing ConfigFile data members, each hierarchy level must be
        separated using the '.' character. So e.g. the entry 'filename' of the example above can
        be accessed using

        \code
        ConfigFile config("myConfig.xml"); // myConfig.xml shall contain the contents above
        string fn = config["general.params.filename"];
        \endcode

        Note, that config["general.params.filename"] can only be assigned to std::string instances
        as it's type is xmlfile type is 'string'. Here we use a special C++ technique, which overloads
        the implicit cast operator for the ConfigFile::Data class (with is returned by the operator[])
        by using a template.


        To avoid errors there's also a "get"-function which can be called with a default return value:
        \code
        ConfigFile config("myConfig.xml");
        string fn = config.get<string>("general.params.filename","defaultpath.xml");
        \endcode

        In the same manner, ConfigFile entries can be generated: The example file above
        was generated with the following code (<b>Note: the "config"-prefix is
        compulsory!</b>)

        \code
        ConfigFile a;
        a["config.general.params.threshold"] = 7;
        a["config.general.params.value"] = 6.45f;
        a["config.general.params.filename"] = std::string("./hallo.txt");
        a["config.general.params.filename"] = std::string("./notHallo.txt");
        a["config.special.hint"] = 'a';
        a["config.special.no-hint"] = 'b';
        a.save("config.xml");
        \endcode

        \section TPS Types

        Functions for data access and data definition are implemented as non-inline templates,
        which are instantiated for the following types:
        - char
        - unsigned char
        - short
        - unsigned short
        - int
        - unsigned int
        - float
        - double
        - string
        - Size
        - Point
        - Rect
        - Range32s
        - Range32f

        // additionally we need a fixed matrix type (e.g. color)

        \section PERF Performance

        Internally data is stored in the parent classes (DataStore) hash maps to optimize
        data access. ConfigFile data key is the the '.'-concatenated identifier.
    */
    class ICLUtils_API ConfigFile : public Lockable{

      public:
      friend class ConfigFileGUI;

      /// Internal exception type, thrown if an entry was not found
      struct EntryNotFoundException : public ICLException{
        EntryNotFoundException(const std::string &entryName):
        ICLException("Entry " + entryName+" could not be found!"){}
        virtual ~EntryNotFoundException() throw(){}
      };

      /// Internal exception type, thrown if an entry type missmatch occurs
      struct InvalidTypeException : public ICLException{
        InvalidTypeException(const std::string &entryName, const std::string &typeName):
        ICLException("Invalid type " + typeName + " (entry " + entryName + ")"){};
        virtual ~InvalidTypeException() throw() {}
      };

      /// thrown if unregistered types are used
      struct UnregisteredTypeException : public ICLException{
        UnregisteredTypeException(const std::string &rttiID):
        ICLException("type with RTTI ID " + rttiID + " is not registered"){}
        ~UnregisteredTypeException() throw(){}
      };

      private:

      /// internally used type map class
      struct Maps{
        /// key: rttiType (e.g. i), value: written type (e.g. int)
        std::map<std::string,std::string> typeMap;

        /// reverse ordered map here, key is written type, and value is rtti-type
        std::map<std::string,std::string> typeMapReverse;
      };

      /// returns a singelton instance of type Maps
      static Maps *getMapsInstance();

      /// returns a singelton instance of type Maps as reference
      static Maps &getMapsInstanceRef() { return *getMapsInstance(); }
#if 0
      static std::map<std::string,std::string> s_typeMap;
      static std::map<std::string,std::string> s_typeMapReverse;
#endif

      /// internally used utitlity function
      template<class T>
      static const std::string &get_type_name() throw (UnregisteredTypeException){
        static const std::string &rttiID = get_rtti_type_id<T>();
        if(!type_registered_by_rtti(rttiID)) throw UnregisteredTypeException(rttiID);
        static const std::string &name = getMapsInstanceRef().typeMap[rttiID];
        return name;
      }

      /// internally used utitlity function
      template<class T>
      static inline const std::string &get_rtti_type_id(){
        static std::string NAME = typeid(T).name();
        return NAME;
      }

      /// internally used utitlity function (id must be given without prefix)
      template<class T>
      inline bool check_type(const std::string &id) const throw (EntryNotFoundException,UnregisteredTypeException){
        return check_type_internal(id,get_rtti_type_id<T>());
      }

      /// internally used utitlity function
      bool check_type_internal(const std::string &id, const std::string &rttiTypeID) const
      throw (EntryNotFoundException,UnregisteredTypeException);

      /// internally used utitlity function
      static bool type_registered_by_rtti(const std::string &rttiID){
        Maps &maps = getMapsInstanceRef();
        return (maps.typeMap.find(rttiID) != maps.typeMap.end());
      }

      public:

      /// the macro <tt>REGISTER_CONFIG_FILE_TYPE(T)</tt> can be used to register new types to the data store. This macro is defined as  <b>::icl::utils::ConfigFile::register_type<T>(T)</b><br><br>
/// \cond
#define REGISTER_CONFIG_FILE_TYPE(T) ::icl::utils::ConfigFile::register_type<T>(#T)
/// \endcond

      /// registers a new type in the data store parsing engine
      /** Note: only registered types can be loaded from an xml-file
          currently, the following types are registered automatically:

          <b>POD Types:</b>
          - char
          - unsigned char
          - short
          - unsigned short
          - int
          - unsigned int
          - float
          - double
          - string
          - long int
          - bool

          <b>Other ICL Types:</b>
          - Size
          - Point
          - Rect
          - Size32f
          - Point32f
          - Rect32f
          - Range32s
          - Range32f
       */
      template<class T>
      static void register_type(const std::string &id){
        Maps &maps = getMapsInstanceRef();
        std::string rtti = get_rtti_type_id<T>();
        maps.typeMap[rtti] = id;
        maps.typeMapReverse[id] = rtti;
#if 0
        s_typeMap[get_rtti_type_id<T>()] = id;
        s_typeMapReverse[id] = get_rtti_type_id<T>();
#endif
      }

      /// Default constructor creating an empty ConfigFile instance
      /** The empty ConfigFile has the following string representation:
          <pre>
          \<?xml version='1.0' encoding='ISO-8859-1'?\>
          \<config\>
            \<title\>no title defined"\</title\>
          \</config\>
          </pre>
      */
      ConfigFile();

      /// Creates a ConfigFile instance with given filename
      /** @param filename if filename is found and it contains a valid ConfigFile structure,
                          it is read into the ConfigFile instance. Otherwise, filename is
                          stored internally for later use if load(void) or save(void) is called. */
      ConfigFile(const std::string &filename) throw(FileNotFoundException,InvalidFileFormatException,UnregisteredTypeException);

      /// Creates a ConfigFile from given handle instance
      /** Note: Ownership is passed to this ConfigFile instance here */
      ConfigFile(pugi::xml_document *handle) throw (UnregisteredTypeException);

      /// creates a ConfigFile instance from given istream.
      /** The constructor will only read the stream until the
          first opening tag is closed */
      ConfigFile(std::istream &stream) throw(FileNotFoundException,InvalidFileFormatException,UnregisteredTypeException);

      /// loads the ConfigFile from given filename and updates internal filename variable
      /** Warning: old data content is lost! */
      void load(const std::string &filename) throw(FileNotFoundException,InvalidFileFormatException,UnregisteredTypeException);

      /// Writes data to disk using given filename
      void save(const std::string &filename) const;

      /// Sets up a default prefix automatically put before each given key
      void setPrefix(const std::string &defaultPrefix) const;

      /// Returns given default prefix
      const std::string &getPrefix() const;

      /// Data- type used for the []-operator of ConfigFile instances
      /** @see Data ConfigFile::operator[](const std::string &id) */
      class Data{
        std::string id; //!< internal id (config.foo.bar)
        ConfigFile *cf; //!< parent config file

        /// private constructor
        Data(const std::string &id, ConfigFile &cf);
        public:

        /// for tight integration with parent ConfigFile class
        friend class ConfigFile;

        /// automatic cast operator (lvalue type determines T)
        /** This automatic cast automatically detects the destination (lvalue) type an calls the
            appropiate parse<T> function */
        template<class T>
        operator T() const throw (InvalidTypeException,EntryNotFoundException){
          return cf->get<T>(id);
        }

        /// explicit cast into given type
        /** Sometimes, the implicit automatic cast is not allowed due to ambiguities */
        template<class T>
        T as() const throw (InvalidTypeException,EntryNotFoundException){
          return cf->get<T>(id);
        }

        /// special assignment for data instances
        /** This does just copy the data-reference, not it's referenced value */
        Data &operator=(const Data &d){
          cf = d.cf;
          id = d.id;
          return *this;
        }

        /// mutlti class assignment operator
        /** assigns a string representation of given T instance to the according ConfigFile Entry
            T to string conversion is performed using std::string utils str- function, which
            internally exploits the std::ostream operator<< for given T instance.
            If given type T is not registered, an UnregisteredTypeException is thrown
        */
        template<class T>
        Data &operator=(const T &t) throw (UnregisteredTypeException,InvalidTypeException,EntryNotFoundException){
          cf->set(id,t);
          return *this;
        }
      };


      /// main access function to datastore entries (unconst)
      /* This function can be used for reading as well as for writing ConfigFile entries
         the returned utitlity structure of type Data defers creation of a new
         ConfigFile entry until actually a value is assigned to it.
      **/
      Data operator [](const std::string &id);

      /// main access function to datastore entries (const)
      /** As above, but only for reading ... */
      const Data operator[](const std::string &id) const throw (EntryNotFoundException);

      /// returns all data entries, that match the given regex
      /** note, the current default prefix is <b>not</b> used here */
      std::vector<Data> find(const std::string &regex);

      /// returns all data entries, that match the given regex (const);
      /** note, the current default prefix is <b>not</b> used here.\n
          note2: const-concept not implemented properly */
      const std::vector<Data> find(const std::string &regex) const{
        return const_cast<ConfigFile*>(this)->find(regex);
      }

      /// sets or updates a new data element to the ConfigFile
      /** Warning: if not unique decidable, an explicit information about the
          value type T is compulsory.

          e.g.
          \code
          ConfigFile f;
          f.set("config.filename","myText.txt");
          \endcode

          Causes an error because the template type T is resolved as const char* which
          is not supported yet. Better versions use an explicit template:
          \code
          ConfigFile f;
          f.set<std::string>("config.filename","myText.txt");
          \endcode

          or an explicit string argument:
          \code
          ConfigFile f;
          f.set("config.filename",std::string("myText.txt"));
          \endcode
          to avoid these errors.\n
          <b>Note:</b>
          Integer constants are of type "int" by default, and floating point constants are of
          type "double" by default:
      */
      template<class T>
      void set(const std::string &id, const T &val) throw (UnregisteredTypeException){
        set_internal(id,str(val),get_rtti_type_id<T>());
      }

      /// returns a given value from the internal string based representation (un const)
      /** Internally, the string to T conversion is performed during this function call,
          so it might increase system performance to extract ConfigFile entries not in
          loops or something like that.\n
          Three errors can occur:
          -# given key id is not contained in the config file -> throws and EntryNotFoundException
          -# given key is found, but internal type is not the type associated with
             template parameter T -> throws and InvalidTypeException
          -# type associated with template parameter T is not registered at the
             data store -> throws an instance of UnregisteredTypeException
      */
      template<class T>
      inline T get(const std::string &idIn) const throw (EntryNotFoundException,InvalidTypeException,UnregisteredTypeException){
        if(!contains(idIn)) throw EntryNotFoundException(m_sDefaultPrefix+idIn);
        if(!check_type<T>(idIn)) throw InvalidTypeException(m_sDefaultPrefix+idIn,get_type_name<T>());
        return parse<T>(m_entries.find(m_sDefaultPrefix+idIn)->second.value);
      }

      /// returns a given value from the internal string based representation
      /** Like the function above, except it uses a default value if given key cannot be found
      */
      template<class T>
      inline T get(const std::string &idIn,const T &def) const throw (InvalidTypeException,UnregisteredTypeException){
        if(!contains(idIn)) return def;
        if(!check_type<T>(idIn)) throw InvalidTypeException(m_sDefaultPrefix+idIn,get_type_name<T>());
        return parse<T>(m_entries.find(m_sDefaultPrefix+idIn)->second.value);
      }

      /// loads the global ConfigFile from given filename
      static void loadConfig(const std::string &filename);

      /// loads a ConfigFile object into the global config file (shallowly copied!)
      static void loadConfig(const ConfigFile &configFile);

      /// returns the global ConfigFile
      static const ConfigFile &getConfig(){ return s_oConfig; }

      /// applies get on the static config instances
      template<class T>
      static inline T sget(const std::string &id) throw (EntryNotFoundException,InvalidTypeException){
        return getConfig().get<T>(id);
      }

      /// applies get on the static config instances (with default)
      template<class T>
      static inline T sget(const std::string &id,const T &def) throw (InvalidTypeException){
        return getConfig().get<T>(id,def);
      }

      /// lists whole datastore contents
      void listContents() const;

      /// returns whether an entry with given ID is contained
      bool contains(const std::string &id) const;

      /// Utility Type for restriction of type values
      /** Currently key restrictions cannot be propergated to the XML structure when set*/
      struct KeyRestriction{
        inline KeyRestriction():
          hasRange(false),hasValues(false){}
        inline KeyRestriction(double min, double max):
          hasRange(true),hasValues(false){
            this->min = min;
            this->max = max;
        }
        inline KeyRestriction(const std::string &values):
          values(values),hasRange(false),hasValues(true){}
        ICLUtils_API std::string toString() const;

        double min,max;
        std::string values;

        bool hasRange;
        bool hasValues;
      };

      /// defined the range for given number-type-valued key
      /** This feature is used by the ConfigFileGUI to create appropriate slider
          ranges if requested */
      void setRestriction(const std::string &id, const KeyRestriction &r) throw (EntryNotFoundException);

      /// returns predefined range for given id (or 0 if no range was defined for this key)
      /** This feature is only used by the config file GUI */
      const KeyRestriction *getRestriction(const std::string &id) const throw (EntryNotFoundException);

      /// internal utility structure for contained data
      struct Entry{
        std::string id;           //!< entries key (config.foo.bar....)
        std::string value;        //!< entries value as string
        std::string rttiType;     //!< entries rtti type ID
        SmartPtr<KeyRestriction> restr;  /// optional restriction pointer

        /// returns the written type name that is associated with rttiType internally
        const std::string &getTypeName() const { return getMapsInstanceRef().typeMap[rttiType]; }

        /// returns this entries id relatively to the parent ConfigFiles prefix
        std::string getRelID() const {
          const std::string &pfx = parent->getPrefix();
          if(!pfx.length()) return id;
          else return id.substr(pfx.length());
        }
        ConfigFile *parent; // parent ConfigFile (used to obtain current prefix)
      };

      /// iterator type to run through all entries (const only)
      typedef std::map<std::string,Entry>::const_iterator const_iterator;

      /// all-entry iterator begin
      const_iterator begin() const{ return m_entries.begin(); }

      /// all-entry iterator end
      const_iterator end() const{ return m_entries.end(); }

      /// returns all entries as vector<const Entry*>
      const std::vector<const Entry*> getEntryList(bool relToPrefix=false) const{
        std::vector<const Entry*> v;
        for(const_iterator it =begin();it != end(); ++it){
          v.push_back(&it->second);
        }
        return v;
      }

      /// removes all contents (except config and title node)
      void clear();

      /// returns internal document handle (forward declared here) (const only)
      /** this function is not available in un-const manner, to avoid that users
          change the document structure somehow, what would cause inconsistencies
          between the internal XMLDocument structure and the ConfigFile data-base */
      const pugi::xml_document *getHandle() const { return m_doc.get(); }

      private:

      /// internal utitlity function to parse existing XMLDocument
      void load_internal();

      /// internal utility function
      Entry &get_entry_internal(const std::string &id) throw (EntryNotFoundException);

      /// internal utility function
      const Entry &get_entry_internal(const std::string &id) const throw (EntryNotFoundException);

      /// internal utility function
      void set_internal(const std::string &id, const std::string &val, const std::string &type) throw (UnregisteredTypeException);

      /// internally synchronized an add- or a set call
      static void add_to_doc(pugi::xml_document &h,const std::string &id,const std::string &type,
                             const std::string &value,const KeyRestriction *restr=0);

      /// shallow copyable smart pointer of the document handle
      mutable SmartPtrBase<pugi::xml_document,XMLDocumentDelOp> m_doc;

      /// global ConfigFile instance
      static ConfigFile s_oConfig;


      /// current string prefix contents
      mutable std::string m_sDefaultPrefix;


      /// DataStore contents
      std::map<std::string,Entry> m_entries;

      /// ostream operator is allowed to access privat members
      friend ICLUtils_API std::ostream &operator<<(std::ostream&, const ConfigFile&);
    };


    /// Default ostream operator to put a ConfigFile into a stream
    ICLUtils_API std::ostream &operator<<(std::ostream &s, const ConfigFile &cf);

    /** \cond */
    template<> inline ConfigFile::Data &ConfigFile::Data::operator=(char * const &t)
    throw (UnregisteredTypeException,InvalidTypeException,EntryNotFoundException){
      return ConfigFile::Data::operator=(std::string(t));
    }
    template<> inline ConfigFile::Data &ConfigFile::Data::operator=(const char * const &t)
    throw (UnregisteredTypeException,InvalidTypeException,EntryNotFoundException){
      return ConfigFile::Data::operator=(std::string(t));
    }
    /** \endcond */

  } // namespace utils
}



