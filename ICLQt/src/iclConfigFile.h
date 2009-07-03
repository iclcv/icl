#ifndef ICL_CONFIG_FILE_H
#define ICL_CONFIG_FILE_H

#include <iclStringUtils.h> 
#include <iclDataStore.h>
#include <iclException.h>
#include <iclRange.h>
#include <map>

namespace icl{

  /** \cond */
  class XMLDocument;
  struct XMLDocumentDelOp{ static void delete_func(XMLDocument *h); };
  /** \endcond */

  /// Utility class for creating and reading XML-based hierarchical configuration files 
  /** ConfigFile class can be used in object based as well as in static manner. 
      
      ConfigFile objects can e.g. be used, to locally read a configuration file or
      to create a configuration file. Besides a static singleton ConfigFile object 
      accessible via ConfigFile::getConfig can be used as a global configuration for
      applications. \n
      Furthermore a powerful runtime-editor called ConfigFileGUI is available!.
      
      ConfigFiles are XML-based (using Qt's XML package for parsing and creating XML
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
      
      \subsection RANGES Data Ranges and Value lists
      
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
      

      \subsection OTHERS Other Information

      When accessing ConfigFile data members, each hierarchy level must be
      separated using the '.' character. So e.g. the filename can be accessed using
      DataStore access function:

      \code
      ConfigFile config("myConfig.xml"); // myConfig.xml shall contain the contents above
      string fn = config.get<string>("general.params.filename");
      \endcode
      
      To avoid errors, the "get"-function can be called with a default return value:
      \code
      ConfigFile config("myConfig.xml"); 
      string fn = config.get<string>("general.params.filename","defaultpath.xml");
      \endcode
      
      In the same manner, ConfigFile entries can be generated: The example file above
      was generated with the following code (<b>Note: the "config"-prefix is
      compulsory!</b>)

      \code
      ConfigFile a;
      a.setTitle("This is the new title");
      a.add("config.general.params.threshold",7);
      a.add("config.general.params.value",6.45);  
      a.add("config.general.params.filename",std::string("./hallo.txt"));
      a.add("config.general.params.filename",std::string("./notHallo.txt"));
      a.add("config.special.hint",'a');
      a.add("config.special.no-hint",'b');
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
  class ConfigFile : protected DataStore{
    public:

    struct EntryNotFoundException : public ICLException{
      std::string entryName;
      std::string typeName;
      EntryNotFoundException(const std::string &entryName, const std::string &typeName):
      ICLException(entryName+" could not be found!"),entryName(entryName),typeName(typeName){}
      virtual ~EntryNotFoundException() throw(){}
    };
    
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
    ConfigFile(const std::string &filename) throw(FileNotFoundException,InvalidFileFormatException);
    
    /// return the current filename
    const std::string &getFileName() const { 
      return m_sFileName;
    }
    
    /// sets new internal filename for using load(void) or save(void)
    void setFileName(const std::string &fileName) {
      m_sFileName = fileName;
    }
    
    /// loads the ConfigFile from given filename and updates internal filename variable
    /** Warning: old data content is lost! */
    void load(const std::string &filename) throw(FileNotFoundException,InvalidFileFormatException);

    /// loads the ConfigFile using internal filename variable
    /** Warning: old data content is lost! */
    void load() throw(FileNotFoundException,InvalidFileFormatException);

    /// Writes data to disk using given filename
    void save(const std::string &filename) const;
    
    /// Writes data to dist using internal filename
    void save() const;
    
    /// Sets up a new title to the internal document
    void setTitle(const std::string &title);
    
    /// Sets up a default prefix automatically put before each given key
    void setDefaultPrefix(const std::string &defaultPrefix){
      m_sDefaultPrefix = defaultPrefix;
    }
    
    /// Returns given default prefix
    const std::string &getDefaultPrefix() const { return m_sDefaultPrefix; }
    
    /// Adds a new data element to the ConfigFile
    /** Warning: if not unique decidable, an explicit information about the
        value type T is compulsory.
        
        e.g. 
        \code
        ConfigFile f;
        f.add("config.filename","myText.txt");
        \endcode

        Causes an error because the template type T is resolved as const char* which
        is not supported yet. Better versions use an explicit template:
        \code
        ConfigFile f;
        f.add<std::string>("config.filename","myText.txt");
        \endcode

        or an explicit string argument:
        \code
        ConfigFile f;
        f.add("config.filename",std::string("myText.txt"));
        \endcode
        to avoid these errors.\n
        <b>Note:</b>
        Integer constants are of type "int" by default, and floating point constants are of
        type "double" by default:
    */
    template<class T>
    void add(const std::string &idIn, const T &val){
      std::string id=m_sDefaultPrefix + idIn;
      if(contains(id)){
        ERROR_LOG("unable to add already existing entry \"" << id << "\" (use set instead)");
      }else{
        set<T>(idIn,val);
      }
    }    

