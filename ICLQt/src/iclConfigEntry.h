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
          m_entry = &((DataStore&)cfg).getValue<T>(key);
        }else{
          ERROR_LOG("type missmatch: Entry \"" << key << "\" (using default)");
          m_def = def;
          m_entry = &def;
        }
      }else{
        m_def = def;
        m_entry = &def;
      }
      m_config->unlock();
    }

    /// returns reference value
    operator T() const{
      ICLASSERT_RETURN_VAL(m_config,T());
      m_config->lock();
      T v = *m_entry;
      m_config->unlock();
      return v;
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
