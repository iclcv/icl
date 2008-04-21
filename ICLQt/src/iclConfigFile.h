#ifndef ICL_CONFIG_FILE_H
#define ICL_CONFIG_FILE_H

#include <string>
#include <iclDataStore.h>

namespace icl{

  
  class ConfigFile : public DataStore{
    public:
    
    ConfigFile();
    ConfigFile(const std::string &filename);
    
    const std::string &getFileName() const { 
      return m_sFileName;
    }
    
    void setFileName(const std::string &fileName) {
      m_sFileName = fileName;
    }
    
    void load(const std::string &filename);
    void load();
    void save(const std::string &filename) const;
    void save() const;

    void setTitle(const std::string &title);
    
    template<class T>
    void add(const std::string &id, const T &val);

    
    template<class T>
    inline const T get(const std::string &id,const T &def=T()) const{
      if(contains(id) && checkType<T>(id)){
        return getValue<T>(id);
      }else{
        return def;
      }
    }
    
    
    // static misc ...
    
    static void loadConfig(const std::string &filename);
    static void loadConfig(const ConfigFile &configFile);
    static const ConfigFile &getConfig(){ return s_oConfig; }
    
    private:
    std::string m_sFileName;
    class XMLDocHandle;

    /** \cond */
    struct XMLDocHandleDelOp{ static void delete_func(XMLDocHandle *h); };
    /** \endcond */

    SmartPtr<XMLDocHandle,XMLDocHandleDelOp> m_spXMLDocHandle;
    static ConfigFile s_oConfig;
    
    void updateTitleFromDocument();
  };
  
  
}

#endif
