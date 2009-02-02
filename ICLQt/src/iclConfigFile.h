#ifndef ICL_CONFIG_FILE_H
#define ICL_CONFIG_FILE_H

#include <string>
#include <iclDataStore.h>
#include <iclException.h>
#include <iostream>
namespace icl{

  /// Utility class for creating and reading XML-based hierarchical configuration files 
  /** ConfigFile class can be used in object based as well as in static manner. 
      
      ConfigFile objects can e.g. be used, to locally read a configuration file or
      to create a configuration file. Besides a static singleton ConfigFile object 
      accessible via ConfigFile::getConfig can be used as a global configuration for
      applications.
      
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
      - Range<int>
      - Range<float>
      
      \section PERF Performance
      
      Internally data is stored in the parent classes (DataStore) hash maps to optimize
      data access. ConfigFile data key is the the '.'-concatenated identifier.
      
  */
  class ConfigFile : public DataStore{
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
    void add(const std::string &id, const T &val);


    /// returns a given value from the internal document hash-map
    /** This function is equivalent to the getValue<T> function template
        provided by the parent DataStore class, except this function applies
        two additional checks first:
        - checking if entry "id" is actually contained (if not, def is returned)
        - checking if entry "id" has actually the given type (if not, def is returned)
    */
    template<class T>
    inline const T get(const std::string &idIn,const T &def=T()) const{
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
    inline const T try_get(const std::string &idIn) const throw (EntryNotFoundException){
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
    static const T stry_get(const std::string &id) throw (EntryNotFoundException){
      return getConfig().try_get<T>(id);
    }
      
    private:
    /// filename
    std::string m_sFileName;
    
    /// internally used XMLDocument type (currently QDomDocument)
    class XMLDocHandle;

    /** \cond */
    struct XMLDocHandleDelOp{ static void delete_func(XMLDocHandle *h); };
    /** \endcond */

    /// shallow copyable smart pointer of the document handle
    SmartPtr<XMLDocHandle,XMLDocHandleDelOp> m_spXMLDocHandle;
    
    /// global ConfigFile instance 
    static ConfigFile s_oConfig;
    
    /// internally used utility function
    void updateTitleFromDocument();

    std::string m_sDefaultPrefix;

    /// ostream operator is allowed to access privat members
    friend std::ostream &operator<<(std::ostream&,const ConfigFile&);
  };
  
  
  /// Default ostream operator to put a ConfigFile into a stream
  std::ostream &operator<<(std::ostream &s, const ConfigFile &cf);
  

}



#endif