    /// like add, but works if key does not exist too
    template<class T>
    void set(const std::string &idIn, const T &val){
      std::string id=m_sDefaultPrefix + idIn;
      if(contains(id) && !checkType<T>(id)){
        ERROR_LOG("id " << id << "is already set with different type!");
        return;
      }
      if(contains(id)) getValue<T>(id) = val;
      else allocValue<T>(id,val);
      add_to_doc(*m_doc,id,get_type_str<T>(),str(val));
    }

    
    /// updates a value within the config file
    /// returns a given value from the internal document hash-map (un const)
    /** This function is equivalent to the getValue<T> function template
        provided by the parent DataStore class, except this function applies
        two additional checks first:

        - checking if entry "id" is actually contained (if not, def is returned)
        - checking if entry "id" has actually the given type (if not, def is returned)

        This is the un-const version, so\n
        If the given entry (+current default prefix) is not found whithin the
        parent data store, it is allocated and initialized with the default value.
    */
    template<class T>
    inline const T &get(const std::string &idIn,const T &def=T()){
      std::string id = m_sDefaultPrefix+idIn;
      if(contains(id)){
        if(checkType<T>(id)){
          return getValue<T>(id);
        }else{
          static T _def;
          ERROR_LOG("type missmatch for entry \"" << idIn << "\"");
          return _def;
        }
      }else{
        add(id,def);
        return getValue<T>(id);
      }
    }

    /// returns a given value from the internal document hash-map (const version)
    /** Essentially, this function behaves like the un-const version, except, it
        does not return a reference to the data store entry, but a deeply copied
        value. If idIn (+current default prefix) is not found, the (given) default
        value is returned;
        
        If a const reference to a value within a const ConfigFile is needed,
        one has to use try_get(const std::string&)
    */
    template<class T>
    inline T get(const std::string &idIn,const T &def=T()) const{
      std::string id = m_sDefaultPrefix+idIn;
      if(contains(id) && checkType<T>(id)){
        return getValue<T>(id);
      }else{
        return def;
      }
    }



    /// returns a given value from the internal document hash-map
    /** This function is equivalent to the getValue<T> function template
        provided by the parent DataStore class, except this function applies
        two additional checks first:
        - checking if entry "id" is actually contained (if not, an ConfigFile::EntryNotFoundException is thrown)
        - checking if entry "id" has actually the given type (if not, an ConfigFile::EntryNotFoundException is thrown)
    */
    template<class T>
    inline const T &try_get(const std::string &idIn) const throw (EntryNotFoundException){
      std::string id = m_sDefaultPrefix+idIn;
      if(!contains(id) || !checkType<T>(id)){
        throw EntryNotFoundException(id,get_type_name<T>());
      }
      return getValue<T>(id);
    }

    // ---------------- static misc ----------------------------
    
    
    /// loads the global ConfigFile from given filename
    static void loadConfig(const std::string &filename);

    /// loads a ConfigFile object into the global config file (shallowly copied!)
    static void loadConfig(const ConfigFile &configFile);
    
    /// returns the global ConfigFile
    static const ConfigFile &getConfig(){ return s_oConfig; }

    /// equivalent to ConfigFile::getConfig().get...
    template<class T>
    static const T sget(const std::string &id, const T &def=T()){
      return getConfig().get<T>(id,def);
    }
    /// equivalent to ConfigFile::getConfig().try_get...
    template<class T>
    static const T &stry_get(const std::string &id) throw (EntryNotFoundException){
      return getConfig().try_get<T>(id);
    }

    
    /// this function is imported from the parent DataStore class
    DataStore::getEntryList;

    /// this function is imported from the parent DataStore class
    DataStore::listContents;

    /// this function is imported from the parent DataStore class
    DataStore::contains;

    /// this function is imported from the parent DataStore class
    DataStore::lock;

    /// this function is imported from the parent DataStore class
    DataStore::unlock;
    
    // this function is imported from the parent DataStore class
    /* this is not useful here, setting a Data does not update the XML structure and
       getting an element is easier using ConfigFile::get() */
    // DataStore::operator[];

    /// just imported form the parent DataStore class
    template<class T>
    inline bool checkType(const std::string &id) const{
      return DataStore::checkType<T>(id);
    }

    
    struct KeyRestriction{
      inline KeyRestriction():
        hasRange(false),hasValues(false){}
      inline KeyRestriction(const Range64f &r):
        range(r),hasRange(true),hasValues(false){}
      inline KeyRestriction(const std::string &values):
        values(values),hasRange(false),hasValues(true){}

      Range64f range;
      std::string values;

      bool hasRange;
      bool hasValues;
    };
    
    /// defined the range for given number-type-valued key
    /** This feature is used by the ConfigFileGUI to create appropriate slider
        ranges if requested */
    void setRestriction(const std::string &id, const KeyRestriction &r);

    /// returns predefined range for given id (or 0 if no range was defined for this key)
    /** This feature is only used by the config file GUI */
    const KeyRestriction *getRestriction(const std::string &id) const;

    private:


    /// filename
    std::string m_sFileName;
    
    /// internally synchronized an add- or a set call
    static void add_to_doc(XMLDocument &h,const std::string &id,const std::string &type, const std::string &value);
   
    template<class T>
    static std::string get_type_str();
     
    /// shallow copyable smart pointer of the document handle
    mutable SmartPtr<XMLDocument,XMLDocumentDelOp> m_doc;
    
    /// global ConfigFile instance 
    static ConfigFile s_oConfig;
    
    /// internally used utility function
    void updateTitleFromDocument();

    std::string m_sDefaultPrefix;
    
    /// for ranged sliders within the config file GUI
    SmartPtr<std::map<std::string,KeyRestriction>,PointerDelOp> m_restrictions;

    /// ostream operator is allowed to access privat members
    friend std::ostream &operator<<(std::ostream&,const ConfigFile&);
  };
  
  
  /// Default ostream operator to put a ConfigFile into a stream
  std::ostream &operator<<(std::ostream &s, const ConfigFile &cf);
  

}



#endif
