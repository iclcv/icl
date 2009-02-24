#ifndef ICL_CONFIG_ENTRY_H
#define ICL_CONFIG_ENTRY_H

#include <iclConfigFile.h>

namespace icl{

  
  /// Utility class for referencing runtime-dynamic ConfigFile entries
  /** TODO: put in a use case here ... */
  template<class T>
  struct ConfigEntry{
    
    /// Create an empty default Entry referencing a default constructed T
    inline ConfigEntry():m_def(T()),m_entry(&m_def),m_config(0){}
    
    inline ConfigEntry(const ConfigEntry &other){
      *this = other;
    }
    /// Creates a new ConfigEntry instance
    /** @param key reference key
        @param default value that should be used if key is not found 
        @param cfg config file to use (note: each entry remains valid as long 
               its parent config file remains allocated */
    inline ConfigEntry(const std::string &key,
                       const T &def=T(),
                       const ConfigFile &cfg=ConfigFile::getConfig()){
      
      m_config = const_cast<ConfigFile*>(&cfg);
      m_config->lock();
      if(cfg.contains(key)){
        if(cfg.checkType<T>(key)){
          m_entry = &cfg.try_get<T>(key);
        }else{
          ERROR_LOG("type missmatch: Entry \"" << key << "\" (using default)");
          m_def = def;
          m_entry = &m_def;
        }
      }else{
        m_def = def;
        m_entry = &m_def;
#ifdef WARN_IF_CONFIG_ENTRY_NOT_FOUND
        ERROR_LOG("config entry \"" << key << "\" was not found! using default value " << (T)(*this));
#endif

      }
      m_config->unlock();
    }
    
    /// default copy operator
    /** If the default value is used, this function ensured, that the default
        value is copied, but the left values reference pointer is not copied,
        but setup to reference it's own default value
    */
    ConfigEntry &operator=(const ConfigEntry &other){
      m_config = other.m_config;
      m_def = other.m_def;
      if(other.m_entry == &other.m_def){
        m_entry = &m_def;
      }else{
        m_def = other.m_def;
        m_entry = other.m_entry;
      }
      return *this;
    }

    /// returns reference value
    operator T() const{
      ICLASSERT_RETURN_VAL(m_config,T());
      m_config->lock();
      T v = *m_entry;
      m_config->unlock();
      return v;
    }
    void debug_show(const std::string &key="any name"){
      DEBUG_LOG("config_entry: " << key);
      DEBUG_LOG("def= " << m_def << "  entry_ptr = " << m_entry << "  entry_val=" << *m_entry << "  config_ptr=" << m_config);
    }
  private:
    T m_def;
    const T *m_entry;
    ConfigFile *m_config;
  };
  
#define ICL_INSTANTIATE_DEPTH(D) typedef ConfigEntry<icl##D> Cfg##D;
  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
  typedef  ConfigEntry<Point> CfgPoint;
  typedef  ConfigEntry<Size> CfgSize;
  typedef  ConfigEntry<Rect> CfgRect;
  typedef  ConfigEntry<Range32s> CfgRange32s;
  typedef  ConfigEntry<Range32f> CfgRange32f;
}


#endif
